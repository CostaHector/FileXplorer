#ifndef SCENESORTPROXYMODEL_H
#define SCENESORTPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "SceneInfoManager.h"

class SceneSortProxyModel : public QSortFilterProxyModel {
  Q_OBJECT
 public:
  using QSortFilterProxyModel::QSortFilterProxyModel;
  bool isSortProxyInited() const { return m_bSortProxyInited; }
  void initSortProxy(SceneInfo::Role initRole, bool bReverseOrder);
  bool setSortOrder(bool bReverseOrder);

 private:
  Qt::SortOrder mSortOrder{Qt::SortOrder::AscendingOrder};
  bool m_bSortProxyInited{false};
#ifdef RUNNING_UNIT_TESTS
  void ForceCompleteSort();
#endif
};

#endif  // SCENESORTPROXYMODEL_H
