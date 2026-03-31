#include "SceneSortProxyModel.h"
#include "Bool2QtEnum.h"

void SceneSortProxyModel::initSortProxy(SceneInfo::Role initRole, bool bReverseOrder) {
  setSortRole(initRole);
  mSortOrder = Bool2QtEnum::toSortOrder(bReverseOrder);
  sort(SceneInfo::SORT_COLUMN, mSortOrder);
  m_bSortProxyInited = true;
}

void SceneSortProxyModel::setSortOrder(bool bReverseOrder) {
  const Qt::SortOrder newSortOrder{Bool2QtEnum::toSortOrder(bReverseOrder)};
  if (newSortOrder == mSortOrder) {
    return;
  }
  mSortOrder = newSortOrder;
  sort(SceneInfo::SORT_COLUMN, mSortOrder);
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
