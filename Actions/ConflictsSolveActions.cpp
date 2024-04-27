#include "ConflictsSolveActions.h"

ConflictsSolveActions& g_conflictSolveAct(){
  static ConflictsSolveActions ins;
  return ins;
}

ConflictsSolveActions::ConflictsSolveActions(QObject* parent) : QObject{parent} {
  REVERT_ACT->setCheckable(true);
  HIDE_NO_CONFLICT_ITEM->setCheckable(true);
  HIDE_NO_CONFLICT_ITEM->setChecked(true);
}
