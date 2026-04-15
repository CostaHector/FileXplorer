#include "RecursiveFilterProxyTreeModel.h"
#include "PublicMacro.h"
#include "Bool2QtEnum.h"

RecursiveFilterProxyTreeModel::RecursiveFilterProxyTreeModel(QObject* parent) : QSortFilterProxyModel(parent) {
  setRecursiveFilteringEnabled(true);
}

void RecursiveFilterProxyTreeModel::initSortProxy(FavoriteItemData::Role initRole, bool bReverseOrder) {
  if (!isSortProxyInited()) {
    setSortRole(initRole);
    mSortOrder = Bool2QtEnum::toSortOrder(bReverseOrder);
    sort(FavoriteItemData::SORT_COLUMN, mSortOrder);
    m_bSortProxyInited = true;
  }
}

bool RecursiveFilterProxyTreeModel::setSortOrder(bool bReverseOrder) {
  const Qt::SortOrder newSortOrder{Bool2QtEnum::toSortOrder(bReverseOrder)};
  if (newSortOrder == mSortOrder) {
    return false;
  }
  mSortOrder = newSortOrder;
  sort(FavoriteItemData::SORT_COLUMN, mSortOrder);
  return true;
}

void RecursiveFilterProxyTreeModel::setFilterString(const QString& filter) {
  m_filter = filter;
  invalidateFilter();
}

void RecursiveFilterProxyTreeModel::setSourceModel(QAbstractItemModel* sourceModel) {
  if (m_sourceModel != nullptr) {
    LOG_W("Don't reset source Model");
    return;
  }
  CHECK_NULLPTR_RETURN_VOID(sourceModel);
  QSortFilterProxyModel::setSourceModel(sourceModel);
  auto* pTmp = qobject_cast<FavoritesTreeModel*>(sourceModel);
  CHECK_NULLPTR_RETURN_VOID(pTmp);
  m_sourceModel = pTmp;
}

bool RecursiveFilterProxyTreeModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
  if (m_filter.isEmpty() && m_sourceModel == nullptr) {
    return true;
  }

  QModelIndex index = m_sourceModel->index(sourceRow, 0, sourceParent);
  if (!index.isValid()) {
    return false;
  }

  FavTreeNode* item = m_sourceModel->itemFromIndex(index);
  if (item == nullptr) {
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

bool RecursiveFilterProxyTreeModel::itemMatches(const FavTreeNode* item, const QString& filter) const {
  // 检查名称
  if (item->value().name.contains(filter, Qt::CaseInsensitive)) {
    return true;
  }

  // 检查路径
  QString path = item->value().fullPath;
  if (!path.isEmpty() && path.contains(filter, Qt::CaseInsensitive)) {
    return true;
  }

  return false;
}

bool RecursiveFilterProxyTreeModel::hasMatchingChild(const FavTreeNode* item, const QString& filter) const {
  for (int i = 0; i < item->rowCount(); ++i) {
    FavTreeNode* child = item->child(i);
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
    if (m_sourceModel) {
      FavTreeNode* parentItem = m_sourceModel->itemFromIndex(parent);
      if (parentItem && itemMatches(parentItem, filter)) {
        return true;
      }
    }
    parent = parent.parent();
  }
  return false;
}

// bool RecursiveFilterProxyTreeModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
//   if (mLessThan == nullptr) {
//     return QSortFilterProxyModel::lessThan(left, right);
//   }

//   QModelIndex leftParent = left.parent();
//   QModelIndex rightParent = right.parent();
//   if (leftParent != rightParent) {
//     // 只对同一父节点下的子节点排序
//     // 如果父节点不同，保持原始顺序
//     return left.row() < right.row();
//   }
//   return mLessThan(left, right);
// }
