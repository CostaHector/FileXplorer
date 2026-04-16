#include "RecursiveFilterProxyTreeModel.h"
#include "PublicMacro.h"
#include "Bool2QtEnum.h"

// setRecursiveFilteringEnabled(true); filterAcceptsRow override its behavior

void RecursiveFilterProxyTreeModel::setFilterString(const QString& filter) {
  m_filter = filter;
  mPassCache.clear();
  invalidateFilter();
}

bool RecursiveFilterProxyTreeModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
  if (curFilter().isEmpty() && sourceModel() == nullptr) {
    return true;
  }
  const QModelIndex index{sourceModel()->index(sourceRow, 0, sourceParent)};
  if (!index.isValid()) {
    return false;
  }
  const FavTreeNode* item = static_cast<FavTreeNode*>(index.internalPointer());
  if (item == nullptr) {
    return false;
  }
  return item->filterAccept(curFilter(), mPassCache);
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
