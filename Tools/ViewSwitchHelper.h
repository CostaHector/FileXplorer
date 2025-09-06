#ifndef VIEWSWITCHHELPER_H
#define VIEWSWITCHHELPER_H

#include <QObject>
#include "StackedAddressAndSearchToolBar.h"
#include "ViewsStackedWidget.h"

class ViewSwitchHelper : public QObject {
 public:
  explicit ViewSwitchHelper(StackedAddressAndSearchToolBar* navigation, ViewsStackedWidget* view, QObject* parent = nullptr);
  void onSwitchByViewType(ViewTypeTool::ViewType);

 private:
  StackedAddressAndSearchToolBar* _navigation;
  ViewsStackedWidget* _view;
};

#endif  // VIEWSWITCHHELPER_H
