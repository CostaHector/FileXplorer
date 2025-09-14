#include "AddressELineEdit.h"
#include "ViewHelper.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QToolTip>
#include <Qt>
#include <QCompleter>
#include <QApplication>
#include <QStyle>

const QString AddressELineEdit::RELEASE_HINT_MSG = "<b>Drop item(s) to ...?</b>:<br/>";

AddressELineEdit::AddressELineEdit(QWidget* parent) : QStackedWidget{parent} {
  m_pathActionsTB = new (std::nothrow) QToolBar{this};
  CHECK_NULLPTR_RETURN_VOID(m_pathActionsTB);

  m_pathComboBox = new (std::nothrow) PathComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_pathComboBox);
  m_pathComboBox->setEditable(true);
  m_pathComboBox->setInsertPolicy(QComboBox::InsertAtTop);
  m_pathComboBox->lineEdit()->addAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirOpenIcon), QLineEdit::ActionPosition::LeadingPosition);
  m_pathComboBox->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  QCompleter* pCompleter = new QCompleter{this};
  CHECK_NULLPTR_RETURN_VOID(pCompleter);
  pCompleter->setCaseSensitivity(Qt::CaseSensitive);
  m_pathComboBox->setCompleter(pCompleter);

#ifdef _WIN32
  const QFileInfoList& drives = QDir::drives();
  for (const auto& d : drives) {
    m_pathComboBox->addItem(d.filePath());
  }
#endif

  addWidget(m_pathActionsTB);
  addWidget(m_pathComboBox);

  clickMode();
  subscribe();

  setFocusPolicy(Qt::FocusPolicy::StrongFocus);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);

  setAcceptDrops(true);
}

AddressELineEdit::~AddressELineEdit() {
  if (mFocusChangedConn) {
    QStackedWidget::disconnect(mFocusChangedConn);
  }
}

QString AddressELineEdit::NormToolBarActionPath(QString actionPath) {
#ifdef _WIN32
  QString pth = actionPath.mid(1);
  int n = pth.size();
  if (n >=2 && pth[n-2] != ':' && pth[n-1] == '/') return pth.left(n-1);
  return pth;
#else
  return actionPath.size() > 1 && actionPath.back() == '/' ? actionPath.left(actionPath.size() - 1) : actionPath;
#endif
}

auto AddressELineEdit::onPathActionTriggered(const QAction* cursorAt) -> void {
  const QString fullPth = pathFromCursorAction(cursorAt);
  LOG_D("Path triggered [%s]", qPrintable(fullPth));
  ChangePath(fullPth);
}

void AddressELineEdit::onReturnPressed() {
  ChangePath(pathFromLineEdit());
}

void AddressELineEdit::updateAddressToolBarPathActions(const QString& newPath) {
  const QString& fullpath = PathTool::normPath(newPath);
  m_pathComboBox->setCurrentText(fullpath);
  LOG_D("set Path [%s]", qPrintable(fullpath));
  m_pathActionsTB->clear();
  m_pathActionsTB->addAction(QIcon{":img/FOLDER_OF_DRIVES"}, "");
  for (const QString& pathSec : fullpath.split(PathTool::PATH_SEP_CHAR, Qt::SkipEmptyParts)) {
    auto* pAct = m_pathActionsTB->addAction(pathSec);
    auto* pWid = m_pathActionsTB->widgetForAction(pAct);
    pWid->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  }
}

auto AddressELineEdit::ChangePath(const QString& path) -> bool {
  const QString& pth = QDir::fromNativeSeparators(path);
  if (!pth.isEmpty() && !QFile::exists(pth)) {
    m_pathComboBox->setCurrentText(pth);
    LOG_ERR_NP("Path not empty but not exist", pth);
    return false;
  }
  const QFileInfo fi{pth};
  if (fi.isFile()) {
    m_pathComboBox->setCurrentText(pth);
    const bool bRet = QDesktopServices::openUrl(QUrl::fromLocalFile(pth));
    LOG_INFO_P("Open file", "[%s]: bRet[%d]", qPrintable(pth), bRet);
  } else {
    updateAddressToolBarPathActions(pth);
    emit pathActionsTriggeredOrLineEditReturnPressed(pth);
    emit m_pathComboBox->focusChanged(false);
  }
  return true;
}

auto AddressELineEdit::subscribe() -> void {
  connect(m_pathActionsTB, &QToolBar::actionTriggered, this, &AddressELineEdit::onPathActionTriggered);
  connect(m_pathComboBox->lineEdit(), &QLineEdit::returnPressed, this, &AddressELineEdit::onReturnPressed);
  mFocusChangedConn = connect(m_pathComboBox, &PathComboBox::focusChanged, this, &AddressELineEdit::onFocusChange);
}

auto AddressELineEdit::onFocusChange(bool hasFocus) -> void {
  if (hasFocus && currentWidget() != m_pathComboBox) {
    inputMode();
  } else if (!hasFocus && currentWidget() != m_pathActionsTB) {
    clickMode();
  }
}

auto AddressELineEdit::clickMode() -> void {
  setCurrentWidget(m_pathActionsTB);
}

auto AddressELineEdit::inputMode() -> void {
  setCurrentWidget(m_pathComboBox);
}

void AddressELineEdit::mousePressEvent(QMouseEvent* event) {
  if (currentWidget() == m_pathActionsTB) {
    QAction* action = m_pathActionsTB->actionAt(event->pos());
    if (action == nullptr) { // click at blank area. no action correspond
      emit m_pathComboBox->focusChanged(true);
      m_pathComboBox->setFocus();
      m_pathComboBox->lineEdit()->selectAll();
      return;
    }
  }
  QStackedWidget::mousePressEvent(event);
}

void AddressELineEdit::keyPressEvent(QKeyEvent* e) {
  if (e->key() == Qt::Key_Escape) {
    emit m_pathComboBox->focusChanged(false);
  }
}

void AddressELineEdit::dragEnterEvent(QDragEnterEvent* event) {
  const QString& draggedEnterPath = pathFromCursorAction(m_pathActionsTB->actionAt(event->pos()));
  View::changeDropAction(event);
  const QString& dragEnterMsg{RELEASE_HINT_MSG + draggedEnterPath};
  LOG_D("%s", qPrintable(dragEnterMsg));
  QToolTip::showText(mapToGlobal(event->pos()), dragEnterMsg);
  event->accept();
}

void AddressELineEdit::dropEvent(QDropEvent* event) {
  setCurrentWidget(m_pathActionsTB);
  View::changeDropAction(event);
  const QString& to = pathFromCursorAction(m_pathActionsTB->actionAt(event->pos()));
  LOG_D("Drop items to path[%s]", qPrintable(to));
  View::onDropMimeData(event->mimeData(), event->dropAction(), to);
  event->accept();
}

void AddressELineEdit::dragMoveEvent(QDragMoveEvent* event) {
  if (!event->mimeData()->hasUrls()) {
    LOG_D("no urls dragMoveEvent");
    return;
  }
  const QString& droppedPath = pathFromCursorAction(m_pathActionsTB->actionAt(event->pos()));
  const QString& dragMoveMsg{RELEASE_HINT_MSG + droppedPath};
  QToolTip::showText(mapToGlobal(event->pos()), dragMoveMsg);
  View::changeDropAction(event);
  event->accept();
}
