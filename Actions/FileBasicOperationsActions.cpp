#include "FileBasicOperationsActions.h"
#include "FileOperation/FileOperatorPub.h"
#include "FileOperation/ComplexOperation.h"
#include "public/MemoryKey.h"
#include "public/StyleSheet.h"
#include <QToolBar>
#include <QApplication>
#include <QStyle>

FileBasicOperationsActions::FileBasicOperationsActions(QObject* parent)
    : QObject(parent)  //
{
  _REVEAL_IN_EXPLORER = new (std::nothrow) QAction(QIcon(":img/REVEAL_IN_EXPLORER"), "Reveal in explorer", this);
  _OPEN_IN_TERMINAL = new (std::nothrow) QAction(QIcon(":img/OPEN_IN_TERMINAL"), "Open in terminal", this);
  OPEN_AG = GetOPENActions();

  COPY_FULL_PATH = new (std::nothrow) QAction(QIcon(":img/COPY_FULL_PATH"), "Copy fullpath");
  COPY_PATH = new (std::nothrow) QAction(QIcon(":img/COPY_PATH"), "Copy path");
  COPY_NAME = new (std::nothrow) QAction(QIcon(":img/COPY_NAME"), "Copy name");

  COPY_THE_PATH = new (std::nothrow) QAction(QIcon(":img/COPY_THE_PATH"), "Copy the path");
  COPY_RECORDS = new (std::nothrow) QAction(QIcon(":img/COPY_RECORD"), "Copy records");
  COPY_PATH_AG = GetCOPY_PATHActions();
  NEW_FOLDER = new (std::nothrow) QAction(QIcon(":img/NEW_FOLDER"), "New folder");

  NEW_TEXT_FILE = new (std::nothrow) QAction(QIcon(":img/NEW_TEXT_DOCUMENT"), "New text");
  NEW_JSON_FILE = new (std::nothrow) QAction(QIcon(":img/NEW_JSON_FILE"), "New json");
  BATCH_NEW_FILES = new (std::nothrow) QAction(QIcon(":img/NEW_TEXT_DOCUMENTS"), "New Files");

  BATCH_NEW_FOLDERS = new (std::nothrow) QAction(QIcon(":img/NEW_FOLDERS"), "New Folders");
  NEW = GetNEWActions();

  _MOVE_TO = new (std::nothrow) QAction(QIcon(":img/MV_TO_COMMAND_PATH"), "Move to");
  _COPY_TO = new (std::nothrow) QAction(QIcon(":img/CP_TO_COMMAND_PATH"), "Copy to");
  MOVE_COPY_TO = GetMOVE_COPY_TOActions();

  MOVE_TO_PATH_HISTORY = GetMOVE_COPY_TO_PATH_HistoryActions(MemoryKey::MOVE_TO_PATH_HISTORY);
  COPY_TO_PATH_HISTORY = GetMOVE_COPY_TO_PATH_HistoryActions(MemoryKey::COPY_TO_PATH_HISTORY);

  using namespace ComplexOperation;
  FILE_STRUCTURE_QRY_BEFORE_PASTE = new (std::nothrow) QAction{QIcon(":img/FILE_STRUCTURE_QUERY"), FILE_STRUCTURE_MODE_STR[(int)FILE_STRUCTURE_MODE::QUERY]};
  FILE_STRUCTURE_PRESERVE = new (std::nothrow) QAction{QIcon(":img/FILE_STRUCTURE_PRESERVE"), FILE_STRUCTURE_MODE_STR[(int)FILE_STRUCTURE_MODE::PRESERVE]};
  FILE_STRUCTURE_FLATTEN = new (std::nothrow) QAction{QIcon(":img/FILE_STRUCTURE_FLATTEN"), FILE_STRUCTURE_MODE_STR[(int)FILE_STRUCTURE_MODE::FLATTEN]};
  FILE_STRUCTURE_AGS = FileStructureActions();

  MOVE_TO_TRASHBIN = new (std::nothrow) QAction(QIcon(":img/MOVE_TO_TRASH_BIN"), "Recycle");
  DELETE_PERMANENTLY = new (std::nothrow) QAction(QIcon(":img/DELETE_ITEMS_PERMANENTLY"), "Delete permanently");
  DELETE_ACTIONS = GetDeleteActions();

  UNDO_OPERATION = new (std::nothrow) QAction(QIcon(":img/UNDO"), "Undo", this);
  REDO_OPERATION = new (std::nothrow) QAction(QIcon(":img/REDO"), "Redo", this);
  UNDO_REDO_RIBBONS = Get_UNDO_REDO_OPERATIONS_Actions();

  CUT = new (std::nothrow) QAction(QIcon(":img/CUT_ITEM"), "Cut");
  COPY = new (std::nothrow) QAction(QIcon(":img/COPY_ITEM"), "Copy");
  PASTE = new (std::nothrow) QAction(QIcon(":img/PASTE_ITEM"), "Paste");
  CUT_COPY_PASTE = Get_CUT_COPY_PASTE_OPERATIONS_Actions();

  MERGE = new (std::nothrow) QAction(QIcon(":img/FOLDER_MERGE_TO_FIRST"), "Merged to front");
  MERGE_REVERSE = new (std::nothrow) QAction(QIcon(":img/FOLDER_MERGE_TO_LAST"), "Merge to back");
  FOLDER_MERGE = FolderMergeActions();

  SELECT_ALL = new (std::nothrow) QAction(QIcon(":img/SELECT_ALL"), "Select all");
  SELECT_NONE = new (std::nothrow) QAction(QIcon(":img/SELECT_NONE"), "Select none");
  SELECT_INVERT = new (std::nothrow) QAction(QIcon(":img/SELECT_INVERT"), "Invert selection");
  SELECTION_RIBBONS = Get_SELECTION_RIBBON_Action();

  _NAME_RULER = new (std::nothrow) QAction(QIcon(":img/NAME_RULER"), "Name Ruler");
  _PACK_FOLDERS = new (std::nothrow) QAction(QIcon(":img/PACK_FOLDERS"), "Packer");
  _UNPACK_FOLDERS = new (std::nothrow) QAction(QIcon(":img/UNPACK_FOLDERS"), "Unpacker");
  _RETURN_ERROR_CODE_UPON_ANY_FAILURE = new (std::nothrow) QAction(QIcon(":img/FAST_FAIL"), "Fast Fail");
  _LONG_PATH_FINDER = new (std::nothrow) QAction(QIcon(":img/LONG_PATH_FINDER"), "Long path finder");
  _RMV_EMPTY_FOLDER_R = new (std::nothrow) QAction(QIcon(":img/EMPTY_FOLDER"), "Rmv empty folders");
  _RMV_01_FILE_FOLDER = new (std::nothrow) QAction("Rmv 0/1 file folders");
  _RMV_FOLDER_BY_KEYWORD = new (std::nothrow) QAction("Rmv folders by keyword");
  _DUPLICATE_IMAGES_FINDER = new (std::nothrow) QAction(QIcon(":img/DUPLICATE_IMAGES_FINDER"), "Duplicate Images Finder");
  _LOW_RESOLUTION_IMGS_RMV = new (std::nothrow) QAction(QIcon(":img/LOW_RESOLUTION_IMGS_RMV"), "Low resolution imgs Rmv");
  _DUPLICATE_VIDEOS_FINDER = new (std::nothrow) QAction(QIcon(":img/DUPLICATE_VIDEOS_FINDER"), "Duplicate Videos Finder");
  FolderFileCategoryProcess();
}

QActionGroup* FileBasicOperationsActions::GetDeleteActions() {
  MOVE_TO_TRASHBIN->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_D));
  MOVE_TO_TRASHBIN->setShortcutVisibleInContextMenu(true);
  MOVE_TO_TRASHBIN->setToolTip(QString("<b>%1 (%2)</b><br/> Move the selected item(s) to the Recyle Bin.").arg(MOVE_TO_TRASHBIN->text(), MOVE_TO_TRASHBIN->shortcut().toString()));

  DELETE_PERMANENTLY->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Delete));
  DELETE_PERMANENTLY->setShortcutVisibleInContextMenu(true);
  DELETE_PERMANENTLY->setToolTip(QString("<b>%1 (%2)</b><br/> Delete the selected item(s) permanently").arg(DELETE_PERMANENTLY->text(), DELETE_PERMANENTLY->shortcut().toString()));

  QActionGroup* actionGroup = new (std::nothrow) QActionGroup(this);
  actionGroup->addAction(MOVE_TO_TRASHBIN);
  actionGroup->addAction(DELETE_PERMANENTLY);
  return actionGroup;
}

QActionGroup* FileBasicOperationsActions::GetMOVE_COPY_TOActions() {
  _MOVE_TO->setShortcutVisibleInContextMenu(true);
  _MOVE_TO->setToolTip(QString("<b>%1 (%2)</b><br/> Move the selected item(s) to the location one specified later").arg(_MOVE_TO->text(), _MOVE_TO->shortcut().toString()));
  _COPY_TO->setShortcutVisibleInContextMenu(true);
  _COPY_TO->setToolTip(QString("<b>%1 (%2)</b><br/> Copy the selected item(s) to the location one specified later").arg(_COPY_TO->text(), _COPY_TO->shortcut().toString()));

  QActionGroup* actionGroup = new (std::nothrow) QActionGroup(this);
  actionGroup->addAction(_MOVE_TO);
  actionGroup->addAction(_COPY_TO);
  return actionGroup;
}

QActionGroup* FileBasicOperationsActions::GetMOVE_COPY_TO_PATH_HistoryActions(const KV& memoryKey) {
  QString historyStr = PreferenceSettings().value(memoryKey.name, memoryKey.v).toString();
  QStringList historyList = historyStr.split('\n');
  QActionGroup* actionGroup = new (std::nothrow) QActionGroup(this);
  for (const QString& path : historyList) {
    QAction* tempPath = new (std::nothrow) QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirIcon), path);
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
  PASTE->setToolTip(QString("<b>%1 (%2)</b><br/> Paste the contents of clipboard to the current location.").arg(PASTE->text(), PASTE->shortcut().toString()));

  QActionGroup* actionGroup = new (std::nothrow) QActionGroup(this);
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

  QActionGroup* actionGroup = new (std::nothrow) QActionGroup(this);
  actionGroup->addAction(UNDO_OPERATION);
  actionGroup->addAction(REDO_OPERATION);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  foreach (QAction* act, actionGroup->actions()) {
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

  QActionGroup* actionGroup = new (std::nothrow) QActionGroup(this);
  actionGroup->addAction(SELECT_ALL);
  actionGroup->addAction(SELECT_NONE);
  actionGroup->addAction(SELECT_INVERT);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  foreach (QAction* act, actionGroup->actions()) {
    act->setCheckable(false);
  }
  return actionGroup;
}

auto FileBasicOperationsActions::GetOPENActions() -> QActionGroup* {
  _REVEAL_IN_EXPLORER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
  _REVEAL_IN_EXPLORER->setToolTip(QString("<b>%1 (%2)</b><br/> Reveal items in system file explorer").arg(_REVEAL_IN_EXPLORER->text(), _REVEAL_IN_EXPLORER->shortcut().toString()));

  _OPEN_IN_TERMINAL->setShortcut(QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_T));
  _OPEN_IN_TERMINAL->setShortcutVisibleInContextMenu(true);

  _OPEN_IN_TERMINAL->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                        "Open current view path in terminal")
                                    .arg(_OPEN_IN_TERMINAL->text(), _OPEN_IN_TERMINAL->shortcut().toString()));

  QActionGroup* actionGroup = new (std::nothrow) QActionGroup(this);
  actionGroup->addAction(_REVEAL_IN_EXPLORER);
  actionGroup->addAction(_OPEN_IN_TERMINAL);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  foreach (QAction* act, actionGroup->actions()) {
    act->setCheckable(false);
  }
  return actionGroup;
}

QActionGroup* FileBasicOperationsActions::GetCOPY_PATHActions() {
  COPY_FULL_PATH->setShortcut(QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_Insert));
  COPY_FULL_PATH->setToolTip(
      QString("<b>%1 (%2)</b><br/>. <b>path/name</b> <br/>Copy the absolute file name of the selected item(s) to the clipboard.").arg(COPY_FULL_PATH->text(), COPY_FULL_PATH->shortcut().toString()));
  COPY_FULL_PATH->setShortcutVisibleInContextMenu(true);

  COPY_PATH->setToolTip(QString("<b>%1 (%2)</b><br/> <b>path</b>/name <br/>Copy the directory of the selected item(s) to the clipboard.").arg(COPY_PATH->text(), COPY_PATH->shortcut().toString()));
  COPY_PATH->setShortcutVisibleInContextMenu(true);

  COPY_NAME->setShortcut(QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_Insert));
  COPY_NAME->setToolTip(QString("<b>%1 (%2)</b><br/> path/<b>name</b> <br/>Copy the name of the selected item(s) to the clipboard.").arg(COPY_NAME->text(), COPY_NAME->shortcut().toString()));
  COPY_NAME->setShortcutVisibleInContextMenu(true);

  COPY_THE_PATH->setToolTip(QString("<b>%1 (%2)</b><br/> <b>pth/itemName.jpg</b> <br/>Given current selected item named 'itemName' and its path "
                                    "'pth', <br/>'pth/itemName.jpg' is copied to the clipboard.")
                                .arg(COPY_THE_PATH->text(), COPY_THE_PATH->shortcut().toString()));
  COPY_THE_PATH->setShortcutVisibleInContextMenu(true);

  COPY_RECORDS->setShortcut(QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_C));
  COPY_RECORDS->setToolTip(QString("<b>%1 (%2)</b><br/> Copy the selected records, especially name, size, and prepath").arg(COPY_RECORDS->text(), COPY_RECORDS->shortcut().toString()));
  COPY_RECORDS->setShortcutVisibleInContextMenu(true);

  QActionGroup* actionGroup = new (std::nothrow) QActionGroup(this);
  actionGroup->addAction(COPY_FULL_PATH);
  actionGroup->addAction(COPY_PATH);
  actionGroup->addAction(COPY_NAME);
  actionGroup->addAction(COPY_THE_PATH);
  actionGroup->addAction(COPY_RECORDS);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  foreach (QAction* act, actionGroup->actions()) {
    act->setCheckable(false);
  }
  return actionGroup;
}

QActionGroup* FileBasicOperationsActions::GetNEWActions() {
  NEW_FOLDER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_N));
  NEW_FOLDER->setShortcutVisibleInContextMenu(true);
  NEW_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/> Create a new folder in current view.").arg(NEW_FOLDER->text(), NEW_FOLDER->shortcut().toString()));
  NEW_FOLDER->setCheckable(false);

  NEW_TEXT_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_T));
  NEW_TEXT_FILE->setShortcutVisibleInContextMenu(true);
  NEW_TEXT_FILE->setToolTip(QString("<b>%1 (%2)</b><br/> Create a new text document in current view.").arg(NEW_TEXT_FILE->text(), NEW_TEXT_FILE->shortcut().toString()));
  NEW_TEXT_FILE->setCheckable(false);

  NEW_JSON_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_J));
  NEW_JSON_FILE->setShortcutVisibleInContextMenu(true);
  NEW_JSON_FILE->setToolTip(QString("<b>%1 (%2)</b><br/> Create a new json file in current view.").arg(NEW_JSON_FILE->text(), NEW_JSON_FILE->shortcut().toString()));
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

  QActionGroup* actionGroup = new (std::nothrow) QActionGroup(this);
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
  MERGE_REVERSE->setToolTip(QString("<b>%1 (%2)</b><br/> Given folderA and folderB, B+=A").arg(MERGE_REVERSE->text(), MERGE_REVERSE->shortcut().toString()));

  QActionGroup* actionGroup = new (std::nothrow) QActionGroup(this);
  actionGroup->addAction(MERGE);
  actionGroup->addAction(MERGE_REVERSE);
  return actionGroup;
}

void FileBasicOperationsActions::FolderFileCategoryProcess() {
  _NAME_RULER->setToolTip(
      "<b>Standardized Files/Folders Name under current view path</b><br/>"
      "Given: [A..mp4, A (1).jpg, A -- 2.json]<br/>"
      "Result: [A.mp4, A - 1.jpg, A - 2.json]");
  _PACK_FOLDERS->setToolTip(
      "<b>Category Files/Folders Name under current view path</b><br/>"
      "Move [A.mp4, A.jpg, A.json]<br/>"
      "To Folder A");
  _UNPACK_FOLDERS->setToolTip(
      "<b>Unpack items from folders to current view path</b><br/>"
      "Move 3 item(s) under path/{A.mp4, A.jpg, A.json}<br/>"
      "To path");
  _RETURN_ERROR_CODE_UPON_ANY_FAILURE->setCheckable(true);
  _RETURN_ERROR_CODE_UPON_ANY_FAILURE->setToolTip(
      "<b>Fast fail</b><br/>"
      "When enabled, <b>immediately return</b> error code if any command fails.<br/>"
      "Otherwise, <b>continue executing</b> remaining commands despite partial failures.");
  _LONG_PATH_FINDER->setToolTip(
      "<b>Long path finder</b><br/>"
      "find out all too long path(s), then chop one section from full path, say the second to last section");
  _LOW_RESOLUTION_IMGS_RMV->setToolTip(
      "<b>Remove Files/Folders whose names in specified pattern under current view path</b><br/>"
      "NAME - {$resolution}.FILETYPE<br/>"
      "It work for any file name meet following resolution pattern.<br/>"
      "Given: [A - 480p, A - 720p, A - 1080p, A - 2160p, A - 4K]<br/>"
      "Result: only A - 2160p will be kept, and others will moved to trashbin.");

  _RMV_01_FILE_FOLDER->setToolTip(
      "<b>Remove empty folders or 1 file folders under current view path</b><br/>"
      "1. Delete empty folders;<br/>"
      "2. If a folder contains only a SINGLE file and the difference between the file name length and the folder name length <= 5 characters.<br/>"
      "Move the file up one level (to the parent directory) and then delete the folder.");
  _RMV_EMPTY_FOLDER_R->setToolTip(
      "<b>Remove Empty folders under current view path recusively</b><br/>"
      "Only Empty folder will be removed");
  _RMV_FOLDER_BY_KEYWORD->setToolTip(
      "<b>Remove folders whose name contain the specified keyword under current view path </b><br/>"
      "1. skip if folder contains sub directory;<br/>"
      "2. skip if folder items count > 10;<br/>"
      "3. skip if folder contains videos;");
  _DUPLICATE_VIDEOS_FINDER->setToolTip(
      "<b>Differ videos by duration or filesize</b><br/>"
      "Let it easy to find duplicate videos from videos size/duration/file name dimension.");
  _DUPLICATE_VIDEOS_FINDER->setCheckable(true);
  _DUPLICATE_IMAGES_FINDER->setToolTip(
      "<b>Find empty or duplicate images</b><br/>"
      "Let it easy to operate on empty or duplicate(already exist) images");
  _DUPLICATE_IMAGES_FINDER->setCheckable(true);

  FileOperatorType::InitReturnErrorCodeUponAnyFailureSw();
  const bool bFastFail = FileOperatorType::IsReturnErrorCodeUponAnyFailureSw();
  _RETURN_ERROR_CODE_UPON_ANY_FAILURE->setChecked(bFastFail);
  connect(_RETURN_ERROR_CODE_UPON_ANY_FAILURE, &QAction::triggered, &FileOperatorType::SetReturnErrorCodeUponAnyFailureSw);
}

QActionGroup* FileBasicOperationsActions::FileStructureActions() {
  FILE_STRUCTURE_QRY_BEFORE_PASTE->setToolTip("Query file structure before procceed each paste operation.");
  FILE_STRUCTURE_QRY_BEFORE_PASTE->setCheckable(true);
  FILE_STRUCTURE_PRESERVE->setToolTip("Set preserve file structure by default");
  FILE_STRUCTURE_PRESERVE->setCheckable(true);
  FILE_STRUCTURE_FLATTEN->setToolTip("Set flatten file structure by default");
  FILE_STRUCTURE_FLATTEN->setCheckable(true);

  auto* FILE_STRUCTURE_AGS = new (std::nothrow) QActionGroup{this};
  FILE_STRUCTURE_AGS->addAction(FILE_STRUCTURE_QRY_BEFORE_PASTE);
  FILE_STRUCTURE_AGS->addAction(FILE_STRUCTURE_PRESERVE);
  FILE_STRUCTURE_AGS->addAction(FILE_STRUCTURE_FLATTEN);
  FILE_STRUCTURE_AGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  int fileStructureWay = PreferenceSettings()
                             .value(MemoryKey::FILE_SYSTEM_STRUCTURE_WAY.name,  //
                                    MemoryKey::FILE_SYSTEM_STRUCTURE_WAY.v)
                             .toInt();
  const QList<QAction*> acts = FILE_STRUCTURE_AGS->actions();
  if (fileStructureWay < 0 || fileStructureWay >= acts.size()) {
    qWarning("FileSystemStructure Way set 0 instead");
    fileStructureWay = 0;
  }
  acts[fileStructureWay]->setChecked(true);
  ComplexOperation::SetDefaultFileStructMode(acts[fileStructureWay]);

  connect(FILE_STRUCTURE_AGS, &QActionGroup::triggered, &ComplexOperation::SetDefaultFileStructMode);
  return FILE_STRUCTURE_AGS;
}

QToolBar* FileBasicOperationsActions::GetFolderOperationModeTb(QWidget* parent) {
  auto* folderOperationModeTB = new (std::nothrow) QToolBar{"File System Structure Mode", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(folderOperationModeTB);
  folderOperationModeTB->setOrientation(Qt::Orientation::Vertical);
  folderOperationModeTB->addActions(g_fileBasicOperationsActions().CUT_COPY_PASTE->actions());
  folderOperationModeTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  folderOperationModeTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  folderOperationModeTB->setStyleSheet("QToolBar { max-width: 256px; }");
  SetLayoutAlightment(folderOperationModeTB->layout(), Qt::AlignmentFlag::AlignLeft);
  return folderOperationModeTB;
}

QToolBar* FileBasicOperationsActions::GetCutCopyPasteTb(QWidget* parent) {
  auto* cutCopyPaste = new (std::nothrow) QToolBar{"Copy/Cut/Paste", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(cutCopyPaste);
  cutCopyPaste->setOrientation(Qt::Orientation::Vertical);
  cutCopyPaste->addActions(g_fileBasicOperationsActions().FILE_STRUCTURE_AGS->actions());
  cutCopyPaste->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  cutCopyPaste->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  cutCopyPaste->setStyleSheet("QToolBar { max-width: 256px; }");
  SetLayoutAlightment(cutCopyPaste->layout(), Qt::AlignmentFlag::AlignLeft);
  return cutCopyPaste;
}

FileBasicOperationsActions& g_fileBasicOperationsActions() {
  static FileBasicOperationsActions fileOpIns;
  return fileOpIns;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
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

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  FileOperationActionIllustration renameIllustration("Rename Items", nullptr);
  renameIllustration.show();
  return a.exec();
}
#endif
