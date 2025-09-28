#ifndef SCENESORTPROXYMODEL_H
#define SCENESORTPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "ScenePageNaviHelper.h"
#include "SceneInfoManager.h"
#include "ScenesListModel.h"

class SceneSortProxyModel : public QSortFilterProxyModel {
  Q_OBJECT
 public:
  using QSortFilterProxyModel::QSortFilterProxyModel;

  void setSourceModel(QAbstractItemModel* sourceModel) override;

  void sort(int column, Qt::SortOrder newOrder = Qt::AscendingOrder) override;
  void sortByFieldDimension(SceneSortOrderHelper::SortDimE newSortDimension, bool bReverse);

 protected:
  bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;

 private:
  SceneSortOrderHelper::SortDimE m_sortDimension = SceneSortOrderHelper::DEFAULT_SCENE_SORT_ORDER;
  SCENE_INFO::CompareFunc mComparator = SCENE_INFO::getCompareFunc(m_sortDimension);
  ScenesListModel* m_sourceModel = nullptr;
};

#endif  // SCENESORTPROXYMODEL_H
