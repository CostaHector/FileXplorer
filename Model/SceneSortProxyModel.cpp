#include "SceneSortProxyModel.h"
#include "PublicMacro.h"

void SceneSortProxyModel::setSourceModel(QAbstractItemModel* sourceModel) {
  if (m_sourceModel != nullptr) {
    LOG_W("Don't reset source Model");
    return;
  }
  CHECK_NULLPTR_RETURN_VOID(sourceModel);
  QSortFilterProxyModel::setSourceModel(sourceModel);
  auto* pTmp = qobject_cast<ScenesListModel*>(sourceModel);
  CHECK_NULLPTR_RETURN_VOID(pTmp);
  m_sourceModel = pTmp;
}

void SceneSortProxyModel::sort(int newColumn, Qt::SortOrder newOrder) {
  if (sourceModel() == nullptr) {
    return;
  }
  bool anyChange = false;
  if (newColumn != (int)m_sortDimension) {  // need update mComparator when dimension changed
    m_sortDimension = SceneSortOrderHelper::toEnum(newColumn);
    mComparator = SceneInfo::getCompareFunc(m_sortDimension);
    anyChange = true;
  }
  if (newOrder != sortOrder()) {
    anyChange = true;
  }
  if (!anyChange) {
    LOG_D("Sort policy unchange at all remains[dimension:%s, order:%d]", SceneSortOrderHelper::c_str(m_sortDimension), (int)newOrder);
    return;
  }
  LOG_D("Sort dimension changed to[%s] order: %d", SceneSortOrderHelper::c_str(m_sortDimension), (int)newOrder);
  QSortFilterProxyModel::sort(0, newOrder);
#ifdef RUNNING_UNIT_TESTS
  ForceCompleteSort();
#endif
}

void SceneSortProxyModel::sortByFieldDimension(SceneSortOrderHelper::SortDimE newSortDimension, Qt::SortOrder newOrder) {
  SceneSortProxyModel::sort((int)newSortDimension, newOrder);
}

bool SceneSortProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const {
  if (m_sourceModel == nullptr) {
    return false;
  }
  int leftRow = 0, rightRow = 0;
  if (!m_sourceModel->isIndexValid(source_left, leftRow)) {
    return false;
  }
  if (!m_sourceModel->isIndexValid(source_right, rightRow)) {
    return false;
  }
  const SceneInfoList::const_iterator iter = m_sourceModel->GetFirstIterator();
  return (iter[leftRow].*mComparator)(iter[rightRow]);;
}

#ifdef RUNNING_UNIT_TESTS
void SceneSortProxyModel::ForceCompleteSort() {
  LOG_D("Force complete sort rows");
  invalidate();
}
#endif
