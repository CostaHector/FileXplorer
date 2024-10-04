#ifndef FILEBASICOPERATIONSACTIONS_H
#define FILEBASICOPERATIONSACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

#include "PublicVariable.h"

class FileBasicOperationsActions : public QObject {
  Q_OBJECT
 public:
  explicit FileBasicOperationsActions(QObject* parent = nullptr)
      : QObject(parent),
        _REVEAL_IN_EXPLORER{new QAction(QIcon(":/themes/REVEAL_IN_EXPLORER"), tr("Reveal in explorer"), this)},
        _OPEN_IN_TERMINAL{new QAction(QIcon(":/themes/OPEN_IN_TERMINAL"), tr("Open in terminal"), this)},
        OPEN_AG(GetOPENActions()),

        COPY_FULL_PATH{new QAction(QIcon(":/themes/COPY_FULL_PATH"), tr("Copy fullpath"))},
        COPY_PATH{new QAction(QIcon(":/themes/COPY_PATH"), tr("Copy path"))},
        COPY_NAME{new QAction(QIcon(":/themes/COPY_NAME"), tr("Copy name"))},

        COPY_THE_PATH{new QAction(QIcon(":/themes/COPY_THE_PATH"), tr("Copy the path"))},
        COPY_RECORDS{new QAction(QIcon(":/themes/COPY_RECORD"), tr("Copy records"))},
        COPY_PATH_AG(GetCOPY_PATHActions()),
        NEW_FOLDER{new QAction(QIcon(":/themes/NEW_FOLDER"), tr("New folder"))},

        NEW_TEXT_FILE{new QAction(QIcon(":/themes/NEW_TEXT_DOCUMENT"), tr("New text"))},
        NEW_JSON_FILE{new QAction(QIcon(":/themes/NEW_JSON_FILE"), tr("New json"))},
        BATCH_NEW_FILES{new QAction(QIcon(":/themes/NEW_TEXT_DOCUMENTS"), tr("New Files"))},

        BATCH_NEW_FOLDERS{new QAction(QIcon(":/themes/NEW_FOLDERS"), tr("New Folders"))},
        NEW(GetNEWActions()),

        _MOVE_TO{new QAction(QIcon(":/themes/MV_TO_COMMAND_PATH"), tr("Move to"))},
        _COPY_TO{new QAction(QIcon(":/themes/CP_TO_COMMAND_PATH"), tr("Copy to"))},
        MOVE_COPY_TO(GetMOVE_COPY_TOActions()),

        MOVE_TO_PATH_HISTORY(GetMOVE_COPY_TO_PATH_HistoryActions(MemoryKey::MOVE_TO_PATH_HISTORY)),
        COPY_TO_PATH_HISTORY(GetMOVE_COPY_TO_PATH_HistoryActions(MemoryKey::COPY_TO_PATH_HISTORY)),

        MOVE_TO_TRASHBIN{new QAction(QIcon(":/themes/MOVE_TO_TRASH_BIN"), tr("Recycle"))},
        DELETE_PERMANENTLY{new QAction(QIcon(":/themes/DELETE_ITEMS_PERMANENTLY"), tr("Delete permanently"))},
        DELETE_ACTIONS(GetDeleteActions()),

        UNDO_OPERATION{new QAction(QIcon(":/themes/UNDO"), tr("Undo"), this)},
        REDO_OPERATION{new QAction(QIcon(":/themes/REDO"), tr("Redo"), this)},
        UNDO_REDO_RIBBONS(Get_UNDO_REDO_OPERATIONS_Actions()),

        CUT{new QAction(QIcon(":/themes/CUT_ITEM"), tr("Cut"))},
        COPY{new QAction(QIcon(":/themes/COPY_ITEM"), tr("Copy"))},
        PASTE{new QAction(QIcon(":/themes/PASTE_ITEM"), tr("Paste"))},
        CUT_COPY_PASTE(Get_CUT_COPY_PASTE_OPERATIONS_Actions()),

        MERGE{new QAction(QIcon(":/themes/FOLDER_MERGE_TO_FIRST"), tr("Merged to front"))},
        MERGE_REVERSE{new QAction(QIcon(":/themes/FOLDER_MERGE_TO_LAST"), tr("Merge to back"))},
        FOLDER_MERGE(FolderMergeActions()),

        SELECT_ALL{new QAction(QIcon(":/themes/SELECT_ALL"), tr("Select all"))},
        SELECT_NONE{new QAction(QIcon(":/themes/SELECT_NONE"), tr("Select none"))},
        SELECT_INVERT{new QAction(QIcon(":/themes/SELECT_INVERT"), tr("Invert selection"))},
        SELECTION_RIBBONS(Get_SELECTION_RIBBON_Action()) {
    FolderFileCategoryProcess();
  }

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
  QAction* _NAME_STANDARDLIZER{new QAction(QIcon(":/themes/NAME_STANDARDLIZER_PATH"), tr("Name Ruler"))};
  QAction* _CLASSIFIER{new QAction(QIcon(":/themes/CATEGORIZER"), tr("Categorizer"))};
  QAction* _LONG_PATH_FINDER{new QAction(QIcon(":/themes/LONG_PATH_FINDER"), "Long path finder")};
  QAction* _DUPLICATE_ITEMS_REMOVER{new QAction(QIcon(":/themes/DEDUPLICATE"), tr("Deduplicator"))};
  QAction* _RMV_REDUN_PARENT_FOLDER{new QAction(QIcon(":/themes/ONE_FILE_IN_ONE_FOLDER"), tr("Rmv redundant parent folder"))};
  QAction* _RMV_EMPTY_FOLDER_R{new QAction(QIcon(":/themes/EMPTY_FOLDER"), tr("Rmv empty folder -r"))};
  QAction* _RMV_FOLDER_BY_KEYWORD{new QAction(QIcon(""), tr("Rmv by keyword"))};
  QAction* _DUPLICATE_VIDEOS_FINDER{new QAction(QIcon(":/themes/DUPLICATE_VIDEOS_FINDER"), "Duplicate Videos Finder")};
  QAction* _REDUNDANT_IMAGES_FINDER{new QAction(QIcon(":/themes/REDUNDANT_IMAGE_FINDER"), "Useless Images Finder")};

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
