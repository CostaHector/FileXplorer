#ifndef FILEBASICOPERATIONSACTIONS_H
#define FILEBASICOPERATIONSACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

struct KV;
class QToolBar;

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
  QAction *_REVEAL_IN_EXPLORER{nullptr}, *_OPEN_IN_TERMINAL{nullptr};
  QActionGroup* OPEN_AG{nullptr};

  QActionGroup* GetCOPY_PATHActions();
  QAction *COPY_FULL_PATH{nullptr}, *COPY_PATH{nullptr}, *COPY_NAME{nullptr}, *COPY_THE_PATH{nullptr}, *COPY_RECORDS{nullptr};
  QActionGroup* COPY_PATH_AG{nullptr};

  QActionGroup* GetNEWActions();
  QAction *NEW_FOLDER{nullptr}, *NEW_TEXT_FILE{nullptr}, *NEW_JSON_FILE{nullptr}, *BATCH_NEW_FILES{nullptr}, *BATCH_NEW_FOLDERS{nullptr};
  QActionGroup* NEW{nullptr};

  QActionGroup* Get_Advance_Search_Actions();
  QActionGroup* FolderMergeActions();

  void FolderFileCategoryProcess();
  QAction* _PACK_FOLDERS{nullptr};
  QAction* _UNPACK_FOLDERS{nullptr};
  QAction* _RETURN_ERROR_CODE_UPON_ANY_FAILURE{nullptr};
  QAction* _LONG_PATH_FINDER{nullptr};
  QAction* _RMV_EMPTY_FOLDER{nullptr};
  QAction* _RMV_01_FILE_FOLDER{nullptr};
  QAction* _RMV_FOLDER_BY_KEYWORD{nullptr};
  QAction* _DUPLICATE_IMAGES_FINDER{nullptr};
  QAction* _LOW_RESOLUTION_IMGS_RMV{nullptr};
  QAction* _DUPLICATE_VIDEOS_FINDER{nullptr};

  QAction *_MOVE_TO{nullptr}, *_COPY_TO{nullptr};
  QActionGroup* MOVE_COPY_TO{nullptr};
  QActionGroup* MOVE_TO_PATH_HISTORY{nullptr};
  QActionGroup* COPY_TO_PATH_HISTORY{nullptr};

  QToolBar* GetFolderOperationModeTb(QWidget* parent=nullptr);
  QAction *FILE_STRUCTURE_QRY_BEFORE_PASTE{nullptr}, *FILE_STRUCTURE_PRESERVE{nullptr}, *FILE_STRUCTURE_FLATTEN{nullptr};
  QActionGroup* FILE_STRUCTURE_AGS{nullptr};
  QActionGroup* FileStructureActions();

  QAction *MOVE_TO_TRASHBIN{nullptr}, *DELETE_PERMANENTLY{nullptr};
  QActionGroup* DELETE_ACTIONS{nullptr};

  QAction *UNDO_OPERATION{nullptr}, *REDO_OPERATION{nullptr};
  QActionGroup* UNDO_REDO_RIBBONS{nullptr};

  QToolBar* GetCutCopyPasteTb(QWidget* parent=nullptr);
  QAction *CUT{nullptr}, *COPY{nullptr}, *PASTE{nullptr};
  QActionGroup* CUT_COPY_PASTE{nullptr};

  QAction *MERGE{nullptr}, *MERGE_REVERSE{nullptr};
  QActionGroup* FOLDER_MERGE{nullptr};

  QAction* _TS_FILES_MERGE{nullptr};

  QAction *SELECT_ALL{nullptr}, *SELECT_NONE{nullptr}, *SELECT_INVERT{nullptr};
  QActionGroup* SELECTION_RIBBONS{nullptr};

  QAction* _FORCE_RESEARCH{nullptr};
};

FileBasicOperationsActions& g_fileBasicOperationsActions();
bool IsNeedQueryBeforePaste();

#endif  // FILEBASICOPERATIONSACTIONS_H
