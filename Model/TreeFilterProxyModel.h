#ifndef TREEFILTERPROXYMODEL_H
#define TREEFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

// Requirement:
// TTreeNode must have implement
// member function:
// bool filterAccept(const QString& subStr, QHash<const void*, bool>& cache)

template<typename TTreeNode>
class TreeFilterProxyModel : public QSortFilterProxyModel {
public:
  using QSortFilterProxyModel::QSortFilterProxyModel;
  void setFilterString(const QString& filter);
  QString curFilter() const { return m_filter; }

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
  QString m_filter;
  mutable QHash<const void*, bool> mPassCache;
};

#endif // TREEFILTERPROXYMODEL_H
