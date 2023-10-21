#include "FileBasicOperationsActions.h"

FileBasicOperationsActions::FileBasicOperationsActions(QObject* parent)
    : QObject(parent),
      MOVE_COPY_TO(GetMOVE_COPY_TO_DELETEActions()),
      MOVE_TO_PATH_HISTORY(GetMOVE_COPY_TO_PATH_HistoryActions(MemoryKey::MOVE_TO_PATH_HISTORY)),
      COPY_TO_PATH_HISTORY(GetMOVE_COPY_TO_PATH_HistoryActions(MemoryKey::COPY_TO_PATH_HISTORY)),

      DELETE_ACTIONS(GetDeleteActions()),

      UNDO_REDO_RIBBONS(Get_UNDO_REDO_OPERATIONS_Actions()),
      CUT_COPY_MERGE_PASTE(Get_CUT_COPY_PASTE_OPERATIONS_Actions()),
      FOLDER_MERGE(FolderMergeActions()),
      SELECTION_RIBBONS(Get_SELECTION_RIBBON_Action()),

      OPEN(GetOPENActions()),
      COPY_PATH(GetCOPY_PATHActions()),
      NEW(GetNEWActions()),
      FOLDER_FILE_PROCESS(FolderFileCategoryProcess()),

      ADVANCE_SEARCH_RIBBON(Get_Advance_Search_Actions()) {}

FileBasicOperationsActions& g_fileBasicOperationsActions() {
  static FileBasicOperationsActions fileOpIns;
  return fileOpIns;
}

#include <QToolBar>

class FileOperationActionIllustration : public QToolBar {
 public:
  explicit FileOperationActionIllustration(const QString& title, QWidget* parent = nullptr) : QToolBar(title, parent) {
    addActions(g_fileBasicOperationsActions().OPEN->actions());
    addActions(g_fileBasicOperationsActions().CUT_COPY_MERGE_PASTE->actions());
    addActions(g_fileBasicOperationsActions().COPY_PATH->actions());
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
