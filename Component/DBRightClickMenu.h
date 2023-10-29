#ifndef DBRIGHTCLICKMENU_H
#define DBRIGHTCLICKMENU_H

#include "Actions/DataBaseActions.h"

#include <QMenu>
class DBRightClickMenu : public QMenu {
 public:
  DBRightClickMenu(const QString& title, QWidget* parent = nullptr) : QMenu(title, parent) {
    setToolTipsVisible(true);
    addActions(g_dbAct().DB_RIGHT_CLICK_MENU_AG->actions());
  }
};

#endif // DBRIGHTCLICKMENU_H
