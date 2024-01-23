#include "FileBasicOperationsActions.h"

FileBasicOperationsActions::FileBasicOperationsActions(QObject* parent)
    : QObject(parent),
      MOVE_COPY_TO(GetMOVE_COPY_TOActions()),
      MOVE_TO_PATH_HISTORY(GetMOVE_COPY_TO_PATH_HistoryActions(MemoryKey::MOVE_TO_PATH_HISTORY)),
      COPY_TO_PATH_HISTORY(GetMOVE_COPY_TO_PATH_HistoryActions(MemoryKey::COPY_TO_PATH_HISTORY)),

      DELETE_ACTIONS(GetDeleteActions()),

      UNDO_REDO_RIBBONS(Get_UNDO_REDO_OPERATIONS_Actions()),
      CUT_COPY_MERGE_PASTE(Get_CUT_COPY_PASTE_OPERATIONS_Actions()),
      FOLDER_MERGE(FolderMergeActions()),
      SELECTION_RIBBONS(Get_SELECTION_RIBBON_Action()),

      PLAY_AG(GetPLAYActions()),
      OPEN_AG(GetOPENActions()),
      COPY_PATH(GetCOPY_PATHActions()),
      NEW(GetNEWActions()),
      FOLDER_FILE_PROCESS(FolderFileCategoryProcess()),

      ADVANCE_SEARCH_RIBBON(Get_Advance_Search_Actions()) {}

auto FileBasicOperationsActions::GetPLAYActions() -> QActionGroup* {
  _VIDEO_PLAYER_EMBEDDED = new QAction(QIcon(":/themes/VIDEO_PLAYER"), "Play in embedded player");
  _VIDEO_PLAYER_EMBEDDED->setShortcutVisibleInContextMenu(true);
  _VIDEO_PLAYER_EMBEDDED->setToolTip(QString("<b>%1 (%2)</b><br/> Open the selected item in embedded video player.")
                                         .arg(_VIDEO_PLAYER_EMBEDDED->text(), _VIDEO_PLAYER_EMBEDDED->shortcut().toString()));

  _PLAY_VIDEOS = new QAction(QIcon(":/themes/PLAY_BUTTON_TRIANGLE"), "Play");
  _PLAY_VIDEOS->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Return));
  _PLAY_VIDEOS->setShortcutVisibleInContextMenu(true);
  _PLAY_VIDEOS->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                   "Play the selected item(s) in default system player.")
                               .arg(_PLAY_VIDEOS->text(), _PLAY_VIDEOS->shortcut().toString()));
  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(_VIDEO_PLAYER_EMBEDDED);
  actionGroup->addAction(_PLAY_VIDEOS);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  for (QAction* act : actionGroup->actions()) {
    act->setCheckable(false);
  }
  return actionGroup;
}

auto FileBasicOperationsActions::GetOPENActions() -> QActionGroup* {
  _REVEAL_IN_EXPLORER = new QAction(QIcon(":/themes/REVEAL_IN_EXPLORER"), "Reveal in explorer");
  _REVEAL_IN_EXPLORER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
  _REVEAL_IN_EXPLORER->setToolTip(QString("<b>%1 (%2)</b><br/> Reveal items in system file explorer")
                                      .arg(_REVEAL_IN_EXPLORER->text(), _REVEAL_IN_EXPLORER->shortcut().toString()));
  _REVEAL_IN_EXPLORER->setCheckable(false);

  _OPEN_IN_TERMINAL = new QAction(QIcon(":/themes/OPEN_IN_TERMINAL"), "Open in terminal");
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

auto FileBasicOperationsActions::FolderFileCategoryProcess() -> QActionGroup* {
  QAction* _NAME_STANDARDLIZER = new QAction(QIcon(":/themes/NAME_STANDARDLIZER_PATH"), "Name Ruler");
  QAction* _CLASSIFIER = new QAction(QIcon(":/themes/CATEGORIZER"), "Categorizer");
  QAction* _ITEM_ORGANIZER = new QAction(QIcon(":/themes/ITEMS_ORGANIZER"), "Organizer");
  QAction* _DUPLICATE_ITEMS_REMOVER = new QAction(QIcon(":/themes/DEDUPLICATE"), "Deduplicator");

  _NAME_STANDARDLIZER->setToolTip(
      "<b>Rename</b><br/>"
      "Given: [A..mp4, A (1).jpg, A -- 2.json]<br/>"
      "Result: [A.mp4, A - 1.jpg, A - 2.json]");
  _CLASSIFIER->setToolTip(
      "<b>Category</b><br/>"
      "Move [A.mp4, A.jpg, A.json]<br/>"
      "To Folder A");
  _ITEM_ORGANIZER->setToolTip(
      "<b>Organize</b><br/>"
      "Move [A - B.mp4, A - C.mp4, A - D.mp4]<br/>"
      "To Folder A");
  _DUPLICATE_ITEMS_REMOVER->setToolTip(
      "<b>Remove duplicates for item with pattern</b>:<br/>"
      "\"NAME - {$resolution}.FILETYPE\"<br/>"
      "It work for any file name meet following resolution pattern.<br/>"
      "Given: [A - 480p, A - 720p, A - 1080p, A - 2160p, A - 4K]<br/>"
      "Result: only A - 2160p will be kept, and others will moved to trashbin.");

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(_NAME_STANDARDLIZER);
  actionGroup->addAction(_CLASSIFIER);
  actionGroup->addAction(_ITEM_ORGANIZER);
  actionGroup->addAction(_DUPLICATE_ITEMS_REMOVER);
  return actionGroup;
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
