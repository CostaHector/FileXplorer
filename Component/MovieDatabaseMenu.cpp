#include "MovieDatabaseMenu.h"

#include "Actions/FileBasicOperationsActions.h"
#include "Actions/ViewActions.h"

MovieDatabaseMenu::MovieDatabaseMenu(const QString& title, QWidget* parent) : QMenu(title, parent) {
  setToolTipsVisible(true);
  addAction(g_viewActions()._SYS_VIDEO_PLAYERS);
  addActions(g_fileBasicOperationsActions().OPEN_AG->actions());
  addSeparator();
  addActions(g_fileBasicOperationsActions().COPY_PATH_AG->actions());
}
