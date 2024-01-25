#ifndef FILEBASICOPERATIONSACTIONS_H
#define FILEBASICOPERATIONSACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

#include "PublicVariable.h"

class FileBasicOperationsActions : public QObject {
 public:
  explicit FileBasicOperationsActions(QObject* parent = nullptr);

  auto GetDeleteActions() -> QActionGroup* {
    QAction* MOVE_TO_TRASHBIN = new QAction(QIcon(":/themes/MOVE_TO_TRASH_BIN"), "Recycle");
    MOVE_TO_TRASHBIN->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_D));
    MOVE_TO_TRASHBIN->setShortcutVisibleInContextMenu(true);
    MOVE_TO_TRASHBIN->setToolTip(QString("<b>%1 (%2)</b><br/> Move the selected item(s) to the Recyle Bin.")
                                     .arg(MOVE_TO_TRASHBIN->text(), MOVE_TO_TRASHBIN->shortcut().toString()));

    QAction* DELETE_PERMANENTLY = new QAction(QIcon(":/themes/DELETE_ITEMS_PERMANENTLY"), "Delete permanently");
    DELETE_PERMANENTLY->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Delete));
    DELETE_PERMANENTLY->setShortcutVisibleInContextMenu(true);
    DELETE_PERMANENTLY->setToolTip(QString("<b>%1 (%2)</b><br/> Delete the selected item(s) permanently")
                                       .arg(DELETE_PERMANENTLY->text(), DELETE_PERMANENTLY->shortcut().toString()));

    QActionGroup* actionGroup = new QActionGroup(this);
    actionGroup->addAction(MOVE_TO_TRASHBIN);
    actionGroup->addAction(DELETE_PERMANENTLY);
    return actionGroup;
  }
  auto GetMOVE_COPY_TOActions() -> QActionGroup* {
    _MOVE_TO = new QAction(QIcon(":/themes/MV_TO_COMMAND_PATH"), "Move to");
    _MOVE_TO->setShortcutVisibleInContextMenu(true);
    _MOVE_TO->setToolTip(QString("<b>%1 (%2)</b><br/> Move the selected item(s) to the location one specified later")
                             .arg(_MOVE_TO->text(), _MOVE_TO->shortcut().toString()));

    _COPY_TO = new QAction(QIcon(":/themes/CP_TO_COMMAND_PATH"), "Copy to");
    _COPY_TO->setShortcutVisibleInContextMenu(true);
    _COPY_TO->setToolTip(QString("<b>%1 (%2)</b><br/> Copy the selected item(s) to the location one specified later")
                             .arg(_COPY_TO->text(), _COPY_TO->shortcut().toString()));

    QActionGroup* actionGroup = new QActionGroup(this);
    actionGroup->addAction(_MOVE_TO);
    actionGroup->addAction(_COPY_TO);
    return actionGroup;
  }
  auto GetMOVE_COPY_TO_PATH_HistoryActions(GVarStr memoryKey) -> QActionGroup* {
    QString historyStr = PreferenceSettings().value(memoryKey.name, memoryKey.v).toString();
    QStringList historyList = historyStr.split(MOVE_COPT_TO_PATH_STR_SEPERATOR);
    QActionGroup* actionGroup = new QActionGroup(this);
    for (const QString& path : historyList) {
      QAction* tempPath = new QAction(QIcon(":/themes/DRAG_FOLDERS"), path);
      tempPath->setCheckable(false);
      actionGroup->addAction(tempPath);
    }
    return actionGroup;
  }
  auto Get_CUT_COPY_PASTE_OPERATIONS_Actions() -> QActionGroup* {
    QAction* CUT = new QAction(QIcon(":/themes/CUT_ITEM"), "Cut");
    CUT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_X));
    CUT->setShortcutVisibleInContextMenu(true);
    CUT->setToolTip(QString("<b>%1 (%2)</b><br/> Copy the selected item(s) to the clipboard.").arg(CUT->text(), CUT->shortcut().toString()));

    QAction* COPY = new QAction(QIcon(":/themes/COPY_ITEM"), "Copy");
    COPY->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_C));
    COPY->setShortcutVisibleInContextMenu(true);
    COPY->setToolTip(QString("<b>%1 (%2)</b><br/> Move the selected item(s) to the clipboard.").arg(COPY->text(), COPY->shortcut().toString()));

    QAction* PASTE = new QAction(QIcon(":/themes/PASTE_ITEM"), "Paste");
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
  auto Get_UNDO_REDO_OPERATIONS_Actions() -> QActionGroup* {
    QAction* UNDO_OPERATION = new QAction(QIcon(":/themes/UNDO"), "Undo");
    UNDO_OPERATION->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Z));
    UNDO_OPERATION->setShortcutVisibleInContextMenu(true);
    UNDO_OPERATION->setToolTip(QString("<b>%1 (%2)</b><br/>").arg(UNDO_OPERATION->text(), UNDO_OPERATION->shortcut().toString()));

    QAction* REDO_OPERATION = new QAction(QIcon(":/themes/REDO"), "Redo");
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
  auto Get_SELECTION_RIBBON_Action() -> QActionGroup* {
    QAction* SELECT_ALL = new QAction(QIcon(":/themes/SELECT_ALL"), "Select all");
    //        SELECT_ALL->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_A));
    SELECT_ALL->setShortcutVisibleInContextMenu(true);
    SELECT_ALL->setToolTip(
        QString("<b>%1 (%2)</b><br/> Select all item(s) in this view.").arg(SELECT_ALL->text(), SELECT_ALL->shortcut().toString()));

    QAction* SELECT_NONE = new QAction(QIcon(":/themes/SELECT_NONE"), "Select none");
    SELECT_NONE->setShortcutVisibleInContextMenu(true);
    SELECT_NONE->setToolTip(QString("<b>%1 (%2)</b><br/> Clear all your selections.").arg(SELECT_NONE->text(), SELECT_NONE->shortcut().toString()));

    QAction* SELECT_INVERT = new QAction(QIcon(":/themes/SELECT_INVERT"), "Invert selection");
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

  auto GetPLAYActions() -> QActionGroup*;

  auto GetOPENActions() -> QActionGroup*;
  auto GetCOPY_PATHActions() -> QActionGroup* {
    QAction* COPY_FULL_PATH = new QAction(QIcon(":/themes/COPY_FULL_PATH"), "Copy fullpath");
    COPY_FULL_PATH->setToolTip(
        QString("<b>%1 (%2)</b><br/>. <b>path/name</b> <br/>Copy the absolute file name of the selected item(s) to the clipboard.")
            .arg(COPY_FULL_PATH->text(), COPY_FULL_PATH->shortcut().toString()));
    COPY_FULL_PATH->setShortcutVisibleInContextMenu(true);

    QAction* COPY_PATH = new QAction(QIcon(":/themes/COPY_PATH"), "Copy path");
    COPY_PATH->setToolTip(QString("<b>%1 (%2)</b><br/> <b>path</b>/name <br/>Copy the directory of the selected item(s) to the clipboard.")
                              .arg(COPY_PATH->text(), COPY_PATH->shortcut().toString()));
    COPY_PATH->setShortcutVisibleInContextMenu(true);

    QAction* COPY_NAME = new QAction(QIcon(":/themes/COPY_NAME"), "Copy name");
    COPY_NAME->setToolTip(QString("<b>%1 (%2)</b><br/> path/<b>name</b> <br/>Copy the name of the selected item(s) to the clipboard.")
                              .arg(COPY_NAME->text(), COPY_NAME->shortcut().toString()));
    COPY_NAME->setShortcutVisibleInContextMenu(true);

    QAction* COPY_THE_PATH = new QAction(QIcon(":/themes/COPY_THE_PATH"), "Copy the path");
    COPY_THE_PATH->setToolTip(QString("<b>%1 (%2)</b><br/> <b>pth/itemName.jpg</b> <br/>Given current selected item named 'itemName' and its path "
                                      "'pth', <br/>'pth/itemName.jpg' is copied to the clipboard.")
                                  .arg(COPY_THE_PATH->text(), COPY_THE_PATH->shortcut().toString()));
    COPY_THE_PATH->setShortcutVisibleInContextMenu(true);

    QActionGroup* actionGroup = new QActionGroup(this);
    actionGroup->addAction(COPY_FULL_PATH);
    actionGroup->addAction(COPY_PATH);
    actionGroup->addAction(COPY_NAME);
    actionGroup->addAction(COPY_THE_PATH);
    actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

    for (QAction* act : actionGroup->actions()) {
      act->setCheckable(false);
    }
    return actionGroup;
  }
  auto GetNEWActions() -> QActionGroup* {
    QAction* NEW_FOLDER = new QAction(QIcon(":/themes/NEW_FOLDER"), "New folder");
    NEW_FOLDER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_N));
    NEW_FOLDER->setShortcutVisibleInContextMenu(true);
    NEW_FOLDER->setToolTip(
        QString("<b>%1 (%2)</b><br/> Create a new folder in current view.").arg(NEW_FOLDER->text(), NEW_FOLDER->shortcut().toString()));
    NEW_FOLDER->setCheckable(false);

    QAction* NEW_TEXT_FILE = new QAction(QIcon(":/themes/NEW_TEXT_DOCUMENT"), "New text");
    NEW_TEXT_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_T));
    NEW_TEXT_FILE->setShortcutVisibleInContextMenu(true);
    NEW_TEXT_FILE->setToolTip(
        QString("<b>%1 (%2)</b><br/> Create a new text document in current view.").arg(NEW_TEXT_FILE->text(), NEW_TEXT_FILE->shortcut().toString()));
    NEW_TEXT_FILE->setCheckable(false);

    QAction* NEW_JSON_FILE = new QAction(QIcon(":/themes/NEW_JSON_FILE"), "New json");
    NEW_JSON_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_J));
    NEW_JSON_FILE->setShortcutVisibleInContextMenu(true);
    NEW_JSON_FILE->setToolTip(
        QString("<b>%1 (%2)</b><br/> Create a new json file in current view.").arg(NEW_JSON_FILE->text(), NEW_JSON_FILE->shortcut().toString()));
    NEW_JSON_FILE->setCheckable(false);

    QAction* BATCH_NEW_FILES = new QAction(QIcon(":/themes/NEW_TEXT_DOCUMENTS"), "Batch New Files");
    BATCH_NEW_FILES->setToolTip(QString("<b>%1 (%2)</b><br/>").arg(BATCH_NEW_FILES->text(), BATCH_NEW_FILES->shortcut().toString()) +
                                "Create a batch of files by specified pattern.<br/>"
                                "e.g. Given pattern:<br/>"
                                "\"Page %03d.txt$1$10\"<br/>"
                                "it will create ten text documents numbered by Page 001,002,...,009 respectively.");
    QAction* BATCH_NEW_FOLDERS = new QAction(QIcon(":/themes/NEW_FOLDERS"), "Batch New Folders");
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
  auto FolderMergeActions() -> QActionGroup* {
    QAction* MERGE = new QAction(QIcon(":/themes/FOLDER_MERGE_TO_FIRST"), "Merge into first folder");
    MERGE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_M));
    MERGE->setShortcutVisibleInContextMenu(true);
    MERGE->setToolTip(QString("<b>%1 (%2)</b><br/> Given folderA and folderB, B+=A").arg(MERGE->text(), MERGE->shortcut().toString()));

    QAction* MERGE_REVERSE = new QAction(QIcon(":/themes/FOLDER_MERGE_TO_LAST"), "Merge into last folder");
    MERGE_REVERSE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_M));
    MERGE_REVERSE->setShortcutVisibleInContextMenu(true);
    MERGE_REVERSE->setToolTip(
        QString("<b>%1 (%2)</b><br/> Given folderA and folderB, B+=A").arg(MERGE_REVERSE->text(), MERGE_REVERSE->shortcut().toString()));

    QActionGroup* actionGroup = new QActionGroup(this);
    actionGroup->addAction(MERGE);
    actionGroup->addAction(MERGE_REVERSE);
    return actionGroup;
  }
  auto FolderFileCategoryProcess() -> QActionGroup*;
  auto Get_Advance_Search_Actions() -> QActionGroup* {
    QAction* _ADVANCE_SEARCH = new QAction(QIcon(":/themes/SEARCH"), "Advance search");
    _ADVANCE_SEARCH->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_F));
    _ADVANCE_SEARCH->setShortcutVisibleInContextMenu(true);
    _ADVANCE_SEARCH->setToolTip(
        QString("<b>%1 (%2)</b><br/> Search by file name or file contents.").arg(_ADVANCE_SEARCH->text(), _ADVANCE_SEARCH->shortcut().toString()));
    _ADVANCE_SEARCH->setCheckable(false);

    QActionGroup* actionGroup = new QActionGroup(this);
    actionGroup->addAction(_ADVANCE_SEARCH);

    actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
    return actionGroup;
  }

  QAction* _MOVE_TO = nullptr;
  QAction* _COPY_TO = nullptr;
  QActionGroup* MOVE_COPY_TO;
  QActionGroup* MOVE_TO_PATH_HISTORY;
  QActionGroup* COPY_TO_PATH_HISTORY;

  QActionGroup* DELETE_ACTIONS;

  QActionGroup* UNDO_REDO_RIBBONS;
  QActionGroup* CUT_COPY_MERGE_PASTE;
  QActionGroup* FOLDER_MERGE;
  QActionGroup* SELECTION_RIBBONS;

  QAction* _PLAY_VIDEOS = nullptr;
  QActionGroup* PLAY_AG;

  QAction *_REVEAL_IN_EXPLORER = nullptr, *_OPEN_IN_TERMINAL = nullptr;
  QActionGroup* OPEN_AG;
  QActionGroup* COPY_PATH;
  QActionGroup* NEW;
  QActionGroup* FOLDER_FILE_PROCESS;

  QActionGroup* ADVANCE_SEARCH_RIBBON;
};

FileBasicOperationsActions& g_fileBasicOperationsActions();

#endif  // FILEBASICOPERATIONSACTIONS_H
