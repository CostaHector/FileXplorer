#include "RightVideoDuplicatesModel.h"
#include "JsonRenameRegex.h"
#include "DataFormatter.h"
#include "PublicMacro.h"

#include <QBrush>
#include <QFileInfo>
#include <QFileIconProvider>

QVariant RightVideoDuplicatesModel::data(const QModelIndex& index, int role) const {
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
        static QFileIconProvider fip;
        return fip.icon(QFileInfo(inf.m_AbsPath));
      }
      return {};
    }
    default:
      return {};
  }
}

int RightVideoDuplicatesModel::rowCountHelper(int leftSelectedRow) const {
  CHECK_NULLPTR_RETURN_INT(_pGroupedVidsList, 0);
  CHECK_NULLPTR_RETURN_INT(_pCurrentDiffer, 0);
  const auto& grpLst = (*_pGroupedVidsList)[(int)*_pCurrentDiffer];
  if (leftSelectedRow < 0 || leftSelectedRow >= grpLst.size()) {
    return 0;
  }
  return grpLst[leftSelectedRow].size();
}

int RightVideoDuplicatesModel::rowCount(const QModelIndex& /*parent*/) const {
  return rowCountHelper(getLeftSelectedRow());
}

auto RightVideoDuplicatesModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant {
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

bool RightVideoDuplicatesModel::SetSharedMemberSrc(const GroupedDupVidListArr* _groupedVidsListArr, const DuplicateVideoDetectionCriteria::DVCriteriaE* _currentDiffer) {
  CHECK_NULLPTR_RETURN_FALSE(_groupedVidsListArr);
  CHECK_NULLPTR_RETURN_FALSE(_currentDiffer);
  _pGroupedVidsList = _groupedVidsListArr;
  _pCurrentDiffer = _currentDiffer;
  return true;
}

int RightVideoDuplicatesModel::onChangeDetailIndex(int newLeftSelectedRow) {
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

QString RightVideoDuplicatesModel::fileNameUsedForToolEverything(const QModelIndex& index) const {
  QString fileNamePlain = fileName(index);
  fileNamePlain.replace(JSON_RENAME_REGEX::INVALID_TABLE_NAME_LETTER, " ");
  return fileNamePlain;
}

QString RightVideoDuplicatesModel::filePath(const QModelIndex& index) const {
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

QString RightVideoDuplicatesModel::fileName(const QModelIndex& index) const {
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

// operation like scanANewPath/AppendToCurrenAnalyzeList/SizeDeviation/DurationDeviation changed will invalidate left selection
void RightVideoDuplicatesModel::onInvalidateLeftSelection() {
  const int beforeRowCnt = rowCount();
  const int afterRowCnt = 0;
  RowsCountBeginChange(beforeRowCnt, afterRowCnt);
  invalidDataLeftSelectedRows();
  RowsCountEndChange();
}

