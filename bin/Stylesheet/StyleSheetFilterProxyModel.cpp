#include "StyleSheetFilterProxyModel.h"
#include "StyleItemData.h"

void StyleSheetFilterProxyModel::setFilterString(const QString& filter) {
  m_filter = filter;
  mPassCache.clear();
  invalidateFilter();
}

bool StyleSheetFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
  if (curFilter().isEmpty() && sourceModel() == nullptr) {
    return true;
  }
  const QModelIndex index{sourceModel()->index(sourceRow, 0, sourceParent)};
  if (!index.isValid()) {
    return false;
  }
  const StyleTreeNode* item = static_cast<StyleTreeNode*>(index.internalPointer());
  if (item == nullptr) {
    return false;
  }
  return item->filterAccept(curFilter(), mPassCache);
}
