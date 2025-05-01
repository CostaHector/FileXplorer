#ifndef NAVIGATIONTOOLBAR_H
#define NAVIGATIONTOOLBAR_H

#include "NavigationExToolBar.h"
class DevicesDriveTableView;
class NavigationToolBar : public QToolBar {
 public:
  explicit NavigationToolBar(const QString& title = "NavigationToolBar", bool isShow_ = true);
  void subscribe();
 private:
  NavigationExToolBar* m_extraAppendTB{nullptr};
  QAction* DEVICES_AND_DRIVES{nullptr};

  DevicesDriveTableView* mDevDriveTV{nullptr};
};

#endif  // NAVIGATIONTOOLBAR_H
