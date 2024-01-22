#ifndef DBRIGHTCLICKMENU_H
#define DBRIGHTCLICKMENU_H

#include "Actions/DataBaseActions.h"

#include <QMenu>
class DBRightClickMenu : public QMenu {
 public:
  DBRightClickMenu(const QString& title, QWidget* parent = nullptr) : QMenu(title, parent) {
    setToolTipsVisible(true);

    addActions({g_dbAct().OPEN_RUN, g_dbAct()._PLAY_VIDEOS, g_dbAct()._REVEAL_IN_EXPLORER});
    addSeparator();
    addActions({g_dbAct().COPY_DB_ITEM_NAME, g_dbAct().COPY_DB_ITEM_FULL_PATH});
    addSeparator();
    addActions({g_dbAct().DELETE_BY_DRIVER, g_dbAct().DELETE_BY_PREPATH});
  }
};

#endif  // DBRIGHTCLICKMENU_H
