#include "LeftVideoGroupsTable.h"
#include "PublicMacro.h"

LeftVideoGroupsTable::LeftVideoGroupsTable(QWidget* parent) : CustomTableView{"LeftVideoGroupsTable", parent} {
  m_leftGrpModel = new (std::nothrow) LeftVideoGroupsModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_leftGrpModel);
  m_leftSortProxy = new (std::nothrow) QSortFilterProxyModel;
  CHECK_NULLPTR_RETURN_VOID(m_leftSortProxy);

  m_leftSortProxy->setSourceModel(m_leftGrpModel);
  setModel(m_leftSortProxy);

  subscribe();

  InitTableView();
  setSortingEnabled(true);
}

const QString LeftVideoGroupsTable::GetCurrentDupVideoGroupInfo() const {
  return QString("grouped into %1 batch(es) differ by %2")  //
      .arg(m_leftGrpModel->rowCount())                      //
      .arg(m_leftGrpModel->getCurDifferTypeStr())           //
      ;                                                     //
}

void LeftVideoGroupsTable::setDifferType(const DuplicateVideoDetectionCriteria::DVCriteriaE& newDifferType) {
  m_leftGrpModel->setDifferType(newDifferType);
  emit windowTitleChanged(GetCurrentDupVideoGroupInfo());
}

void LeftVideoGroupsTable::setDeviationDuration(int newDuration) {
  m_leftGrpModel->setDeviationDuration(newDuration);
  emit windowTitleChanged(GetCurrentDupVideoGroupInfo());
}

void LeftVideoGroupsTable::setDeviationSize(qint64 newSize) {
  m_leftGrpModel->setDeviationSize(newSize);
  emit windowTitleChanged(GetCurrentDupVideoGroupInfo());
}

int LeftVideoGroupsTable::onDuplicateVideosListChanged(const DupVidMetaInfoList& needAnalyzeVidLst) {
  clearSelection();
  int rowCountDelta = m_leftGrpModel->onDuplicateVideosListChanged(needAnalyzeVidLst);
  emit windowTitleChanged(GetCurrentDupVideoGroupInfo());
  return rowCountDelta;
}

int LeftVideoGroupsTable::GetCurSrcIndexRow(const QItemSelection& selected) const {
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

void LeftVideoGroupsTable::subscribe() {
  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
          [this](const QItemSelection& selected, const QItemSelection& /*deselected*/) {
            int newRow = GetCurSrcIndexRow(selected);
            LOG_D("Row selection changed to %d", newRow);
            emit leftSelectionChanged(newRow);
          });
}
