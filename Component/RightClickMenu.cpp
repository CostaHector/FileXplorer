#include "RightClickMenu.h"
#include "ArchiveFilesActions.h"
#include "FileOpActs.h"
#include "RenameActions.h"
#include "ViewActions.h"
#include "RightClickMenuActions.h"

RightClickMenu::RightClickMenu(const QString& title, QWidget* parent) //
  : QMenu{title, parent} {
  setToolTipsVisible(true);

  addAction(g_viewActions()._SYS_VIDEO_PLAYERS);
  addActions(FileOpActs::GetInst().OPEN_AG->actions());
  addSeparator();
  QMenu* renameMenu = addMenu(QIcon(":img/RENAME"), "&Rename");
  renameMenu->addActions(g_renameAg().RENAME_RIBBONS->actions());
  QMenu* newMenu = addMenu(QIcon(":img/NEW_FILE_FOLDER_PATH"), "&New");
  newMenu->addActions(FileOpActs::GetInst().NEW->actions());
  QMenu* copyPathMenu = addMenu(QIcon(":img/COPY_FULL_PATH"), "Copy Path");
  copyPathMenu->addActions(FileOpActs::GetInst().COPY_PATH_AG->actions());
  addSeparator();
  addActions(FileOpActs::GetInst().CUT_COPY_PASTE->actions());
  addActions(FileOpActs::GetInst().MOVE_COPY_TO->actions());
  addActions(FileOpActs::GetInst().FOLDER_MERGE->actions());
  addSeparator();
  addActions(FileOpActs::GetInst().DELETE_ACTIONS->actions());
  addSeparator();
  addAction(g_AchiveFilesActions().ARCHIVE_PREVIEW);
  addAction(g_rightClickActions()._CALC_MD5_ACT);
  addAction(g_rightClickActions()._PROPERTIES);
  addAction(g_rightClickActions()._FORCE_REFRESH_FILESYSTEMMODEL);
}
