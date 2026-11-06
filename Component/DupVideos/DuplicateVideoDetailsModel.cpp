#include "DuplicateVideoDetailsModel.h"
#include "JsonRenameRegex.h"
#include "DataFormatter.h"
#include "PublicCheckMacro.h"
#include "ImageTool.h"
#include "PathTool.h"

#include <QUrl>
#include <QBrush>
#include <QFileInfo>
#include <QMimeData>

QVariant DuplicateVideoDetailsModel::data(const QModelIndex& index, int role) const {
  CHECK_NULLPTR_RETURN_DEFAULT_CONSTRUCT(_pGroupedVidsList);
  if (!isLeftSelectedRowValid()) {
    LOG_D("selectedRow[%d] out of range", getLeftSelectedRow());
    return {};
  }
  if (!index.isValid()) {
    LOG_D("invalid index");
    return {};
  }
  const int row = index.row();
  const int column = index.column();
  const DuplicateVideoMetaInfo::DVInfo& inf = (*_pGroupedVidsList)[(int)*_pCurrentDiffer][getLeftSelectedRow()][row];
  switch (role) {
    case Qt::DisplayRole: {
      switch (column) {
#define DUP_VIDEO_META_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) case DuplicateVideoMetaInfo::enu: return formatter(inf.m_##enu);   //
      DUP_VIDEO_META_INFO_KEY_MAPPING    //
#undef DUP_VIDEO_META_INFO_KEY_ITEM      //
        default : return {};
      }
    }
    case Qt::ForegroundRole: {
      return QFile::exists(inf.m_AbsPath) ? QBrush(Qt::GlobalColor::black) : QBrush(Qt::GlobalColor::gray);
    }
    case Qt::DecorationRole: {
      if (column == 0) {
        return ImageTool::GetIconFromCachedByFullPath(inf.m_AbsPath);
      }
      return {};
    }
    default:
      return {};
  }
}

int DuplicateVideoDetailsModel::rowCountHelper(int leftSelectedRow) const {
  CHECK_NULLPTR_RETURN_INT(_pGroupedVidsList, 0);
  CHECK_NULLPTR_RETURN_INT(_pCurrentDiffer, 0);
  const auto& grpLst = (*_pGroupedVidsList)[(int)*_pCurrentDiffer];
  if (leftSelectedRow < 0 || leftSelectedRow >= grpLst.size()) {
    return 0;
  }
  return grpLst[leftSelectedRow].size();
}

int DuplicateVideoDetailsModel::rowCount(const QModelIndex& /*parent*/) const {
  return rowCountHelper(getLeftSelectedRow());
}

QVariant DuplicateVideoDetailsModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  }
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Orientation::Horizontal) {
      return DuplicateVideoMetaInfo::DV_TABLE_HEADERS[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags DuplicateVideoDetailsModel::flags(const QModelIndex &index) const {
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

QMimeData* DuplicateVideoDetailsModel::mimeData(const QModelIndexList &indexes) const {
  if (indexes.isEmpty()) {
    return nullptr;
  }

  // 过滤首列
  QModelIndexList pathIndexes;
  for (const QModelIndex& index : indexes) {
    if (index.column() != 0) {
      continue;
    }
    pathIndexes.push_back(index);
  }
  if (pathIndexes.isEmpty()) {
    return nullptr;
  }
  const QStringList vids = filesFullPath(pathIndexes);

  QList<QUrl> urls;
  for (const QString& vid: vids) {
    if (vid.isEmpty()) {
      continue;
    }
    urls.append(QUrl::fromLocalFile(vid));
  }
  if (urls.isEmpty()) {
    return nullptr;
  }

  QMimeData* mimeData = new QMimeData;
  mimeData->setUrls(urls);
  return mimeData;
}

bool DuplicateVideoDetailsModel::SetSharedMemberSrc(const GroupedDupVidListArr* _groupedVidsListArr, const DuplicateVideoDetectionCriteria::DVCriteriaE* _currentDiffer) {
  CHECK_NULLPTR_RETURN_FALSE(_groupedVidsListArr);
  CHECK_NULLPTR_RETURN_FALSE(_currentDiffer);
  _pGroupedVidsList = _groupedVidsListArr;
  _pCurrentDiffer = _currentDiffer;
  return true;
}

int DuplicateVideoDetailsModel::onChangeDetailIndex(int newLeftSelectedRow) {
  CHECK_NULLPTR_RETURN_INT(_pGroupedVidsList, -1);
  CHECK_NULLPTR_RETURN_INT(_pCurrentDiffer, -1);
  const int beforeRowN = rowCount();
  int afterRowN = 0;
  if (newLeftSelectedRow == INVALID_LEFT_SELECTED_ROW) {
    afterRowN = 0;
    RowsCountBeginChange(beforeRowN, afterRowN);
    invalidDataLeftSelectedRows();
    RowsCountEndChange();
  } else {
    afterRowN = rowCountHelper(newLeftSelectedRow);
    RowsCountBeginChange(beforeRowN, afterRowN);
    m_leftSelectedRow = newLeftSelectedRow;
    RowsCountEndChange();
  }
  LOG_D("Details rowCount %d->%d. (left selected row=%d)", beforeRowN, afterRowN, getLeftSelectedRow());
  return afterRowN - beforeRowN;
}

QString DuplicateVideoDetailsModel::fileNameUsedForToolEverything(const QModelIndex& index) const {
  QString fileNameWithExt = fileName(index); // may contains FolderName
  QString fileName = PathTool::GetBaseName(fileNameWithExt);
  fileName.replace(JSON_RENAME_REGEX::INVALID_TABLE_NAME_LETTER, " ");
  return fileName;
}

QString DuplicateVideoDetailsModel::filePath(const QModelIndex& index) const {
  if (!index.isValid()) {
    LOG_W("modelindex is invalid");
    return {};
  }
  if (!isLeftSelectedRowValid()) {
    return {};
  }

  CHECK_NULLPTR_RETURN_DEFAULT_CONSTRUCT(_pGroupedVidsList);
  CHECK_NULLPTR_RETURN_DEFAULT_CONSTRUCT(_pCurrentDiffer);

  const DuplicateVideoMetaInfo::DVInfo& inf = (*_pGroupedVidsList)[(int)*_pCurrentDiffer][getLeftSelectedRow()][index.row()];
  return inf.m_AbsPath;
}

QString DuplicateVideoDetailsModel::fileName(const QModelIndex& index) const {
  if (!index.isValid()) {
    LOG_W("modelindex is invalid");
    return {};
  }
  if (!isLeftSelectedRowValid()) {
    return {};
  }

  CHECK_NULLPTR_RETURN_DEFAULT_CONSTRUCT(_pGroupedVidsList);
  CHECK_NULLPTR_RETURN_DEFAULT_CONSTRUCT(_pCurrentDiffer);

  const DuplicateVideoMetaInfo::DVInfo& inf = (*_pGroupedVidsList)[(int)*_pCurrentDiffer][getLeftSelectedRow()][index.row()];
  return inf.m_Name;
}

QStringList DuplicateVideoDetailsModel::filesFullPath(const QModelIndexList& indexes) const {
  if (indexes.isEmpty()) {
    return {};
  }
  if (!isLeftSelectedRowValid()) {
    return {};
  }
  CHECK_NULLPTR_RETURN_DEFAULT_CONSTRUCT(_pGroupedVidsList);
  CHECK_NULLPTR_RETURN_DEFAULT_CONSTRUCT(_pCurrentDiffer);
  const DupVidMetaInfoList& dvInfoLst = (*_pGroupedVidsList)[(int)*_pCurrentDiffer][getLeftSelectedRow()];
  QStringList files;
  for (const QModelIndex& ind: indexes) {
    files.push_back(dvInfoLst[ind.row()].m_AbsPath);
  }
  return files;
}

// operation like scanANewPath/AppendToCurrenAnalyzeList/SizeDeviation/DurationDeviation changed will invalidate left selection
void DuplicateVideoDetailsModel::onInvalidateLeftSelection() {
  const int beforeRowCnt = rowCount();
  const int afterRowCnt = 0;
  RowsCountBeginChange(beforeRowCnt, afterRowCnt);
  invalidDataLeftSelectedRows();
  RowsCountEndChange();
}

int DuplicateVideoDetailsModel::AfterVideoFilesRecycled(const QModelIndexList& indexes) {
  if (indexes.isEmpty()) {
    return 0;
  }
  const int rowCnt = rowCount();
  const int colCnt = columnCount();
  if (rowCnt <= 0 || colCnt <= 0) {
    return 0;
  }

  // 从 selectByRow 的 indexes 中找出最上端、最下端的行
  int minRow = std::numeric_limits<int>::max();
  int maxRow = -1;
  for (const QModelIndex& idx : indexes) {
    if (!idx.isValid()) {
      continue;
    }
    const int r = idx.row();
    if (r < minRow) minRow = r;
    if (r > maxRow) maxRow = r;
  }
  if (maxRow < minRow) {
    LOG_D("AfterVideoFilesRecycled: no valid index");
    return 0;
  }

  // 左上用第 0 列、右下用最后一列，确保覆盖所有列的 ForegroundRole
  const QModelIndex topLeft = index(minRow, 0);
  const QModelIndex bottomRight = index(maxRow, colCnt - 1);
  if (!topLeft.isValid() || !bottomRight.isValid()) {
    LOG_D("AfterVideoFilesRecycled: invalid topLeft/bottomRight");
    return 0;
  }

  // 不删除行，只刷新 ForegroundRole：
  // data() 中 QFile::exists(inf.m_AbsPath) 对已回收文件返回 false → gray
  emit dataChanged(topLeft, bottomRight, {Qt::ForegroundRole});
  return indexes.size();
}