#include "MovieDatabaseMenu.h"

#include "Actions/DataBaseActions.h"
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/ViewActions.h"

MovieDatabaseMenu::MovieDatabaseMenu(const QString& title, QWidget* parent) : QMenu(title, parent) {
  setToolTipsVisible(true);
  addActions(g_viewActions()._VIDEO_PLAYERS->actions());
  addActions(g_fileBasicOperationsActions().OPEN_AG->actions());
  addSeparator();
  addActions(g_fileBasicOperationsActions().COPY_PATH_AG->actions());
  addSeparator();
  addActions({g_dbAct().DELETE_BY_DRIVER, g_dbAct().DELETE_BY_PREPATH});
}
