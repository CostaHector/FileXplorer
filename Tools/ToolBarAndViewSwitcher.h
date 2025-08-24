#ifndef TOOLBARANDVIEWSWITCHER_H
#define TOOLBARANDVIEWSWITCHER_H

#include <QObject>
#include "StackedAddressAndSearchToolBar.h"
#include "ViewsStackedWidget.h"

class ToolBarAndViewSwitcher : public QObject {
 public:
  explicit ToolBarAndViewSwitcher(StackedAddressAndSearchToolBar* navigation, ViewsStackedWidget* view, QObject* parent = nullptr);

  void onSwitchByViewType(ViewTypeTool::ViewType);
  void onSwitchByViewAction(const QAction* activatedAction);

 private:
  StackedAddressAndSearchToolBar* _navigation;
  ViewsStackedWidget* _view;
};

#endif  // TOOLBARANDVIEWSWITCHER_H
