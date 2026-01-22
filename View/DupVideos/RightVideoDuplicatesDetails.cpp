#include "RightVideoDuplicatesDetails.h"
#include "FileOperatorPub.h"
#include "DuplicateVideosFinderActions.h"
#include "PublicMacro.h"
#include "PublicTool.h"
#include "NotificatorMacro.h"
#include "UndoRedo.h"
#include <QFile>

RightVideoDuplicatesDetails::RightVideoDuplicatesDetails(QWidget* parent) //
  : CustomTableView{"RightVideoDuplicatesDetails", parent} {
  m_detailsModel = new (std::nothrow) RightVideoDuplicatesModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_detailsModel);

  m_rightSortProxy = new (std::nothrow) QSortFilterProxyModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_rightSortProxy);

  m_rightSortProxy->setSourceModel(m_detailsModel);
  setModel(m_rightSortProxy);

  subscribe();

  InitTableView();
  setSortingEnabled(true);
}

bool RightVideoDuplicatesDetails::on_effectiveNameCopiedForEverything(const QModelIndex& ind) const {
  if (!ind.isValid()) {
    return false;
  }
  const auto& srcIndex = m_rightSortProxy->mapToSource(ind);
  if (!srcIndex.isValid()) {
    return false;
  }
  const QString& name = m_detailsModel->fileNameUsedForToolEverything(srcIndex);
  return FileTool::CopyTextToSystemClipboard(name);
}

bool RightVideoDuplicatesDetails::on_cellDoubleClicked(const QModelIndex& ind) const {
  if (!ind.isValid()) {
    return false;
  }
  const auto& srcIndex = m_rightSortProxy->mapToSource(ind);
  if (!srcIndex.isValid()) {
    return false;
  }
  const QString& filepath = m_detailsModel->filePath(srcIndex);
  if (!QFile::exists(filepath)) {
    return false;
  }
  return FileTool::OpenLocalFileUsingDesktopService(filepath);
}

bool RightVideoDuplicatesDetails::setSharedMember(GroupedDupVidListArr* pGroupedVidsList,
                                                  DuplicateVideoDetectionCriteria::DVCriteriaE* pCurDifferType) {
  CHECK_NULLPTR_RETURN_FALSE(pGroupedVidsList);
  CHECK_NULLPTR_RETURN_FALSE(pCurDifferType);
  return m_detailsModel->SyncFrom(pGroupedVidsList, pCurDifferType);
}

bool RightVideoDuplicatesDetails::onRecycleSelection() {
  const int SELECTED_CNT = selectionModel()->selectedRows().size();
  if (SELECTED_CNT < 1) {
    LOG_INFO_NP("Skip", "nothing selected to recycle");
    return false;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE recycleCmds;
  recycleCmds.reserve(SELECTED_CNT);
  for (const auto& proInd : selectionModel()->selectedRows()) {
    const auto& srcInd = m_rightSortProxy->mapToSource(proInd);
    recycleCmds.append(ACMD::GetInstMOVETOTRASH("", m_detailsModel->filePath(srcInd)));
  }
  auto isRenameAllSucceed = UndoRedo::GetInst().Do(recycleCmds);
  LOG_OE_P(isRenameAllSucceed, "Recycle selection", "%d item(s) bResult:%d", SELECTED_CNT, isRenameAllSucceed);
  return isRenameAllSucceed;
}

int RightVideoDuplicatesDetails::onLeftVideoGroupsTableSelectionChanged(int newRow) {
  return m_detailsModel->onChangeDetailIndex(newRow);
}

void RightVideoDuplicatesDetails::subscribe() {
  connect(this, &QTableView::doubleClicked, this, &RightVideoDuplicatesDetails::on_cellDoubleClicked);
  connect(g_dupVidFinderAg().RECYCLE_SELECTIONS, &QAction::triggered, this, &RightVideoDuplicatesDetails::onRecycleSelection);
}
