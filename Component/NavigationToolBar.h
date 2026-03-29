#ifndef NAVIGATIONTOOLBAR_H
#define NAVIGATIONTOOLBAR_H

#include "NavigationExToolBar.h"
#include "DevicesDrivesTV.h"
#include "PopupWidgetManager.h"

class NavigationToolBar : public QToolBar {
  Q_OBJECT
 public:
  explicit NavigationToolBar(const QString& title = "NavigationToolBar", QWidget* parent = nullptr);
  ~NavigationToolBar();

 private:
  void subscribe();
  void onExpandSidebar(bool bExpand);
  void updateToolbarButtonStyle(bool bExpand, bool bAnimation);

  QAction* EXPAND_SIDEBAR{nullptr};
  QAction* DEVICES_AND_DRIVES{nullptr};
  QActionGroup* m_pathActionGroups{nullptr};
  NavigationExToolBar* m_extraAppendTB{nullptr};
  PopupWidgetManager<DevicesDrivesTV>* mDevDriveTV{nullptr};
  static constexpr int MAXIMUM_WIDTH_WHEN_NOT_EXPAND = 32, MAXIMUM_WIDTH_WHEN_EXPAND = 200;
};

#endif  // NAVIGATIONTOOLBAR_H
