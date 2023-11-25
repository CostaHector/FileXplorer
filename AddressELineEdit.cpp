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

AddressELineEdit::AddressELineEdit(QWidget* parent)
    : QToolBar(parent),
      pathActionsGroup(new QActionGroup(this)),
      pathLineEdit(new QLineEdit),
      pathComboBox(new QComboBox),
      pathComboBoxFocusWatcher(new FocusEventWatch(pathComboBox)),
      addressCBActH(nullptr) {
#ifdef _WIN32
  const QFileInfoList& drives = QDir::drives();
  for (const auto& d : drives) {
    pathComboBox->addItem(d.filePath());
  }
#endif
  pathComboBox->setLineEdit(pathLineEdit);

  addressCBActH = addWidget(pathComboBox);

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
  QString fullpath = AddressELineEdit::PathProcess(newPath);
  pathLineEdit->setText(fullpath);
  for (QAction* action : pathActionsGroup->actions()) {
    pathActionsGroup->removeAction(action);
    removeAction(action);
  }
  for (const QString& pt : fullpath.split('/')) {
    pathActionsGroup->addAction(new QAction(pt));
  }
  addActions(pathActionsGroup->actions());

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
  QString pth = QDir::fromNativeSeparators(path);
  QFileInfo fi(pth);
  if ((not QDir(pth).exists()) and (not fi.exists())) {
    const QString pathInexist = QString("cannot into inexist path[%1]").arg(pth);
    qDebug("%s", pathInexist.toStdString().c_str());
    QMessageBox::warning(this, "Into path failed", pathInexist);
    pathLineEdit->setText(textFromActions());
    return false;
  }

  if (fi.isDir()) {
    emit intoAPath_active(pth);
    pathChangeTo(pth);
  } else if (fi.isFile()) {
    auto openRet = QDesktopServices::openUrl(pth);
    qDebug("Opening [%s]", pth.toStdString().c_str());
  }
  emit pathComboBoxFocusWatcher->focusChanged(false);
  return true;
}

auto AddressELineEdit::subscribe() -> void {
  connect(pathActionsGroup, &QActionGroup::triggered, this, &AddressELineEdit::onPathActionTriggered);
  connect(pathLineEdit, &QLineEdit::returnPressed, this, [this]() -> void { onReturnPressed(text()); });
  connect(pathComboBoxFocusWatcher, &FocusEventWatch::focusChanged, this, &AddressELineEdit::onFocusChange);
}

auto AddressELineEdit::onPathActionTriggered(const QAction* clkAct) -> void {
  QString rawPath;
  for (const auto* act : pathActionsGroup->actions()) {
    rawPath += (act->text() + '/');
    if (act == clkAct) {
      break;
    }
  }
  QString fullPth = AddressELineEdit::PathProcess(rawPath);
  qDebug("now[%s]", fullPth.toStdString().c_str());
  onReturnPressed(fullPth);
}

auto AddressELineEdit::onFocusChange(bool hasFocus) -> void {
  if (hasFocus) {
    inputMode();
  } else {
    clickMode();
  }
}

auto AddressELineEdit::clickMode() -> void {
  if (not pathActionsGroup->isVisible()) {
    pathActionsGroup->setVisible(true);
  }
  addressCBActH->setVisible(false);
}

auto AddressELineEdit::inputMode() -> void {
  pathActionsGroup->setVisible(false);
  if (not addressCBActH->isVisible()) {
    addressCBActH->setVisible(true);
  }
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
  View::changeDropAction(event, event->pos(), "", this);
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
  View::changeDropAction(event, event->pos(), "", this);

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

  const QString& to = textFromCurrentCursor(actionAt(event->pos()));
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
  QToolBar::dropEvent(event);
}

void AddressELineEdit::dragMoveEvent(QDragMoveEvent* event) {
  if (not event->mimeData()->hasUrls()) {
    qDebug("no urls dragMoveEvent");
    return;
  }
  const QString& droppedPath = textFromCurrentCursor(actionAt(event->pos()));
  qDebug("release to drop here [%s]", droppedPath.toStdString().c_str());
  View::changeDropAction(event, event->pos(), droppedPath, this);
  return QToolBar::dragMoveEvent(event);
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
