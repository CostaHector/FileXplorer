#ifndef NAVIGATIONTOOLBAR_H
#define NAVIGATIONTOOLBAR_H

#include "NavigationExToolBar.h"
#include "public/PublicVariable.h"
class DevicesDrivesTV;
class NavigationToolBar : public QToolBar {
 public:
  explicit NavigationToolBar(const QString& title = "NavigationToolBar", bool isShow_ = true);
  void subscribe();
 private:
  NavigationExToolBar* m_extraAppendTB{nullptr};
  DevicesDrivesTV* mDevDriveTV{nullptr};
};

#endif  // NAVIGATIONTOOLBAR_H
