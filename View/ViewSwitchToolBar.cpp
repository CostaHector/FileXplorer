#include "ViewSwitchToolBar.h"
#include "ViewActions.h"
#include "NotificatorMacro.h"

void ViewSwitchToolBar::subscribe() {
  auto* actGrp = mViewTypeIntAction.getActionGroup();
  connect(actGrp, &QActionGroup::triggered, this, &ViewSwitchToolBar::onPushNewViewIntoUndoStack);
  auto& viewInst = g_viewActions();
  connect(viewInst._VIEW_BACK_TO, &QAction::triggered, this, &ViewSwitchToolBar::onViewNavigateBackward);
  connect(viewInst._VIEW_FORWARD_TO, &QAction::triggered, this, &ViewSwitchToolBar::onViewNavigateForward);
}

void ViewSwitchToolBar::onPushNewViewIntoUndoStack(QAction* viewAct) {
  ViewTypeTool::ViewType newViewType = mViewTypeIntAction.act2Enum(viewAct);
  mViewRD.operator()(newViewType);
}

bool ViewSwitchToolBar::onViewNavigateBackward() {
  if (!mViewRD.undoViewAvailable()) {
    LOG_INFO_NP("[Info] Backward view unavailable", "Already at the earliest view in history");
    return false;
  }

  ViewTypeTool::ViewType vt = mViewRD.undo();
  if (!mViewTypeIntAction) {
    LOG_WARN_NP("[Error] Action group not initialized", "Cannot perform operation");
    return false;
  }

  QAction* checkedAction = mViewTypeIntAction.setCheckedIfActionExist(vt);
  if (checkedAction == nullptr) {
    LOG_WARN_P("[Error] No action mapped for view type", "ViewType: %s has no corresponding QAction", ViewTypeTool::c_str(vt));
    return false;
  }
  emit actionTriggered(checkedAction);
  return true;
}

bool ViewSwitchToolBar::onViewNavigateForward() {
  if (!mViewRD.redoViewAvailable()) {
    LOG_INFO_NP("[Info] Forward view unavailable", "Already at the most recent view in history");
    return false;
  }

  ViewTypeTool::ViewType vt = mViewRD.redo();
  if (!mViewTypeIntAction) {
    LOG_WARN_NP("[Error] Action group not initialized", "Cannot perform operation");
    return false;
  }

  QAction* checkedAction = mViewTypeIntAction.setCheckedIfActionExist(vt);
  if (checkedAction == nullptr) {
    LOG_WARN_P("[Error] No action mapped for view type", "ViewType: %s has no corresponding QAction", ViewTypeTool::c_str(vt));
    return false;
  }
  emit actionTriggered(checkedAction);
  return true;
}
