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
  ViewTypeTool::ViewType GetCurViewType() const;

signals:
  void viewTypeChanged(const ViewTypeTool::ViewType viewType);

private:
  void subscribe();
  void onViewTypeActionTriggered(QAction* viewAct);

  bool onViewNavigateBackward();
  bool onViewNavigateForward();
  ViewTypeHistory mViewRD;

  EnumIntAction<ViewTypeTool::ViewType> mViewTypeIntAction;
};

#endif // VIEWSWITCHTOOLBAR_H
