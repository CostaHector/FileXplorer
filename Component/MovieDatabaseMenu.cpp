#include "MovieDatabaseMenu.h"

#include "FileOpActs.h"
#include "ViewActions.h"

MovieDatabaseMenu::MovieDatabaseMenu(const QString& title, QWidget* parent) : QMenu(title, parent) {
  setToolTipsVisible(true);
  addAction(g_viewActions()._SYS_VIDEO_PLAYERS);
  addActions(FileOpActs::GetInst().OPEN_AG->actions());
  addSeparator();
  addActions(FileOpActs::GetInst().COPY_PATH_AG->actions());
}
