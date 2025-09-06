#ifndef VIEWSWITCHTOOLBAR_H
#define VIEWSWITCHTOOLBAR_H

#include <QToolBar>
#include "EnumIntAction.h"
#include "ViewTypeTool.h"
#include "ViewTypeFormerLadder.h"

extern template struct EnumIntAction<ViewTypeTool::ViewType>;

class ViewSwitchToolBar : public QToolBar {
public:
  using QToolBar::QToolBar;
  EnumIntAction<ViewTypeTool::ViewType> mViewTypeIntAction;
  void subscribe();

private:
  void onViewActionInActionGroupTriggered(QAction* viewAct);

  bool onViewBackward();
  bool onViewForward();
  ViewTypeFormerLadder mViewRD;
};

#endif // VIEWSWITCHTOOLBAR_H
