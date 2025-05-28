#ifndef FILEBASICOPERATIONSACTIONS_H
#define FILEBASICOPERATIONSACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

#include "public/PublicVariable.h"
#include "public/MemoryKey.h"

class FileBasicOperationsActions : public QObject {
 public:
  explicit FileBasicOperationsActions(QObject* parent = nullptr);

  QActionGroup* GetDeleteActions();
  QActionGroup* GetMOVE_COPY_TOActions();
  QActionGroup* GetMOVE_COPY_TO_PATH_HistoryActions(const KV& memoryKey);
  QActionGroup* Get_CUT_COPY_PASTE_OPERATIONS_Actions();
  QActionGroup* Get_UNDO_REDO_OPERATIONS_Actions();
  QActionGroup* Get_SELECTION_RIBBON_Action();

  QActionGroup* GetOPENActions();
  QAction *_REVEAL_IN_EXPLORER, *_OPEN_IN_TERMINAL;
  QActionGroup* OPEN_AG;

  QActionGroup* GetCOPY_PATHActions();
  QAction *COPY_FULL_PATH, *COPY_PATH, *COPY_NAME, *COPY_THE_PATH, *COPY_RECORDS;
  QActionGroup* COPY_PATH_AG;

  QActionGroup* GetNEWActions();
  QAction *NEW_FOLDER, *NEW_TEXT_FILE, *NEW_JSON_FILE, *BATCH_NEW_FILES, *BATCH_NEW_FOLDERS;
  QActionGroup* NEW;

  QActionGroup* Get_Advance_Search_Actions();
  QActionGroup* FolderMergeActions();

  void FolderFileCategoryProcess();
  QAction* _NAME_RULER{new (std::nothrow) QAction(QIcon(":img/NAME_RULER"), "Name Ruler")};
  QAction* _PACK_FOLDERS{new (std::nothrow) QAction(QIcon(":img/PACK_FOLDERS"), "Packer")};
  QAction* _UNPACK_FOLDERS{new (std::nothrow) QAction(QIcon(":img/UNPACK_FOLDERS"), "Unpacker")};
  QAction* _LONG_PATH_FINDER{new (std::nothrow) QAction(QIcon(":img/LONG_PATH_FINDER"), "Long path finder")};
  QAction* _DUPLICATE_ITEMS_REMOVER{new (std::nothrow) QAction(QIcon(":img/DEDUPLICATE"), "Deduplicator")};
  QAction* _RMV_01_FILE_FOLDER{new (std::nothrow) QAction(QIcon(":img/ONE_FILE_IN_ONE_FOLDER"), "Rmv 0/1 file folders")};
  QAction* _RMV_EMPTY_FOLDER_R{new (std::nothrow) QAction(QIcon(":img/EMPTY_FOLDER"), "Rmv empty folders")};
  QAction* _RMV_FOLDER_BY_KEYWORD{new (std::nothrow) QAction(QIcon(""), "Rmv folders by keyword")};
  QAction* _DUPLICATE_VIDEOS_FINDER{new (std::nothrow) QAction(QIcon(":img/DUPLICATE_VIDEOS_FINDER"), "Duplicate Videos Finder")};
  QAction* _REDUNDANT_IMAGES_FINDER{new (std::nothrow) QAction(QIcon(":img/REDUNDANT_IMAGE_FINDER"), "Useless Images Finder")};

  QAction *_MOVE_TO, *_COPY_TO;
  QActionGroup* MOVE_COPY_TO;
  QActionGroup* MOVE_TO_PATH_HISTORY;
  QActionGroup* COPY_TO_PATH_HISTORY;

  QAction *MOVE_TO_TRASHBIN, *DELETE_PERMANENTLY;
  QActionGroup* DELETE_ACTIONS;

  QAction *UNDO_OPERATION, *REDO_OPERATION;
  QActionGroup* UNDO_REDO_RIBBONS;

  QAction *CUT, *COPY, *PASTE;
  QActionGroup* CUT_COPY_PASTE;

  QAction *MERGE, *MERGE_REVERSE;
  QActionGroup* FOLDER_MERGE;

  QAction *SELECT_ALL, *SELECT_NONE, *SELECT_INVERT;
  QActionGroup* SELECTION_RIBBONS;
};

FileBasicOperationsActions& g_fileBasicOperationsActions();

#endif  // FILEBASICOPERATIONSACTIONS_H
