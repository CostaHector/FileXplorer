#ifndef RECURSIVEFILTERPROXYTREEMODEL_H
#define RECURSIVEFILTERPROXYTREEMODEL_H

#include "TreeFilterProxyModel.h"
#include "FavoriteItemData.h"

extern template class TreeFilterProxyModel<FavTreeNode>;

class RecursiveFilterProxyTreeModel : public TreeFilterProxyModel<FavTreeNode> {
public:
  using TreeFilterProxyModel<FavTreeNode>::TreeFilterProxyModel;

  bool isSortProxyInited() const { return m_bSortProxyInited; }
  void initSortProxy(FavoriteItemData::Role initRole, bool bReverseOrder);
  bool setSortOrder(bool bReverseOrder);

private:
  bool m_bSortProxyInited{false};
  Qt::SortOrder mSortOrder{Qt::SortOrder::AscendingOrder};
};

#endif // RECURSIVEFILTERPROXYTREEMODEL_H
