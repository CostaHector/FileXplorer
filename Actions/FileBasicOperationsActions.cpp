#include "FileBasicOperationsActions.h"
#include <QApplication>
#include <QStyle>

QActionGroup* FileBasicOperationsActions::GetDeleteActions() {
  MOVE_TO_TRASHBIN->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_D));
  MOVE_TO_TRASHBIN->setShortcutVisibleInContextMenu(true);
  MOVE_TO_TRASHBIN->setToolTip(QString("<b>%1 (%2)</b><br/> Move the selected item(s) to the Recyle Bin.")
                                   .arg(MOVE_TO_TRASHBIN->text(), MOVE_TO_TRASHBIN->shortcut().toString()));

  DELETE_PERMANENTLY->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Delete));
  DELETE_PERMANENTLY->setShortcutVisibleInContextMenu(true);
  DELETE_PERMANENTLY->setToolTip(QString("<b>%1 (%2)</b><br/> Delete the selected item(s) permanently")
                                     .arg(DELETE_PERMANENTLY->text(), DELETE_PERMANENTLY->shortcut().toString()));

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(MOVE_TO_TRASHBIN);
  actionGroup->addAction(DELETE_PERMANENTLY);
  return actionGroup;
}

QActionGroup* FileBasicOperationsActions::GetMOVE_COPY_TOActions() {
  _MOVE_TO->setShortcutVisibleInContextMenu(true);
  _MOVE_TO->setToolTip(QString("<b>%1 (%2)</b><br/> Move the selected item(s) to the location one specified later")
                           .arg(_MOVE_TO->text(), _MOVE_TO->shortcut().toString()));
  _COPY_TO->setShortcutVisibleInContextMenu(true);
  _COPY_TO->setToolTip(QString("<b>%1 (%2)</b><br/> Copy the selected item(s) to the location one specified later")
                           .arg(_COPY_TO->text(), _COPY_TO->shortcut().toString()));

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(_MOVE_TO);
  actionGroup->addAction(_COPY_TO);
  return actionGroup;
}

QActionGroup* FileBasicOperationsActions::GetMOVE_COPY_TO_PATH_HistoryActions(const KV& memoryKey) {
  QString historyStr = PreferenceSettings().value(memoryKey.name, memoryKey.v).toString();
  QStringList historyList = historyStr.split(MOVE_COPT_TO_PATH_STR_SEPERATOR);
  QActionGroup* actionGroup = new QActionGroup(this);
  for (const QString& path : historyList) {
    QAction* tempPath = new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirIcon), path);
    tempPath->setCheckable(false);
    actionGroup->addAction(tempPath);
  }
  return actionGroup;
}

QActionGroup* FileBasicOperationsActions::Get_CUT_COPY_PASTE_OPERATIONS_Actions() {
  CUT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_X));
  CUT->setShortcutVisibleInContextMenu(true);
  CUT->setToolTip(QString("<b>%1 (%2)</b><br/> Copy the selected item(s) to the clipboard.").arg(CUT->text(), CUT->shortcut().toString()));

  COPY->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_C));
  COPY->setShortcutVisibleInContextMenu(true);
  COPY->setToolTip(QString("<b>%1 (%2)</b><br/> Move the selected item(s) to the clipboard.").arg(COPY->text(), COPY->shortcut().toString()));

  PASTE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_V));
  PASTE->setShortcutVisibleInContextMenu(true);
  PASTE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Paste the contents of clipboard to the current location.").arg(PASTE->text(), PASTE->shortcut().toString()));

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(CUT);
  actionGroup->addAction(COPY);
  actionGroup->addAction(PASTE);
  return actionGroup;
}

QActionGroup* FileBasicOperationsActions::Get_UNDO_REDO_OPERATIONS_Actions() {
  UNDO_OPERATION->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Z));
  UNDO_OPERATION->setShortcutVisibleInContextMenu(true);
  UNDO_OPERATION->setToolTip(QString("<b>%1 (%2)</b><br/>").arg(UNDO_OPERATION->text(), UNDO_OPERATION->shortcut().toString()));

  REDO_OPERATION->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Y));
  REDO_OPERATION->setShortcutVisibleInContextMenu(true);
  REDO_OPERATION->setToolTip(QString("<b>%1 (%2)</b><br/>").arg(REDO_OPERATION->text(), REDO_OPERATION->shortcut().toString()));

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(UNDO_OPERATION);
  actionGroup->addAction(REDO_OPERATION);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  for (QAction* act : actionGroup->actions()) {
    act->setCheckable(false);
  }
  return actionGroup;
}

QActionGroup* FileBasicOperationsActions::Get_SELECTION_RIBBON_Action() {
  //        SELECT_ALL->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_A));
  SELECT_ALL->setShortcutVisibleInContextMenu(true);
  SELECT_ALL->setToolTip(QString("<b>%1 (%2)</b><br/> Select all item(s) in this view.").arg(SELECT_ALL->text(), SELECT_ALL->shortcut().toString()));

  SELECT_NONE->setShortcutVisibleInContextMenu(true);
  SELECT_NONE->setToolTip(QString("<b>%1 (%2)</b><br/> Clear all your selections.").arg(SELECT_NONE->text(), SELECT_NONE->shortcut().toString()));

  SELECT_INVERT->setShortcutVisibleInContextMenu(true);
  SELECT_INVERT->setToolTip(QString("<b>%1 </b><br/> Reverse the current selections.").arg(SELECT_INVERT->text()));

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(SELECT_ALL);
  actionGroup->addAction(SELECT_NONE);
  actionGroup->addAction(SELECT_INVERT);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  for (QAction* act : actionGroup->actions()) {
    act->setCheckable(false);
  }
  return actionGroup;
}

auto FileBasicOperationsActions::GetOPENActions() -> QActionGroup* {
  _REVEAL_IN_EXPLORER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
  _REVEAL_IN_EXPLORER->setToolTip(QString("<b>%1 (%2)</b><br/> Reveal items in system file explorer")
                                      .arg(_REVEAL_IN_EXPLORER->text(), _REVEAL_IN_EXPLORER->shortcut().toString()));

  _OPEN_IN_TERMINAL->setShortcut(QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_T));
  _OPEN_IN_TERMINAL->setShortcutVisibleInContextMenu(true);

  _OPEN_IN_TERMINAL->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                        "Open current view path in terminal")
                                    .arg(_OPEN_IN_TERMINAL->text(), _OPEN_IN_TERMINAL->shortcut().toString()));

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(_REVEAL_IN_EXPLORER);
  actionGroup->addAction(_OPEN_IN_TERMINAL);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  for (QAction* act : actionGroup->actions()) {
    act->setCheckable(false);
  }
  return actionGroup;
}

QActionGroup* FileBasicOperationsActions::GetCOPY_PATHActions() {
  COPY_FULL_PATH->setShortcut(QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_Insert));
  COPY_FULL_PATH->setToolTip(
      QString("<b>%1 (%2)</b><br/>. <b>path/name</b> <br/>Copy the absolute file name of the selected item(s) to the clipboard.")
          .arg(COPY_FULL_PATH->text(), COPY_FULL_PATH->shortcut().toString()));
  COPY_FULL_PATH->setShortcutVisibleInContextMenu(true);

  COPY_PATH->setToolTip(QString("<b>%1 (%2)</b><br/> <b>path</b>/name <br/>Copy the directory of the selected item(s) to the clipboard.")
                            .arg(COPY_PATH->text(), COPY_PATH->shortcut().toString()));
  COPY_PATH->setShortcutVisibleInContextMenu(true);

  COPY_NAME->setShortcut(QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_Insert));
  COPY_NAME->setToolTip(QString("<b>%1 (%2)</b><br/> path/<b>name</b> <br/>Copy the name of the selected item(s) to the clipboard.")
                            .arg(COPY_NAME->text(), COPY_NAME->shortcut().toString()));
  COPY_NAME->setShortcutVisibleInContextMenu(true);

  COPY_THE_PATH->setToolTip(QString("<b>%1 (%2)</b><br/> <b>pth/itemName.jpg</b> <br/>Given current selected item named 'itemName' and its path "
                                    "'pth', <br/>'pth/itemName.jpg' is copied to the clipboard.")
                                .arg(COPY_THE_PATH->text(), COPY_THE_PATH->shortcut().toString()));
  COPY_THE_PATH->setShortcutVisibleInContextMenu(true);

  COPY_RECORDS->setToolTip(QString("<b>%1 (%2)</b><br/> <b>Copy the selected records, especially name, size, and prepath")
                               .arg(COPY_RECORDS->text(), COPY_RECORDS->shortcut().toString()));
  COPY_RECORDS->setShortcutVisibleInContextMenu(true);

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(COPY_FULL_PATH);
  actionGroup->addAction(COPY_PATH);
  actionGroup->addAction(COPY_NAME);
  actionGroup->addAction(COPY_THE_PATH);
  actionGroup->addAction(COPY_RECORDS);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  for (QAction* act : actionGroup->actions()) {
    act->setCheckable(false);
  }
  return actionGroup;
}

QActionGroup* FileBasicOperationsActions::GetNEWActions() {
  NEW_FOLDER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_N));
  NEW_FOLDER->setShortcutVisibleInContextMenu(true);
  NEW_FOLDER->setToolTip(
      QString("<b>%1 (%2)</b><br/> Create a new folder in current view.").arg(NEW_FOLDER->text(), NEW_FOLDER->shortcut().toString()));
  NEW_FOLDER->setCheckable(false);

  NEW_TEXT_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_T));
  NEW_TEXT_FILE->setShortcutVisibleInContextMenu(true);
  NEW_TEXT_FILE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Create a new text document in current view.").arg(NEW_TEXT_FILE->text(), NEW_TEXT_FILE->shortcut().toString()));
  NEW_TEXT_FILE->setCheckable(false);

  NEW_JSON_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_J));
  NEW_JSON_FILE->setShortcutVisibleInContextMenu(true);
  NEW_JSON_FILE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Create a new json file in current view.").arg(NEW_JSON_FILE->text(), NEW_JSON_FILE->shortcut().toString()));
  NEW_JSON_FILE->setCheckable(false);

  BATCH_NEW_FILES->setToolTip(QString("<b>%1 (%2)</b><br/>").arg(BATCH_NEW_FILES->text(), BATCH_NEW_FILES->shortcut().toString()) +
                              "Create a batch of files by specified pattern.<br/>"
                              "e.g. Given pattern:<br/>"
                              "\"Page %03d.txt$1$10\"<br/>"
                              "it will create ten text documents numbered by Page 001,002,...,009 respectively.");
  BATCH_NEW_FOLDERS->setToolTip(QString("<b>%1 (%2)</b><br/>").arg(BATCH_NEW_FOLDERS->text(), BATCH_NEW_FOLDERS->shortcut().toString()) +
                                "Create a batch of folders by specified pattern.<br/>"
                                "e.g. Given pattern:<br/>"
                                "\"Page %03d$1$10\"<br/>"
                                "it will create ten folders numbered by Page 001,002,...,009 respectively.");

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(NEW_FOLDER);
  actionGroup->addAction(NEW_TEXT_FILE);
  actionGroup->addAction(NEW_JSON_FILE);
  actionGroup->addAction(BATCH_NEW_FILES);
  actionGroup->addAction(BATCH_NEW_FOLDERS);
  return actionGroup;
}

QActionGroup* FileBasicOperationsActions::FolderMergeActions() {
  MERGE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_M));
  MERGE->setShortcutVisibleInContextMenu(true);
  MERGE->setToolTip(QString("<b>%1 (%2)</b><br/> Given folderA and folderB, B+=A").arg(MERGE->text(), MERGE->shortcut().toString()));

  MERGE_REVERSE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_M));
  MERGE_REVERSE->setShortcutVisibleInContextMenu(true);
  MERGE_REVERSE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Given folderA and folderB, B+=A").arg(MERGE_REVERSE->text(), MERGE_REVERSE->shortcut().toString()));

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(MERGE);
  actionGroup->addAction(MERGE_REVERSE);
  return actionGroup;
}

void FileBasicOperationsActions::FolderFileCategoryProcess() {
  _NAME_STANDARDLIZER->setToolTip(
      "<b>Standardized Files/Folders Name under current view path</b><br/>"
      "Given: [A..mp4, A (1).jpg, A -- 2.json]<br/>"
      "Result: [A.mp4, A - 1.jpg, A - 2.json]");
  _CLASSIFIER->setToolTip(
      "<b>Category Files/Folders Name under current view path</b><br/>"
      "Move [A.mp4, A.jpg, A.json]<br/>"
      "To Folder A");
  _DUPLICATE_ITEMS_REMOVER->setToolTip(
      "<b>Remove Files/Folders whose names with a certern pattern under current view path</b>:<br/>"
      "\"NAME - {$resolution}.FILETYPE\"<br/>"
      "It work for any file name meet following resolution pattern.<br/>"
      "Given: [A - 480p, A - 720p, A - 1080p, A - 2160p, A - 4K]<br/>"
      "Result: only A - 2160p will be kept, and others will moved to trashbin.");

  _REMOVE_REDUNDANT_ITEMS->setToolTip(
      "<b>Remove redundant Files/Folders under current view path </b><br/>"
      "A folder with no item or only one item<br/>"
      "move the file (if it exists) to its upper level folder. And erase the redundant folder");
  _REMOVE_EMPTY_FOLDER->setToolTip(
      "<b>Remove Empty folders under current view path </b>"
      "Only Empty folder will be removed");
  _REMOVE_FOLDER_BY_KEYWORD->setToolTip(
      "<b>Remove Empty folders name contains keyword under current view path </b>"
      "Only folder items count <= 10 and not contains videos will be removed");
  _DUPLICATE_VIDEOS_FINDER->setToolTip(
      "<b>Differ videos by duration or filesize</b>"
      "to let it easy to find duplicate one");
  _DUPLICATE_VIDEOS_FINDER->setCheckable(true);

  _REDUNDANT_IMAGES_FINDER->setToolTip(
      "<b>Find empty or redundant images</b>"
      "to let it easy to find duplicate one");
  _REDUNDANT_IMAGES_FINDER->setCheckable(true);
}

FileBasicOperationsActions& g_fileBasicOperationsActions() {
  static FileBasicOperationsActions fileOpIns;
  return fileOpIns;
}

#include <QToolBar>

class FileOperationActionIllustration : public QToolBar {
 public:
  explicit FileOperationActionIllustration(const QString& title, QWidget* parent = nullptr) : QToolBar(title, parent) {
    addActions(g_fileBasicOperationsActions().OPEN_AG->actions());
    addActions(g_fileBasicOperationsActions().CUT_COPY_PASTE->actions());
    addActions(g_fileBasicOperationsActions().COPY_PATH_AG->actions());
    addActions(g_fileBasicOperationsActions().NEW->actions());
    addActions(g_fileBasicOperationsActions().DELETE_ACTIONS->actions());
    addActions(g_fileBasicOperationsActions().MOVE_COPY_TO->actions());
    addActions(g_fileBasicOperationsActions().SELECTION_RIBBONS->actions());
  }
};

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  FileOperationActionIllustration renameIllustration("Rename Items", nullptr);
  renameIllustration.show();
  return a.exec();
}
#endif
