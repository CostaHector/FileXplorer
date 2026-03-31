#ifndef NAVIGATIONTOOLBAR_H
#define NAVIGATIONTOOLBAR_H

#include "NavigationFavorites.h"
#include "DevicesDrivesTV.h"
#include "PopupWidgetManager.h"
#include "PublicVariable.h"
#include <QToolBar>

class NavigationToolBar : public QToolBar {
  Q_OBJECT
 public:
  explicit NavigationToolBar(const QString& title = "NavigationToolBar", QWidget* parent = nullptr);
  ~NavigationToolBar();
  void BindIntoNewPathNavi(T_IntoNewPath IntoNewPath) { m_IntoNewPathNavi = IntoNewPath; }

 private:
  void subscribe();
  void onExpandSidebar(bool bExpand);
  void updateToolbarButtonStyle(bool bExpand, bool bAnimation);
  bool onAccessNewPathRequest(const QString& newPath, bool isNew);
  bool onPathActionTriggeredNavi(const QAction* pAct);

  T_IntoNewPath m_IntoNewPathNavi{nullptr};
  QAction* EXPAND_SIDEBAR{nullptr};
  QAction* DEVICES_AND_DRIVES{nullptr};
  QActionGroup* m_pathActionGroups{nullptr};
  NavigationFavorites* m_favorites{nullptr};
  PopupWidgetManager<DevicesDrivesTV>* mDevDriveTV{nullptr};
  static constexpr int MAXIMUM_WIDTH_WHEN_NOT_EXPAND = 32, MAXIMUM_WIDTH_WHEN_EXPAND = 200;
};

#endif  // NAVIGATIONTOOLBAR_H
