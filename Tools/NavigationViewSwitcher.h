#ifndef NAVIGATIONVIEWSWITCHER_H
#define NAVIGATIONVIEWSWITCHER_H

#include <QObject>
#include "Component/StackedToolBar.h"
#include "Component/ContentPanel.h"

class NavigationViewSwitcher : public QObject {
 public:
  explicit NavigationViewSwitcher(StackedToolBar* navigation, ContentPanel* view, QObject* parent = nullptr);

  void onSwitchByViewType(ViewTypeTool::ViewType);
  void onSwitchByViewAction(const QAction* activatedAction);

 private:
  StackedToolBar* _navigation;
  ContentPanel* _view;
};

#endif  // NAVIGATIONVIEWSWITCHER_H
