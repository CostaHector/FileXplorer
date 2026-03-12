#ifndef VIEWSWITCHTOOLBAR_H
#define VIEWSWITCHTOOLBAR_H

#include <QToolBar>
#include "EnumIntAction.h"
#include "ViewTypeTool.h"
#include "ViewTypeHistory.h"

extern template struct EnumIntAction<ViewTypeTool::ViewType>;

class ViewSwitchToolBar : public QToolBar {
  Q_OBJECT
public:
  explicit ViewSwitchToolBar(const QString &title, QWidget *parent = nullptr);
  EnumIntAction<ViewTypeTool::ViewType> mViewTypeIntAction;
  void subscribe();
signals:
  void viewTypeChanged(const ViewTypeTool::ViewType viewType);
private:
  void onViewTypeActionTriggered(QAction* viewAct);

  bool onViewNavigateBackward();
  bool onViewNavigateForward();
  ViewTypeHistory mViewRD;
};

#endif // VIEWSWITCHTOOLBAR_H
