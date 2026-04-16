#ifndef RECURSIVEFILTERPROXYTREEMODEL_H
#define RECURSIVEFILTERPROXYTREEMODEL_H

#include <QSortFilterProxyModel>
#include "FavoriteItemData.h"

class RecursiveFilterProxyTreeModel : public QSortFilterProxyModel {
public:
  using QSortFilterProxyModel::QSortFilterProxyModel;
  void setFilterString(const QString& filter);
  const QString& curFilter() const { return m_filter; }

  bool isSortProxyInited() const { return m_bSortProxyInited; }
  void initSortProxy(FavoriteItemData::Role initRole, bool bReverseOrder);
  bool setSortOrder(bool bReverseOrder);

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
  QString m_filter;
  mutable QHash<const void*, bool> mPassCache;

  bool m_bSortProxyInited{false};
  Qt::SortOrder mSortOrder{Qt::SortOrder::AscendingOrder};
};

#endif // RECURSIVEFILTERPROXYTREEMODEL_H
