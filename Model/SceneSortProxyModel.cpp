#include "SceneSortProxyModel.h"
#include "Bool2QtEnum.h"

void SceneSortProxyModel::initSortProxy(SceneInfo::Role initRole, bool bReverseOrder) {
  if (!isSortProxyInited()) {
    setSortRole(initRole);
    mSortOrder = Bool2QtEnum::toSortOrder(bReverseOrder);
    sort(SceneInfo::SORT_COLUMN, mSortOrder);
    m_bSortProxyInited = true;
  }
}

bool SceneSortProxyModel::setSortOrder(bool bReverseOrder) {
  const Qt::SortOrder newSortOrder{Bool2QtEnum::toSortOrder(bReverseOrder)};
  if (newSortOrder == mSortOrder) {
    return false;
  }
  mSortOrder = newSortOrder;
  sort(SceneInfo::SORT_COLUMN, mSortOrder);
  return true;
}

#ifdef RUNNING_UNIT_TESTS
void SceneSortProxyModel::ForceCompleteSort() {
  const bool bNeedResetModel{true};
  if (bNeedResetModel) beginResetModel();
  QSortFilterProxyModel::sort(SceneInfo::SORT_COLUMN, sortOrder());
  if (bNeedResetModel) endResetModel();
  invalidate();
}
#endif
