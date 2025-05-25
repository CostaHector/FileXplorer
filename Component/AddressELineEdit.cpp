#include "AddressELineEdit.h"
#include "View/ViewHelper.h"
#include "public/PublicMacro.h"
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QList>
#include <QMessageBox>
#include <QToolTip>
#include <Qt>

FocusEventWatch::FocusEventWatch(QObject* parent)  //
{
  CHECK_NULLPTR_RETURN_VOID(parent);
  parent->installEventFilter(this);
}

bool FocusEventWatch::eventFilter(QObject* watched, QEvent* event) {
  switch (event->type()) {
    case QEvent::Type::MouseButtonPress: {
      mouseButtonPressedBefore = true;
      break;
    }
    case QEvent::Type::MouseButtonRelease: {
      break;
    }
    case QEvent::Type::FocusIn: {
      if (!mouseButtonPressedBefore) {  // block until next time focus out
        emit focusChanged(true);
      }
      mouseButtonPressedBefore = false;
      break;
    }
    case QEvent::Type::FocusOut: {
      if (!mouseButtonPressedBefore) {  // block until next time focus out
        emit focusChanged(false);
      }
      mouseButtonPressedBefore = false;
      break;
    }
    default:
      break;
  }
  return QObject::eventFilter(watched, event);
}

constexpr int AddressELineEdit::MAX_PATH_SECTIONS_CNT;
const QString AddressELineEdit::DRAG_HINT_MSG = "Drag some files/folders here";
const QString AddressELineEdit::RELEASE_HINT_MSG = "Drop item(s) to [%1]";

AddressELineEdit::AddressELineEdit(QWidget* parent)  //
    : QStackedWidget(parent)                         //
{
  m_pathActionsTB = new (std::nothrow) QToolBar{this};
  CHECK_NULLPTR_RETURN_VOID(m_pathActionsTB);

  pathComboBox = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(pathComboBox);

  pathLineEdit = new (std::nothrow) QLineEdit{pathComboBox};
  CHECK_NULLPTR_RETURN_VOID(pathLineEdit);

  m_dropPanel = new (std::nothrow) QLabel(DRAG_HINT_MSG, this);
  CHECK_NULLPTR_RETURN_VOID(m_dropPanel);

  pathComboBoxFocusWatcher = new (std::nothrow) FocusEventWatch(pathComboBox);
  CHECK_NULLPTR_RETURN_VOID(pathComboBoxFocusWatcher);

#ifdef _WIN32
  const QFileInfoList& drives = QDir::drives();
  for (const auto& d : drives) {
    pathComboBox->addItem(d.filePath());
  }
#endif
  pathComboBox->setLineEdit(pathLineEdit);

  addWidget(m_pathActionsTB);
  addWidget(pathComboBox);
  addWidget(m_dropPanel);

  clickMode();
  subscribe();

  pathComboBox->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  m_pathActionsTB->setStyleSheet(
      "QToolBar{"
      "border-left: 1px solid gray;"
      "border-right: 1px solid gray;"
      "};");
  m_dropPanel->setStyleSheet(
      "QLabel{"
      "border: 3px solid cyan;"
      "};");

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);

  setAcceptDrops(true);
}

auto AddressELineEdit::onPathActionTriggered(const QAction* cursorAt) -> void {
  const QString& rawPath = pathFromCursorAction(cursorAt);
  const QString& fullPth = PathTool::StripTrailingSlash(rawPath);
  qDebug("Path triggered [%s]", qPrintable(fullPth));
  ChangePath(fullPth);
}

void AddressELineEdit::onReturnPressed() {
  ChangePath(pathFromLineEdit());
}

void AddressELineEdit::updateAddressToolBarPathActions(const QString& newPath) {
  const QString& fullpath = PathTool::StripTrailingSlash(PathTool::normPath(newPath));
  if (fullpath != pathLineEdit->text()) {
    pathLineEdit->setText(fullpath);
  }
  qDebug("set Path [%s]", qPrintable(fullpath));

  int n = m_pathActionsTB->actions().count();
  for (int i = 0; i < n; ++i) {
    m_pathActionsTB->removeAction(&mPathSections[i]);
  }

  const QStringList sections{fullpath.split(PathTool::PATH_SEP_CHAR)};
  const int N = sections.size();
  if (N > MAX_PATH_SECTIONS_CNT) {
    qWarning("Path[%s] contains more than %d sections", qPrintable(fullpath), sections.size());
    return;
  }

  for (int i = 0; i < N; ++i) {
    mPathSections[i].setText(sections[i]);
    m_pathActionsTB->addAction(&mPathSections[i]);
  }

  if (!pathComboBox->hasFocus()) {  // in disp mode
    pathComboBox->insertItem(0, fullpath);
  } else {  // in edit mode, when return pressed it would auto append one text to the back
    pathComboBox->insertItem(0, fullpath);
    pathComboBox->removeItem(pathComboBox->count() - 1);
  }
  if (pathComboBox->count() != 0) {
    pathComboBox->setCurrentIndex(0);
  }
}

auto AddressELineEdit::ChangePath(const QString& path) -> bool {
  const QString& pth = QDir::fromNativeSeparators(path);
#ifdef WIN32
  if (!pth.isEmpty() && !QFile::exists(pth)) {
#else
  if (!QFile::exists(pth)) {
#endif
    const QString& pathInexist = QString("Return pressed with inexist path [%1].").arg(pth);
    qDebug("%s", qPrintable(pathInexist));
    QMessageBox::warning(this, "Into path failed", pathInexist);
    pathLineEdit->setText(pathFromFullActions());
    return false;
  }
  const QFileInfo fi(pth);
  if (fi.isFile()) {
    const bool openRet = QDesktopServices::openUrl(QUrl::fromLocalFile(pth));
    qDebug("Direct open file [%s]: [%d]", qPrintable(pth), openRet);
    pathLineEdit->setText(pathFromFullActions());
  } else {
    updateAddressToolBarPathActions(pth);
    emit pathActionsTriggeredOrLineEditReturnPressed(pth);
    emit pathComboBoxFocusWatcher->focusChanged(false);
  }
  return true;
}

auto AddressELineEdit::subscribe() -> void {
  connect(m_pathActionsTB, &QToolBar::actionTriggered, this, &AddressELineEdit::onPathActionTriggered);
  connect(pathLineEdit, &QLineEdit::returnPressed, this, &AddressELineEdit::onReturnPressed);
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
  pathLineEdit->setFocus();
  pathLineEdit->selectAll();
}

void AddressELineEdit::keyPressEvent(QKeyEvent* e) {
  if (e->key() == Qt::Key_Escape) {
    emit pathComboBoxFocusWatcher->focusChanged(false);
  }
}

void AddressELineEdit::dragEnterEvent(QDragEnterEvent* event) {
  const QString& draggedEnterPath = pathFromCursorAction(m_pathActionsTB->actionAt(event->pos()));
  View::changeDropAction(event);
  qDebug("dragged enter[%s]", qPrintable(draggedEnterPath));
  m_dropPanel->setText(RELEASE_HINT_MSG.arg(draggedEnterPath));
  setCurrentWidget(m_dropPanel);
  event->accept();
}

void AddressELineEdit::dropEvent(QDropEvent* event) {
  setCurrentWidget(m_pathActionsTB);
  const QString& to = pathFromCursorAction(m_pathActionsTB->actionAt(event->pos()));
  View::onDropMimeData(event->mimeData(), event->dropAction(), to);
  QStackedWidget::dropEvent(event);
}

void AddressELineEdit::dragMoveEvent(QDragMoveEvent* event) {
  if (not event->mimeData()->hasUrls()) {
    qDebug("no urls dragMoveEvent");
    return;
  }
  const QString& droppedPath = pathFromCursorAction(m_pathActionsTB->actionAt(event->pos()));
  m_dropPanel->setText(RELEASE_HINT_MSG.arg(droppedPath));
  qDebug("release to drop here [%s]", qPrintable(droppedPath));
  View::changeDropAction(event);
  QStackedWidget::dragMoveEvent(event);
}

void AddressELineEdit::dragLeaveEvent(QDragLeaveEvent* event) {
  m_dropPanel->setText(DRAG_HINT_MSG);
  setCurrentWidget(m_pathActionsTB);
  QStackedWidget::dragLeaveEvent(event);
}

class TestAddressELineEdit : public QWidget {
  Q_OBJECT
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
 signals:
};

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  TestAddressELineEdit wid(nullptr);
  wid.show();
  return a.exec();
}
#endif
