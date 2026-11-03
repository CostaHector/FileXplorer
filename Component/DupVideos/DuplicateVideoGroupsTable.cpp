#include "DuplicateVideoGroupsTable.h"
#include "PublicCheckMacro.h"

DuplicateVideoGroupsTable::DuplicateVideoGroupsTable(QWidget* parent) : CustomTableView{"LeftVideoGroupsTable", parent} {
  m_leftGrpModel = new (std::nothrow) DuplicateVideoGroupsModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_leftGrpModel);
  m_leftSortProxy = new (std::nothrow) QSortFilterProxyModel;
  CHECK_NULLPTR_RETURN_VOID(m_leftSortProxy);

  m_leftSortProxy->setSourceModel(m_leftGrpModel);
  setModel(m_leftSortProxy);

  subscribe();

  InitTableView();
  setSortingEnabled(true);
}

const QString DuplicateVideoGroupsTable::GetCurrentDupVideoGroupInfo() const {
  return QString("grouped into %1 batch(es) differ by %2")  //
      .arg(m_leftGrpModel->rowCount())                      //
      .arg(m_leftGrpModel->getCurDifferTypeStr())           //
      ;                                                     //
}

void DuplicateVideoGroupsTable::initGroupMember(DuplicateVideoDetectionCriteria::DVCriteriaE newDifferType, qint64 newSize, int newDuration) {
  m_leftGrpModel->initGroupMember(newDifferType, newSize, newDuration);
}

void DuplicateVideoGroupsTable::setDifferType(const DuplicateVideoDetectionCriteria::DVCriteriaE& newDifferType) {
  m_leftGrpModel->setDifferType(newDifferType);
  emit windowTitleChanged(GetCurrentDupVideoGroupInfo());
}

void DuplicateVideoGroupsTable::setDeviationSize(qint64 newSize) {
  m_leftGrpModel->setDeviationSize(newSize);
  emit windowTitleChanged(GetCurrentDupVideoGroupInfo());
}

void DuplicateVideoGroupsTable::setDeviationDuration(int newDuration) {
  m_leftGrpModel->setDeviationDuration(newDuration);
  emit windowTitleChanged(GetCurrentDupVideoGroupInfo());
}

int DuplicateVideoGroupsTable::onDuplicateVideosListChanged(const DupVidMetaInfoList& needAnalyzeVidLst) {
  clearSelection();
  int rowCountDelta = m_leftGrpModel->onDuplicateVideosListChanged(needAnalyzeVidLst);
  emit windowTitleChanged(GetCurrentDupVideoGroupInfo());
  return rowCountDelta;
}

int DuplicateVideoGroupsTable::GetCurSrcIndexRow(const QItemSelection& selected) const {
  if (selected.isEmpty()) {
    return INVALID_LEFT_SELECTED_ROW;
  }
  const auto& proxyIndex = currentIndex();
  if (!proxyIndex.isValid()) {
    return INVALID_LEFT_SELECTED_ROW;
  }
  const auto& srcIndex = m_leftSortProxy->mapToSource(proxyIndex);
  if (!srcIndex.isValid()) {
    return INVALID_LEFT_SELECTED_ROW;
  }
  return srcIndex.row();
}

void DuplicateVideoGroupsTable::subscribe() {
  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
          [this](const QItemSelection& selected, const QItemSelection& /*deselected*/) {
            int newRow = GetCurSrcIndexRow(selected);
            LOG_D("Row selection changed to %d", newRow);
            emit leftSelectionChanged(newRow);
          });
}
