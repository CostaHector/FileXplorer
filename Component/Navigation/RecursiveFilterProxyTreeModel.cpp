#include "RecursiveFilterProxyTreeModel.h"

#include <QStandardItemModel>

RecursiveFilterProxyTreeModel::RecursiveFilterProxyTreeModel(QObject* parent) : QSortFilterProxyModel(parent) {
  setRecursiveFilteringEnabled(true);
}

void RecursiveFilterProxyTreeModel::setFilterString(const QString& filter) {
  m_filter = filter;
  invalidateFilter();
}

bool RecursiveFilterProxyTreeModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
  if (m_filter.isEmpty()) {
    return true;
  }

  QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
  if (!index.isValid()) {
    return false;
  }

  QStandardItemModel* model = qobject_cast<QStandardItemModel*>(sourceModel());
  if (!model) {
    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
  }

  QStandardItem* item = model->itemFromIndex(index);
  if (!item) {
    return false;
  }

  // 三种情况显示：
  // 1. 当前项匹配
  if (itemMatches(item, m_filter)) {
    return true;
  }

  // 2. 有子项匹配
  if (hasMatchingChild(item, m_filter)) {
    return true;
  }

  // 3. 有父项匹配
  if (hasMatchingParent(index, m_filter)) {
    return true;
  }

  return false;
}

bool RecursiveFilterProxyTreeModel::itemMatches(const QStandardItem* item, const QString& filter) const {
  // 检查名称
  if (item->text().contains(filter, Qt::CaseInsensitive)) {
    return true;
  }

  // 检查路径
  QString path = item->data(Qt::UserRole + 2).toString();
  if (!path.isEmpty() && path.contains(filter, Qt::CaseInsensitive)) {
    return true;
  }

  return false;
}

bool RecursiveFilterProxyTreeModel::hasMatchingChild(const QStandardItem* item, const QString& filter) const {
  for (int i = 0; i < item->rowCount(); ++i) {
    QStandardItem* child = item->child(i);
    if (!child)
      continue;

    if (itemMatches(child, filter)) {
      return true;
    }
    if (hasMatchingChild(child, filter)) {
      return true;
    }
  }
  return false;
}

bool RecursiveFilterProxyTreeModel::hasMatchingParent(const QModelIndex& index, const QString& filter) const {
  QModelIndex parent = index.parent();
  while (parent.isValid()) {
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(sourceModel());
    if (model) {
      QStandardItem* parentItem = model->itemFromIndex(parent);
      if (parentItem && itemMatches(parentItem, filter)) {
        return true;
      }
    }
    parent = parent.parent();
  }
  return false;
}
