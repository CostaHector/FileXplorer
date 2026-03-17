#include "ViewSwitchToolBar.h"
#include "ViewActions.h"
#include "NotificatorMacro.h"
#include "ImageTool.h"
#include "StyleSheet.h"
#include "ViewTypeTool.h"
#include "ViewActions.h"

ViewSwitchToolBar::ViewSwitchToolBar(const QString& title, QWidget* parent) : QToolBar{title, parent} {
  auto& inst = g_viewActions();
  mViewTypeIntAction.init({{inst._LIST_VIEW, ViewTypeTool::ViewType::LIST},
                           {inst._TABLE_VIEW, ViewTypeTool::ViewType::TABLE},
                           {inst._TREE_VIEW, ViewTypeTool::ViewType::TREE},
                           {inst._MOVIE_VIEW, ViewTypeTool::ViewType::MOVIE},
                           {inst._CAST_VIEW, ViewTypeTool::ViewType::CAST},
                           {inst._SCENE_VIEW, ViewTypeTool::ViewType::SCENE},
                           {inst._JSON_VIEW, ViewTypeTool::ViewType::JSON},
                           {inst._ADVANCE_SEARCH_VIEW, ViewTypeTool::ViewType::SEARCH}},
                          ViewTypeTool::DEFAULT_VIEW_TYPE, QActionGroup::ExclusionPolicy::Exclusive);
  addActions(mViewTypeIntAction.getActionEnumAscendingList());  // action sorted in user specified sequence
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  setOrientation(Qt::Orientation::Horizontal);
  setStyleSheet("QToolBar { max-width: 256px; }");
  setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));

  subscribe();
}

ViewTypeTool::ViewType ViewSwitchToolBar::GetCurViewType() const {
  return mViewTypeIntAction.curVal();
}

void ViewSwitchToolBar::subscribe() {
  connect(mViewTypeIntAction.getActionGroup(), &QActionGroup::triggered, this, &ViewSwitchToolBar::onViewTypeActionTriggered);

  auto& viewInst = g_viewActions();
  connect(viewInst._VIEW_BACK_TO, &QAction::triggered, this, &ViewSwitchToolBar::onViewNavigateBackward);
  connect(viewInst._VIEW_FORWARD_TO, &QAction::triggered, this, &ViewSwitchToolBar::onViewNavigateForward);
}

void ViewSwitchToolBar::onViewTypeActionTriggered(QAction* viewAct) {
  ViewTypeTool::ViewType newViewType = mViewTypeIntAction.act2Enum(viewAct);
  mViewRD.operator()(newViewType);
  LOG_D("onViewTypeActionTriggered newViewType[%d]", int(newViewType));
  emit viewTypeChanged(newViewType);
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
  emit viewTypeChanged(vt);
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
  emit viewTypeChanged(vt);
  return true;
}
