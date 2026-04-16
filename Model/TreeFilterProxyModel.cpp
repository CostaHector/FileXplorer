#include "TreeFilterProxyModel.h"

template<typename TTreeNode>
void TreeFilterProxyModel<TTreeNode>::setFilterString(const QString& filter) {
  m_filter = filter;
  mPassCache.clear();
  invalidateFilter();
}

template<typename TTreeNode>
bool TreeFilterProxyModel<TTreeNode>::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
  if (curFilter().isEmpty() && sourceModel() == nullptr) {
    return true;
  }
  const QModelIndex index{sourceModel()->index(sourceRow, 0, sourceParent)};
  if (!index.isValid()) {
    return false;
  }
  const TTreeNode* item = static_cast<TTreeNode*>(index.internalPointer());
  if (item == nullptr) {
    return false;
  }
  return item->filterAccept(curFilter(), mPassCache);
}

#include "FavoriteItemData.h"
template class TreeFilterProxyModel<FavTreeNode>;

#include "StyleItemData.h"
template class TreeFilterProxyModel<StyleTreeNode>;
