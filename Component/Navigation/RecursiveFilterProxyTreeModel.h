#ifndef RECURSIVEFILTERPROXYTREEMODEL_H
#define RECURSIVEFILTERPROXYTREEMODEL_H

#include <QSortFilterProxyModel>
#include "FavoritesTreeModel.h"

class RecursiveFilterProxyTreeModel : public QSortFilterProxyModel {
  Q_OBJECT

 public:
  explicit RecursiveFilterProxyTreeModel(QObject* parent = nullptr);
  void initSortProxy(FavoriteItemData::Role initRole, bool bReverseOrder);
  void setFilterString(const QString& filter);
  void setSourceModel(QAbstractItemModel* sourceModel) override;
  void setSortOrder(bool bReverseOrder);

 protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

 private:
  bool itemMatches(const QStandardItem* item, const QString& filter) const;
  bool hasMatchingChild(const QStandardItem* item, const QString& filter) const;
  bool hasMatchingParent(const QModelIndex& index, const QString& filter) const;

  QString m_filter;
  Qt::SortOrder mSortOrder{Qt::SortOrder::AscendingOrder};
  FavoritesTreeModel* m_sourceModel{nullptr};
};

#endif  // RECURSIVEFILTERPROXYTREEMODEL_H
