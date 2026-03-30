#ifndef RECURSIVEFILTERPROXYTREEMODEL_H
#define RECURSIVEFILTERPROXYTREEMODEL_H

#include <QSortFilterProxyModel>
#include <QStandardItem>

class RecursiveFilterProxyTreeModel : public QSortFilterProxyModel {
  Q_OBJECT

 public:
  explicit RecursiveFilterProxyTreeModel(QObject* parent = nullptr);
  void setFilterString(const QString& filter);

 protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

 private:
  bool itemMatches(const QStandardItem* item, const QString& filter) const;
  bool hasMatchingChild(const QStandardItem* item, const QString& filter) const;
  bool hasMatchingParent(const QModelIndex& index, const QString& filter) const;

  QString m_filter;
};

#endif  // RECURSIVEFILTERPROXYTREEMODEL_H
