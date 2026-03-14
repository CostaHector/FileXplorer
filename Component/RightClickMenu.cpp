#include "RightClickMenu.h"
#include "ArchiveFilesActions.h"
#include "FileOpActs.h"
#include "RenameActions.h"
#include "ViewActions.h"
#include "RightClickMenuActions.h"
#include <QMenu>

QAction* GetSeperator(QWidget* parent) {
  QAction* sep = new (std::nothrow) QAction(parent);
  sep->setSeparator(true);
  return sep;
}

QList<QAction*> GetRightClickMenuActions(QWidget* parent) {
  CHECK_NULLPTR_RETURN_INT(parent, {});

  QList<QAction*> actions;
  actions.push_back(g_viewActions()._SYS_VIDEO_PLAYERS);
  actions += FileOpActs::GetInst().OPEN_AG->actions();
  actions.push_back(GetSeperator(parent));

  {
    QMenu* renameMenu = new QMenu("&Rename", parent);
    renameMenu->setIcon(QIcon(":img/RENAME"));
    renameMenu->addActions(g_renameAg().RENAME_RIBBONS->actions());
    actions.push_back(renameMenu->menuAction());
  }
  {
    QMenu* newMenu = new QMenu("&New", parent);
    newMenu->setIcon(QIcon(":img/NEW_FILE_FOLDER_PATH"));
    newMenu->addActions(FileOpActs::GetInst().NEW->actions());
    actions.push_back(newMenu->menuAction());
  }
  {
    QMenu* copyPathMenu = new QMenu("Copy Path", parent);
    copyPathMenu->setIcon(QIcon(":img/COPY_FULL_PATH"));
    copyPathMenu->addActions(FileOpActs::GetInst().COPY_PATH_AG->actions());
    actions.push_back(copyPathMenu->menuAction());
  }
  actions.push_back(GetSeperator(parent));

  actions += FileOpActs::GetInst().CUT_COPY_PASTE->actions();
  actions += FileOpActs::GetInst().MOVE_COPY_TO->actions();
  actions += FileOpActs::GetInst().FOLDER_MERGE->actions();
  actions.push_back(GetSeperator(parent));

  actions += FileOpActs::GetInst().DELETE_ACTIONS->actions();
  actions.push_back(GetSeperator(parent));

  actions.push_back(g_AchiveFilesActions().ARCHIVE_PREVIEW);
  actions.push_back(g_rightClickActions()._CALC_MD5_ACT);
  actions.push_back(g_rightClickActions()._PROPERTIES);
  actions.push_back(g_rightClickActions()._FORCE_REFRESH_FILESYSTEMMODEL);
  return actions;
}
