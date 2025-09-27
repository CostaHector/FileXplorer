#ifndef NAVIGATIONTOOLBAR_H
#define NAVIGATIONTOOLBAR_H

#include "NavigationExToolBar.h"
#include "DevicesDrivesTV.h"
#include "PopupWidgetManager.h"

class NavigationToolBar : public QToolBar {
 public:
  explicit NavigationToolBar(const QString& title = "NavigationToolBar", bool isShow_ = true);
  void subscribe();
 private:
  NavigationExToolBar* m_extraAppendTB{nullptr};
  PopupWidgetManager<DevicesDrivesTV>* mDevDriveTV{nullptr};
};

#endif  // NAVIGATIONTOOLBAR_H
