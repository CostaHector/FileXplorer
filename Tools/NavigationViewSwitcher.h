#ifndef NAVIGATIONVIEWSWITCHER_H
#define NAVIGATIONVIEWSWITCHER_H

#include <QObject>
#include "Component/StackedToolBar.h"
#include "ContentPanel.h"

class NavigationViewSwitcher : public QObject {
 public:
  explicit NavigationViewSwitcher(StackedToolBar* navigation, ContentPanel* view, QObject* parent = nullptr);

  void onSwitchByViewType(const QString& viewType);
  void onSwitchByViewAction(const QAction* activatedAction);

 private:
  StackedToolBar* _navigation;
  ContentPanel* _view;
};

#endif  // NAVIGATIONVIEWSWITCHER_H
