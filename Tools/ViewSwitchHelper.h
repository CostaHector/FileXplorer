#ifndef VIEWSWITCHHELPER_H
#define VIEWSWITCHHELPER_H

#include <QObject>
#include "StackedAddressAndSearchToolBar.h"
#include "ViewsStackedWidget.h"
#include "ScenePageControl.h"
#include "NavigationToolBar.h"

class ViewSwitchHelper : public QObject {
 public:
  explicit ViewSwitchHelper(StackedAddressAndSearchToolBar* navigation,  //
                            ViewsStackedWidget* view,                    //
                            ScenePageControl* scenePageControl,          //
                            NavigationToolBar* navigationToolBar,
                            QObject* parent = nullptr);
  void onSwitchByViewType(ViewTypeTool::ViewType);

 private:
  StackedAddressAndSearchToolBar* _navigation{nullptr};
  ViewsStackedWidget* _view{nullptr};
  ScenePageControl* _scenePageControl{nullptr};
  NavigationToolBar* _navigationToolBar{nullptr};
};

#endif  // VIEWSWITCHHELPER_H
