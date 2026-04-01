#include "SceneSortProxyModel.h"
#include "Bool2QtEnum.h"

bool SceneSortProxyModel::initSortSetting(SceneInfo::Role initRole, bool bReverseOrder) {
  if (isSortProxyInited()) {
    return false;
  }
  setSortRole(initRole);
  mSortOrder = Bool2QtEnum::toSortOrder(bReverseOrder);
  sort(SceneInfo::SORT_COLUMN, mSortOrder);
  m_bSortProxyInited = true;
  return true;
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
