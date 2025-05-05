#include "PerformersTableView.h"

#include <QHeaderView>
#include "Actions/CastDBActions.h"

PerformersTableView::PerformersTableView(QWidget* parent): CustomTableView("PERFORMERS_TABLE", parent) {
  BindMenu(g_castAct().GetRightClickMenu());
  AppendVerticalHeaderMenuAGS(g_castAct().GetVerAGS());
  AppendHorizontalHeaderMenuAGS(g_castAct().GetHorAGS());
}
