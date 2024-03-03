#include "PerformersTableView.h"

#include <QHeaderView>
#include "Actions/PerformersManagerActions.h"

PerformersTableView::PerformersTableView(QWidget* parent): CustomTableView("PERFORMERS_TABLE", parent) {
  BindMenu(g_performersManagerActions().GetRightClickMenu());
  AppendVerticalHeaderMenuAGS(g_performersManagerActions().GetVerAGS());
  AppendHorizontalHeaderMenuAGS(g_performersManagerActions().GetHorAGS());
}
