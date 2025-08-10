#include "AddressELineEdit.h"
#include "ViewHelper.h"
#include "PublicMacro.h"
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

AddressELineEdit::AddressELineEdit(QWidget* parent)  //
  : QStackedWidget{parent}                         //
{
  m_pathActionsTB = new (std::nothrow) QToolBar{this};
  CHECK_NULLPTR_RETURN_VOID(m_pathActionsTB);

  pathComboBox = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(pathComboBox);
  pathComboBox->setEditable(true);
  pathComboBox->setInsertPolicy(QComboBox::InsertAtTop);
  pathComboBox->lineEdit()->addAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirOpenIcon), QLineEdit::ActionPosition::LeadingPosition);
  pathComboBox->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  QCompleter* pCompleter = new QCompleter{this};
  CHECK_NULLPTR_RETURN_VOID(pCompleter);
  pCompleter->setCaseSensitivity(Qt::CaseSensitive);
  pathComboBox->setCompleter(pCompleter);

#ifdef _WIN32
  const QFileInfoList& drives = QDir::drives();
  for (const auto& d : drives) {
    pathComboBox->addItem(d.filePath());
  }
#endif

  pathComboBoxFocusWatcher = new (std::nothrow) FocusEventWatch(pathComboBox);
  CHECK_NULLPTR_RETURN_VOID(pathComboBoxFocusWatcher);

  addWidget(m_pathActionsTB);
  addWidget(pathComboBox);

  clickMode();
  subscribe();

  setFocusPolicy(Qt::FocusPolicy::StrongFocus);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);

  setAcceptDrops(true);
}

auto AddressELineEdit::onPathActionTriggered(const QAction* cursorAt) -> void {
  const QString& rawPath = pathFromCursorAction(cursorAt);
  const QString fullPth {PathTool::StripTrailingSlash(rawPath)};
  qDebug("Path triggered [%s]", qPrintable(fullPth));
  ChangePath(fullPth);
}

void AddressELineEdit::onReturnPressed() {
  ChangePath(pathFromLineEdit());
}

void AddressELineEdit::updateAddressToolBarPathActions(const QString& newPath) {
  const QString& fullpath = PathTool::StripTrailingSlash(PathTool::normPath(newPath));
  pathComboBox->setCurrentText(fullpath);
  qDebug("set Path [%s]", qPrintable(fullpath));
  m_pathActionsTB->clear();
#ifdef WIN32
  m_pathActionsTB->addAction(QIcon{":img/FOLDER_OF_DRIVES"}, "");
#endif
  for (const QString& pathSec : fullpath.split(PathTool::PATH_SEP_CHAR)) {
    m_pathActionsTB->addAction(pathSec);
    qobject_cast<QWidget*>(m_pathActionsTB->children().last())->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  }
}

auto AddressELineEdit::ChangePath(const QString& path) -> bool {
  const QString& pth = QDir::fromNativeSeparators(path);
#ifdef WIN32
  if (!pth.isEmpty() && !QFile::exists(pth)) {
#else
  if (!QFile::exists(pth)) {
#endif
    pathComboBox->setCurrentText(pth);
    const QString pathInexist{QString{"Return pressed with inexist path [%1]."}.arg(pth)};
    qWarning("Into path[%s] failed", qPrintable(pathInexist));
    QMessageBox::warning(this, "Into path failed", pathInexist);
    return false;
  }
  const QFileInfo fi{pth};
  if (fi.isFile()) {
    pathComboBox->setCurrentText(pth);
    const bool openRet = QDesktopServices::openUrl(QUrl::fromLocalFile(pth));
    qDebug("Direct open file [%s]: [%d]", qPrintable(pth), openRet);
  } else {
    updateAddressToolBarPathActions(pth);
    emit pathActionsTriggeredOrLineEditReturnPressed(pth);
    emit pathComboBoxFocusWatcher->focusChanged(false);
  }
  return true;
}

auto AddressELineEdit::subscribe() -> void {
  connect(m_pathActionsTB, &QToolBar::actionTriggered, this, &AddressELineEdit::onPathActionTriggered);
  connect(pathComboBox->lineEdit(), &QLineEdit::returnPressed, this, &AddressELineEdit::onReturnPressed);
  connect(pathComboBoxFocusWatcher, &FocusEventWatch::focusChanged, this, &AddressELineEdit::onFocusChange);
}

auto AddressELineEdit::onFocusChange(bool hasFocus) -> void {
  if (hasFocus) {
    inputMode();
  } else {
    clickMode();
  }
}

auto AddressELineEdit::clickMode() -> void {
  setCurrentWidget(m_pathActionsTB);
}

auto AddressELineEdit::inputMode() -> void {
  setCurrentWidget(pathComboBox);
}

void AddressELineEdit::mousePressEvent(QMouseEvent* /* event */) {
  emit pathComboBoxFocusWatcher->focusChanged(true);
  pathComboBox->setFocus();
  pathComboBox->lineEdit()->selectAll();
}

void AddressELineEdit::keyPressEvent(QKeyEvent* e) {
  if (e->key() == Qt::Key_Escape) {
    emit pathComboBoxFocusWatcher->focusChanged(false);
  }
}

void AddressELineEdit::dragEnterEvent(QDragEnterEvent* event) {
  const QString& draggedEnterPath = pathFromCursorAction(m_pathActionsTB->actionAt(event->pos()));
  View::changeDropAction(event);
  const QString& dragEnterMsg{RELEASE_HINT_MSG + draggedEnterPath};
  qDebug("%s", qPrintable(dragEnterMsg));
  QToolTip::showText(mapToGlobal(event->pos()), dragEnterMsg);
  event->accept();
}

void AddressELineEdit::dropEvent(QDropEvent* event) {
  setCurrentWidget(m_pathActionsTB);
  const QString& to = pathFromCursorAction(m_pathActionsTB->actionAt(event->pos()));
  qDebug("Drop items to path[%s]", qPrintable(to));
  View::onDropMimeData(event->mimeData(), event->dropAction(), to);
  QStackedWidget::dropEvent(event);
}

void AddressELineEdit::dragMoveEvent(QDragMoveEvent* event) {
  if (!event->mimeData()->hasUrls()) {
    qDebug("no urls dragMoveEvent");
    return;
  }
  const QString& droppedPath = pathFromCursorAction(m_pathActionsTB->actionAt(event->pos()));
  const QString& dragMoveMsg{RELEASE_HINT_MSG + droppedPath};
  QToolTip::showText(mapToGlobal(event->pos()), dragMoveMsg);
  View::changeDropAction(event);
  QStackedWidget::dragMoveEvent(event);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
#include <QHBoxLayout>
class TestAddressELineEdit : public QWidget {
public:
  explicit TestAddressELineEdit(QWidget* parent = nullptr) : QWidget(parent) {
    AddressELineEdit* add = new AddressELineEdit;

    QLineEdit* searchLe = new QLineEdit("Search here");
    searchLe->setClearButtonEnabled(true);

    searchLe->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
    searchLe->setPlaceholderText("Search...");

    QHBoxLayout* lo = new QHBoxLayout;
    lo->addWidget(add);
    lo->addWidget(searchLe);
    setLayout(lo);

    add->setMinimumWidth(400);
    searchLe->setMinimumWidth(100);

    add->updateAddressToolBarPathActions(QFileInfo(__FILE__).absolutePath());
  }
};

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  TestAddressELineEdit wid(nullptr);
  wid.show();
  return a.exec();
}
#endif
