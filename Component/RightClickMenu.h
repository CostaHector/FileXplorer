#ifndef RIGHTCLICKMENU_H
#define RIGHTCLICKMENU_H

#include <QMenu>
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/RenameActions.h"

class RightClickMenu : public QMenu {
 public:
  QAction* MOVE_TO_TRASHBIN;
  explicit RightClickMenu(const QString& title, QWidget* parent = nullptr) : QMenu(title, parent),
        MOVE_TO_TRASHBIN(g_fileBasicOperationsActions().DELETE_ACTIONS->actions()[0]){

    setToolTipsVisible(true);

    auto* NEW_MENU = GetNewMenu();
    auto* VIEW_MENU = GetViewMenu();

    addActions(g_fileBasicOperationsActions().OPEN->actions());
    addSeparator();
    addActions(g_fileBasicOperationsActions().COPY_PATH->actions());
    addSeparator();

    addSeparator();
    addMenu(NEW_MENU);
    addMenu(VIEW_MENU);
    addSeparator();
    addActions(g_fileBasicOperationsActions().MOVE_COPY_TO->actions());
    addSeparator();

    addActions(g_fileBasicOperationsActions().CUT_COPY_MERGE_PASTE->actions());
    addSeparator();
    addActions(g_fileBasicOperationsActions().FOLDER_MERGE->actions());
    addSeparator();

    addActions(g_fileBasicOperationsActions().DELETE_ACTIONS->actions());
    addMenu(GetRenameMenu());
  }
  auto GetNewMenu() -> QMenu* {
    auto* _newMenuLevel2 = new QMenu("New", this);
    _newMenuLevel2->setIcon(QIcon(":/themes/NEW_FILE_FOLDER_PATH"));
    _newMenuLevel2->setToolTipsVisible(true);
    _newMenuLevel2->addActions(g_fileBasicOperationsActions().NEW->actions());
    return _newMenuLevel2;
  }
  auto GetViewMenu() -> QMenu* {
    auto* viewMenuL2 = new QMenu("&View", this);
    viewMenuL2->setIcon(QIcon(":/themes/SORTING_FILE_FOLDER"));
    viewMenuL2->setToolTipsVisible(true);
    //  viewMenuL2->addActions(g_viewActions().SORT_INDICATOR_ORDER->actions());
    //  viewMenuL2->addSeparator();
    //  viewMenuL2->addActions(g_viewActions().SORT_INDICATOR_LOGICAL_INDEX->actions());
    //  viewMenuL2->addSeparator();
    //  viewMenuL2->addActions(g_viewActions().TRIPLE_VIEW->actions());
    //  viewMenuL2->addSeparator();
    //  viewMenuL2->addActions(g_viewActions().LISTVIEW_VIEWMODE->actions());
    //  viewMenuL2->addSeparator();
    //  viewMenuL2->addActions(g_viewActions().LISTVIEW_FLOW->actions());
    //  viewMenuL2->addSeparator();
    //  viewMenuL2->addActions(g_viewActions().LISTVIEW_GRIDSIZE_LEVEL->actions());
    //  viewMenuL2->addSeparator();
    return viewMenuL2;
  }

  auto GetRenameMenu() -> QMenu* {
    auto* renameMenuLevel2 = new QMenu("Rename", this);
    renameMenuLevel2->setIcon(QIcon(":/themes/RENAME_PATH"));
    renameMenuLevel2->addActions(g_renameAg().RENAME_RIBBONS->actions());
    return renameMenuLevel2;
  }
};

#endif  // RIGHTCLICKMENU_H
