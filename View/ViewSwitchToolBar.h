#ifndef VIEWSWITCHTOOLBAR_H
#define VIEWSWITCHTOOLBAR_H

#include <QToolBar>
#include "EnumIntAction.h"
#include "ViewTypeTool.h"
#include "ViewTypeHistory.h"

extern template struct EnumIntAction<ViewTypeTool::ViewType>;

class ViewSwitchToolBar : public QToolBar {
public:
  using QToolBar::QToolBar;
  EnumIntAction<ViewTypeTool::ViewType> mViewTypeIntAction;
  void subscribe();

private:
  void onPushNewViewIntoUndoStack(QAction* viewAct);

  bool onViewNavigateBackward();
  bool onViewNavigateForward();
  ViewTypeHistory mViewRD;
};

#endif // VIEWSWITCHTOOLBAR_H
