#include "AddressELineEdit.h"
#include "Component/RenameConflicts.h"
#include "Tools/ConflictsItemHelper.h"
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
    qDebug("MouseButtonPress");
    mouseButtonPressedBefore = true;
  } else if (event->type() == QEvent::Type::MouseButtonRelease) {
    ;
  } else if (event->type() == QEvent::Type::FocusOut) {
    if (not mouseButtonPressedBefore) {  // block until next time focus out
      qDebug("1");
      emit focusChanged(false);
    }
    qDebug("2");
    mouseButtonPressedBefore = false;
  } else if (event->type() == QEvent::Type::FocusIn) {
    if (not mouseButtonPressedBefore) {  // block until next time focus out
      qDebug("3");
      emit focusChanged(true);
    }
    qDebug("4");
    mouseButtonPressedBefore = false;
  }
  return QObject::eventFilter(watched, event);
}

constexpr char AddressELineEdit::PATH_SEP_CHAR;

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

  pathComboBox->setFixedHeight(CONTROL_HEIGHT);
  pathComboBox->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  setFixedHeight(CONTROL_HEIGHT);
  setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  m_pathActionsTB->setStyleSheet(
      "QToolBar{"
      "border-left: 1px solid black;"
      "border-right: 1px solid gray;"
      "border-top: 1px solid gray;"
      "border-bottom: 1px solid gray;"
      "};");
  m_dropPanel->setStyleSheet(
      "QLabel{"
      "border: 3px solid cyan;"
      "};");

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);

  setAcceptDrops(true);
}

auto AddressELineEdit::pathChangeTo(const QString& newPath) -> void {
  m_pathActionsTB->clear();

  const QString& fullpath = AddressELineEdit::PathProcess(newPath);
  pathLineEdit->setText(fullpath);
  for (const QString& pt : fullpath.split(PATH_SEP_CHAR)) {
    m_pathActionsTB->addAction(new QAction(pt));
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

auto AddressELineEdit::onReturnPressed(const QString& path) -> bool {
  const QString& pth = QDir::fromNativeSeparators(path);
  if (not QFile::exists(pth)) {
    const QString& pathInexist = QString("Return pressed with inexist path [%1].").arg(pth);
    qDebug("%s", qPrintable(pathInexist));
    QMessageBox::warning(this, "Into path failed", pathInexist);
    pathLineEdit->setText(textFromActions());
    return false;
  }
  const QFileInfo fi(pth);
  if (fi.isFile()) {
    const bool openRet = QDesktopServices::openUrl(QUrl::fromLocalFile(pth));
    qDebug("Direct open file [%s]: [%d]", qPrintable(pth), openRet);
    pathLineEdit->setText(textFromActions());
    return true;
  }
  if (fi.isDir()) {
    pathChangeTo(pth);
    emit intoAPath_active(pth);
    emit pathComboBoxFocusWatcher->focusChanged(false);
    return true;
  }
  return false;
}

auto AddressELineEdit::subscribe() -> void {
  connect(m_pathActionsTB, &QToolBar::actionTriggered, this, &AddressELineEdit::onPathActionTriggered);
  connect(pathLineEdit, &QLineEdit::returnPressed, this, [this]() -> void { onReturnPressed(text()); });
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

void AddressELineEdit::mousePressEvent(QMouseEvent* event) {
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
  const QString& draggedEnterPath = textFromCurrentCursor(m_pathActionsTB->actionAt(event->pos()));
  View::changeDropAction(event, mapToGlobal(event->pos() + TOOLTIP_MSG_PNG_DEV), draggedEnterPath, this);
  qDebug("dragged enter[%s]", qPrintable(draggedEnterPath));
  m_dropPanel->setText(RELEASE_HINT_MSG.arg(draggedEnterPath));
  setCurrentWidget(m_dropPanel);
  event->accept();
}

void AddressELineEdit::dropEvent(QDropEvent* event) {
  setCurrentWidget(m_pathActionsTB);

  QStringList selectedItems;
  for (const QUrl& url : event->mimeData()->urls()) {
    if (url.isLocalFile()) {
      selectedItems.append(url.toLocalFile());
    }
  }
  if (selectedItems.isEmpty()) {
    qDebug("nothing selected to drop");
    return;
  }

  const QString& to = textFromCurrentCursor(m_pathActionsTB->actionAt(event->pos()));
  View::changeDropAction(event, mapToGlobal(event->pos() + TOOLTIP_MSG_PNG_DEV), to, this);

  const auto action = event->dropAction();
  qDebug("dropMimeData. action=[%d]", int(action));
  CCMMode opMode = CCMMode::ERROR;
  if (action == Qt::DropAction::CopyAction) {
    opMode = CCMMode::COPY;
  } else if (action == Qt::DropAction::MoveAction) {
    opMode = CCMMode::CUT;
  } else if (action == Qt::DropAction::LinkAction) {
    opMode = CCMMode::LINK;
  } else {
    qDebug("[Err] Unknown action[%d]", int(action));
    return;
  }

  ConflictsItemHelper conflictIF(selectedItems, to);
  auto* tfm = new RenameConflicts(conflictIF, opMode);

  if (to == conflictIF.l and opMode != CCMMode::LINK) {  // skip
    return;
  }

  if (not conflictIF) {  // conflict
    tfm->on_Submit();
  } else {
    tfm->exec();
  }
  QStackedWidget::dropEvent(event);
}

void AddressELineEdit::dragMoveEvent(QDragMoveEvent* event) {
  if (not event->mimeData()->hasUrls()) {
    qDebug("no urls dragMoveEvent");
    return;
  }
  const QString& droppedPath = textFromCurrentCursor(m_pathActionsTB->actionAt(event->pos()));
  m_dropPanel->setText(RELEASE_HINT_MSG.arg(droppedPath));
  qDebug("release to drop here [%s]", qPrintable(droppedPath));
  View::changeDropAction(event, mapToGlobal(event->pos() + TOOLTIP_MSG_PNG_DEV), droppedPath, this);
  QStackedWidget::dragMoveEvent(event);
}

void AddressELineEdit::dragLeaveEvent(QDragLeaveEvent* event) {
  m_dropPanel->setText(DRAG_HINT_MSG);
  setCurrentWidget(m_pathActionsTB);
  QStackedWidget::dragLeaveEvent(event);
}

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
