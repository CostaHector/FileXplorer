#include "AddressELineEdit.h"
#include "View/ViewHelper.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QList>
#include <QMessageBox>
#include <QToolTip>
#include <Qt>

FocusEventWatch::FocusEventWatch(QObject* parent) : mouseButtonPressedBefore(false) {
  if (parent == nullptr) {
    qDebug("Focus Event not works for a nullptr.");
    return;
  }
  parent->installEventFilter(this);
}

bool FocusEventWatch::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::Type::MouseButtonPress) {
    mouseButtonPressedBefore = true;
  } else if (event->type() == QEvent::Type::MouseButtonRelease) {
    ;
  } else if (event->type() == QEvent::Type::FocusOut) {
    if (not mouseButtonPressedBefore) {  // block until next time focus out
      emit focusChanged(false);
    }
    mouseButtonPressedBefore = false;
  } else if (event->type() == QEvent::Type::FocusIn) {
    if (not mouseButtonPressedBefore) {  // block until next time focus out
      emit focusChanged(true);
    }
    mouseButtonPressedBefore = false;
  }
  return QObject::eventFilter(watched, event);
}

const QString AddressELineEdit::DRAG_HINT_MSG = "Drag some files/folders here";
const QString AddressELineEdit::RELEASE_HINT_MSG = "Drop item(s) to [%1]";

AddressELineEdit::AddressELineEdit(QWidget* parent)
    : QStackedWidget(parent),
      m_pathActionsTB(new QToolBar(this)),
      pathLineEdit(new QLineEdit),
      pathComboBox(new QComboBox),
      m_dropPanel(new QLabel(DRAG_HINT_MSG)),
      pathComboBoxFocusWatcher(new FocusEventWatch(pathComboBox)) {
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
  const QString& fullPth = PATHTOOL::StripTrailingSlash(rawPath);
  qDebug("Path triggered [%s]", qPrintable(fullPth));
  ChangePath(fullPth);
}

void AddressELineEdit::onReturnPressed() {
  ChangePath(pathFromLineEdit());
}

auto AddressELineEdit::updateAddressToolBarPathActions(const QString& newPath) -> void {
  m_pathActionsTB->clear();
  const QString& fullpath = PATHTOOL::StripTrailingSlash(PATHTOOL::normPath(newPath));
  if (fullpath != pathLineEdit->text()) {
    pathLineEdit->setText(fullpath);
  }
  qDebug("set Path [%s]", qPrintable(fullpath));
  for (const QString& pt : fullpath.split(PATHTOOL::PATH_SEP_CHAR)) {
    m_pathActionsTB->addAction(new (std::nothrow) QAction{pt, m_pathActionsTB});
  }

  if (not pathComboBox->hasFocus()) {  // in disp mode
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
  if (not pth.isEmpty() and not QFile::exists(pth)) {
#else
  if (not QFile::exists(pth)) {
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
