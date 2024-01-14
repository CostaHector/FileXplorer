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

AddressELineEdit::AddressELineEdit(QWidget* parent)
    : QStackedWidget(parent),
      m_pathActionsTB(new QToolBar(this)),
      pathLineEdit(new QLineEdit),
      pathComboBox(new QComboBox),
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

  clickMode();
  subscribe();

  pathComboBox->setFixedHeight(CONTROL_HEIGHT);
  pathComboBox->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  setFixedHeight(CONTROL_HEIGHT);
  setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  setStyleSheet(
      "QToolBar{"
      "border-left: 1px solid black;"
      "border-right: 1px solid gray;"
      "border-top: 1px solid gray;"
      "border-bottom: 1px solid gray;"
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
    qDebug("%s", pathInexist.toStdString().c_str());
    QMessageBox::warning(this, "Into path failed", pathInexist);
    pathLineEdit->setText(textFromActions());
    return false;
  }
  const QFileInfo fi(pth);
  if (fi.isFile()) {
    const bool openRet = QDesktopServices::openUrl(QUrl::fromLocalFile(pth));
    qDebug("Direct open file [%s]: [%d]", pth.toStdString().c_str(), openRet);
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
  View::changeDropAction(event, mapToGlobal(event->pos() + TOOLTIP_MSG_PNG_DEV), "", this);
  event->accept();
}

void AddressELineEdit::dropEvent(QDropEvent* event) {
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
  View::changeDropAction(event, mapToGlobal(event->pos() + TOOLTIP_MSG_PNG_DEV), "", this);

  auto urlsLst = event->mimeData()->urls();
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

  const QString& to = textFromCurrentCursor(m_pathActionsTB->actionAt(event->pos()));
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
  qDebug("release to drop here [%s]", droppedPath.toStdString().c_str());
  View::changeDropAction(event, mapToGlobal(event->pos() + TOOLTIP_MSG_PNG_DEV), droppedPath, this);
  return QStackedWidget::dragMoveEvent(event);
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
