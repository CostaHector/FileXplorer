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
        COPY_FULL_PATH{new QAction(QIcon(":/themes/COPY_FULL_PATH"), tr("Copy fullpath"))},
        COPY_PATH{new QAction(QIcon(":/themes/COPY_PATH"), tr("Copy path"))},
        COPY_NAME{new QAction(QIcon(":/themes/COPY_NAME"), tr("Copy name"))},

        COPY_THE_PATH{new QAction(QIcon(":/themes/COPY_THE_PATH"), tr("Copy the path"))},
        COPY_PATH_AG(GetCOPY_PATHActions()),
        NEW_FOLDER{new QAction(QIcon(":/themes/NEW_FOLDER"), tr("New folder"))},

        NEW_TEXT_FILE{new QAction(QIcon(":/themes/NEW_TEXT_DOCUMENT"), tr("New text"))},
        NEW_JSON_FILE{new QAction(QIcon(":/themes/NEW_JSON_FILE"), tr("New json"))},
        BATCH_NEW_FILES{new QAction(QIcon(":/themes/NEW_TEXT_DOCUMENTS"), tr("New Files"))},

        BATCH_NEW_FOLDERS{new QAction(QIcon(":/themes/NEW_FOLDERS"), tr("New Folders"))},
        NEW(GetNEWActions()),
        MOVE_COPY_TO(GetMOVE_COPY_TOActions()),

        MOVE_TO_PATH_HISTORY(GetMOVE_COPY_TO_PATH_HistoryActions(MemoryKey::MOVE_TO_PATH_HISTORY)),
        COPY_TO_PATH_HISTORY(GetMOVE_COPY_TO_PATH_HistoryActions(MemoryKey::COPY_TO_PATH_HISTORY)),
        MOVE_TO_TRASHBIN{new QAction(QIcon(":/themes/MOVE_TO_TRASH_BIN"), tr("Recycle"))},

        DELETE_PERMANENTLY{new QAction(QIcon(":/themes/DELETE_ITEMS_PERMANENTLY"), tr("Delete permanently"))},
        DELETE_ACTIONS(GetDeleteActions()),
        UNDO_OPERATION{new QAction(QIcon(":/themes/UNDO"), tr("Undo"), this)},
        REDO_OPERATION{new QAction(QIcon(":/themes/REDO"), tr("Redo"), this)},
        UNDO_REDO_RIBBONS(Get_UNDO_REDO_OPERATIONS_Actions()),

        CUT_COPY_MERGE_PASTE(Get_CUT_COPY_PASTE_OPERATIONS_Actions()),
        FOLDER_MERGE(FolderMergeActions()),
        SELECTION_RIBBONS(Get_SELECTION_RIBBON_Action()),
        _REVEAL_IN_EXPLORER{new QAction(QIcon(":/themes/REVEAL_IN_EXPLORER"), tr("Reveal in explorer"), this)},
        _OPEN_IN_TERMINAL{new QAction(QIcon(":/themes/OPEN_IN_TERMINAL"), tr("Open in terminal"), this)},
        OPEN_AG(GetOPENActions()),

        FOLDER_FILE_PROCESS(FolderFileCategoryProcess()),

        ADVANCE_SEARCH_RIBBON(Get_Advance_Search_Actions()) {}

  QActionGroup* GetDeleteActions();
  QActionGroup* GetMOVE_COPY_TOActions();
  QActionGroup* GetMOVE_COPY_TO_PATH_HistoryActions(GVarStr memoryKey);
  QActionGroup* Get_CUT_COPY_PASTE_OPERATIONS_Actions();
  QActionGroup* Get_UNDO_REDO_OPERATIONS_Actions();
  QActionGroup* Get_SELECTION_RIBBON_Action();

  QActionGroup* GetOPENActions();

  QActionGroup* GetCOPY_PATHActions();
  QAction *COPY_FULL_PATH, *COPY_PATH, *COPY_NAME, *COPY_THE_PATH;
  QActionGroup* COPY_PATH_AG;

  QActionGroup* GetNEWActions();
  QAction *NEW_FOLDER, *NEW_TEXT_FILE, *NEW_JSON_FILE, *BATCH_NEW_FILES, *BATCH_NEW_FOLDERS;
  QActionGroup* NEW;

  QActionGroup* Get_Advance_Search_Actions();
  QActionGroup* FolderMergeActions();
  QActionGroup* FolderFileCategoryProcess();

  QAction *_MOVE_TO, *_COPY_TO;
  QActionGroup* MOVE_COPY_TO;
  QActionGroup* MOVE_TO_PATH_HISTORY;
  QActionGroup* COPY_TO_PATH_HISTORY;

  QAction *MOVE_TO_TRASHBIN, *DELETE_PERMANENTLY;
  QActionGroup* DELETE_ACTIONS;

  QAction *UNDO_OPERATION, *REDO_OPERATION;
  QActionGroup* UNDO_REDO_RIBBONS;

  QActionGroup* CUT_COPY_MERGE_PASTE;
  QActionGroup* FOLDER_MERGE;
  QActionGroup* SELECTION_RIBBONS;

  QAction *_REVEAL_IN_EXPLORER, *_OPEN_IN_TERMINAL;
  QActionGroup* OPEN_AG;

  QActionGroup* FOLDER_FILE_PROCESS;

  QActionGroup* ADVANCE_SEARCH_RIBBON;
};

FileBasicOperationsActions& g_fileBasicOperationsActions();

#endif  // FILEBASICOPERATIONSACTIONS_H
