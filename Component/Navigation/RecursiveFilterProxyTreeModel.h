#ifndef RECURSIVEFILTERPROXYTREEMODEL_H
#define RECURSIVEFILTERPROXYTREEMODEL_H

#include <QSortFilterProxyModel>
#include "FavoritesTreeModel.h"

class RecursiveFilterProxyTreeModel : public QSortFilterProxyModel {
  Q_OBJECT

 public:
  explicit RecursiveFilterProxyTreeModel(QObject* parent = nullptr);
  bool isSortProxyInited() const { return m_bSortProxyInited; }
  void initSortProxy(FavoriteItemData::Role initRole, bool bReverseOrder);
  void setFilterString(const QString& filter);
  void setSourceModel(QAbstractItemModel* sourceModel) override;
  bool setSortOrder(bool bReverseOrder);
  QString curFilter() const { return m_filter; }

 protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

 private:
  bool itemMatches(const MyTreeNode* item, const QString& filter) const;
  bool hasMatchingChild(const MyTreeNode* item, const QString& filter) const;
  bool hasMatchingParent(const QModelIndex& index, const QString& filter) const;

  bool m_bSortProxyInited{false};
  QString m_filter;
  Qt::SortOrder mSortOrder{Qt::SortOrder::AscendingOrder};
  FavoritesTreeModel* m_sourceModel{nullptr};
};

#endif  // RECURSIVEFILTERPROXYTREEMODEL_H
