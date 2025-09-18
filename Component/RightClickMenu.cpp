#include "RightClickMenu.h"
#include "ArchiveFilesActions.h"
#include "FileBasicOperationsActions.h"
#include "RenameActions.h"
#include "ViewActions.h"
#include "RightClickMenuActions.h"

RightClickMenu::RightClickMenu(const QString& title, QWidget* parent)  //
    : QMenu{title, parent},                                            //
      NEW_MENU{GetNewMenu()}                                           //
{
  setToolTipsVisible(true);

  addAction(g_viewActions()._SYS_VIDEO_PLAYERS);
  addActions(g_fileBasicOperationsActions().OPEN_AG->actions());
  addAction(g_AchiveFilesActions().ARCHIVE_PREVIEW);
  addSeparator();
  addActions(g_fileBasicOperationsActions().COPY_PATH_AG->actions());
  addSeparator();

  addMenu(NEW_MENU);
  addSeparator();
  addActions(g_fileBasicOperationsActions().MOVE_COPY_TO->actions());
  addSeparator();

  addActions(g_fileBasicOperationsActions().CUT_COPY_PASTE->actions());
  addSeparator();
  addActions(g_fileBasicOperationsActions().FOLDER_MERGE->actions());
  addSeparator();

  addActions(g_fileBasicOperationsActions().DELETE_ACTIONS->actions());
  addMenu(GetRenameMenu());
  addSeparator();
  addAction(g_rightClickActions()._CALC_MD5_ACT);
  addAction(g_rightClickActions()._PROPERTIES);
  addAction(g_rightClickActions()._FORCE_REFRESH_FILESYSTEMMODEL);
}

QMenu* RightClickMenu::GetNewMenu() {
  auto* _newMenuLevel2 = new (std::nothrow) QMenu{"&New", this};
  _newMenuLevel2->setIcon(QIcon(":img/NEW_FILE_FOLDER_PATH"));
  _newMenuLevel2->setToolTipsVisible(true);
  _newMenuLevel2->addActions(g_fileBasicOperationsActions().NEW->actions());
  return _newMenuLevel2;
}

QMenu* RightClickMenu::GetRenameMenu() {
  auto* renameMenuLevel2 = new QMenu("&Rename", this);
  renameMenuLevel2->setIcon(QIcon(":img/RENAME"));
  renameMenuLevel2->addActions(g_renameAg().RENAME_RIBBONS->actions());
  return renameMenuLevel2;
}
