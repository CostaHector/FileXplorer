#include "DuplicateVideoDetailsTable.h"
#include "FileOperatorPub.h"
#include "PublicCheckMacro.h"
#include "FileTool.h"
#include "NotificatorMacro.h"
#include "UndoRedo.h"
#include "PathTool.h"
#include "FileBatchOps.h"
#include "RenameWidget_Replace.h"
#include "FileOpActs.h"
#include <QFile>

DuplicateVideoDetailsTable::DuplicateVideoDetailsTable(QWidget* parent) //
  : CustomTableView{"RightVideoDuplicatesDetails", parent} {
  setDragDropMode(QAbstractItemView::DragDropMode::DragOnly);

  m_detailsModel = new (std::nothrow) DuplicateVideoDetailsModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_detailsModel);

  m_rightSortProxy = new (std::nothrow) QSortFilterProxyModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_rightSortProxy);

  m_rightSortProxy->setSourceModel(m_detailsModel);
  setModel(m_rightSortProxy);
  registerProxyModel(m_rightSortProxy);

  subscribe();

  InitTableView();
  setSortingEnabled(true);
}

QStringList DuplicateVideoDetailsTable::selectedFilesFullPath() const {
  const QModelIndexList srcIndexes = selectedRowsSource();
  return m_detailsModel->filesFullPath(srcIndexes);
}

bool DuplicateVideoDetailsTable::on_effectiveNameCopiedForEverything(const QModelIndex& proInd) const {
  if (!proInd.isValid()) {
    return false;
  }
  const auto& srcIndex = m_rightSortProxy->mapToSource(proInd);
  if (!srcIndex.isValid()) {
    return false;
  }
  const QString& name = m_detailsModel->fileNameUsedForToolEverything(srcIndex);
  return FileTool::CopyTextToSystemClipboard(name);
}

bool DuplicateVideoDetailsTable::on_cellDoubleClicked(const QModelIndex& proInd) const {
  if (!proInd.isValid()) {
    return false;
  }
  const auto& srcIndex = m_rightSortProxy->mapToSource(proInd);
  if (!srcIndex.isValid()) {
    return false;
  }
  const QString& filepath = m_detailsModel->filePath(srcIndex);
  if (!QFile::exists(filepath)) {
    return false;
  }
  return FileTool::OpenLocalFileUsingDesktopService(filepath);
}

bool DuplicateVideoDetailsTable::setSharedMember(GroupedDupVidListArr* pGroupedVidsList,
                                                  DuplicateVideoDetectionCriteria::DVCriteriaE* pCurDifferType) {
  CHECK_NULLPTR_RETURN_FALSE(pGroupedVidsList);
  CHECK_NULLPTR_RETURN_FALSE(pCurDifferType);
  return m_detailsModel->SetSharedMemberSrc(pGroupedVidsList, pCurDifferType);
}

bool DuplicateVideoDetailsTable::on_Rename(AdvanceRenamer& renameWid) {
  const QModelIndexList srcIndexes = selectedRowsSource();
  const int selectedCnt = srcIndexes.size();
  if (selectedCnt < 1) {
    LOG_INFO_NP("Rename", "skipped: selection is empty");
    return false;
  }
  QStringList vidsFullPath;
  vidsFullPath.reserve(selectedCnt);
  for (const QModelIndex& srcInd : srcIndexes) {
    vidsFullPath.push_back(m_detailsModel->filePath(srcInd));
  }
  QString prepath;
  QStringList rels;
  std::tie(prepath, rels) = PathTool::GetLAndRels(vidsFullPath);
  if (prepath.isEmpty()) {
    LOG_WARN_NP("Rename", "prepath is empty");
    return false;
  }

  const FileBatchOps::OpOutcome outcome = FileBatchOps::RenameSelectedWithRelated(renameWid, prepath, rels);
  if (outcome.result == FileBatchOps::OpStatus::SKIP) {
    LOG_INFO_P("Rename", "User cancel rename %d item(s)", outcome.count);
    return false;
  }
  return true;
}

bool DuplicateVideoDetailsTable::on_RecycleSelection() {
  const QModelIndexList srcIndexes = selectedRowsSource();
  const int selectedCnt = srcIndexes.size();
  if (selectedCnt < 1) {
    LOG_INFO_NP("Recycle", "skipped: selection is empty");
    return false;
  }
  QStringList vidsFullPath;
  vidsFullPath.reserve(selectedCnt);
  for (const QModelIndex& srcInd : srcIndexes) {
    QString vidFullPath = m_detailsModel->filePath(srcInd);
    if (QFile::exists(vidFullPath)) {
      vidsFullPath.push_back(vidFullPath);
    }
  }

  QString prepath;
  QStringList rels;
  std::tie(prepath, rels) = PathTool::GetLAndRels(vidsFullPath);

  FileBatchOps::OpOutcome recycleOutcome = FileBatchOps::MoveToTrashSelectedWithRelated(prepath, rels);
  bool bRecycleSucceed{false};
  int affectedFilesCnt{-1};
  switch (recycleOutcome.result) {
    case FileBatchOps::OpStatus::PARTIAL_FAILED:{
      affectedFilesCnt = -1;
      bRecycleSucceed = false;
      break;
    }
    case FileBatchOps::OpStatus::SKIP: { // Nothing need to recycle or user cancel
      affectedFilesCnt = 0;
      return true;
    }
    case FileBatchOps::OpStatus::ALL_SUCCEED: {
      affectedFilesCnt = recycleOutcome.count;
      bRecycleSucceed = true;
      break;
    }
    default:
      break;
  }
  const int recycleRowCnt = m_detailsModel->AfterVideoFilesRecycled(srcIndexes);
  LOG_OE_P(bRecycleSucceed, "Recycle", "recycle %d expanded json/img/video items, rows[%d]", affectedFilesCnt, recycleRowCnt);
  return bRecycleSucceed;
}

int DuplicateVideoDetailsTable::onLeftVideoGroupsTableSelectionChanged(int newRow) {
  return m_detailsModel->onChangeDetailIndex(newRow);
}

void DuplicateVideoDetailsTable::subscribe() {
  connect(this, &QTableView::doubleClicked, this, &DuplicateVideoDetailsTable::on_cellDoubleClicked);
}

bool DuplicateVideoDetailsTable::on_RenameReplace() {
  RenameWidget_Replace pReplacer;
  return on_Rename(pReplacer);
}
