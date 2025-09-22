#include "RightVideoDuplicatesModel.h"
#include "MD5Calculator.h"
#include "JsonRenameRegex.h"
#include "DisplayEnhancement.h"
#include "PublicMacro.h"

#include <QBrush>
#include <QFileInfo>
#include <QFileIconProvider>

const QStringList RightVideoDuplicatesModel::VIDS_DETAIL_HEADER{"Name", "Date", "Size", "Duration", "Hash", "FullPath"};
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
  const DupVidMetaInfo& inf = (*_pGroupedVidsList)[(int)*_pCurrentDiffer][getLeftSelectedRow()][row];
  switch (role) {
    case Qt::DisplayRole: {
      switch (column) {
        case 0:
          return inf.name;
        case 1:
          return QDateTime::fromMSecsSinceEpoch(inf.modifiedDate);
        case 2:
          return FILE_PROPERTY_DSP::sizeToHumanReadFriendly(inf.sz);
        case 3:
          return FILE_PROPERTY_DSP::durationToHumanReadFriendly(inf.dur);
        case 4: {
          if (inf.hash.isEmpty() && rowCount() <= 10) {
            return MD5Calculator::GetFileMD5(inf.abspath, 1024);
          }
          return inf.hash;
        }
        case 5:
          return inf.abspath;
        default:
          return {};
      }
    }
    case Qt::ForegroundRole: {
      return QFile::exists(inf.abspath) ? QBrush(Qt::GlobalColor::black) : QBrush(Qt::GlobalColor::gray);
    }
    case Qt::DecorationRole: {
      if (column == 0) {
        static QFileIconProvider fip;
        return fip.icon(QFileInfo(inf.abspath));
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
  if (!isIndexInrange(0, leftSelectedRow, grpLst.size())) {
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
      return VIDS_DETAIL_HEADER[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

bool RightVideoDuplicatesModel::SyncFrom(const GroupedDupVidListArr* _groupedVidsListArr, const RedundantVideoTool::DIFFER_BY_TYPE* _currentDiffer) {
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

  const DupVidMetaInfo& inf = (*_pGroupedVidsList)[(int)*_pCurrentDiffer][getLeftSelectedRow()][index.row()];
  return inf.abspath;
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

  const DupVidMetaInfo& inf = (*_pGroupedVidsList)[(int)*_pCurrentDiffer][getLeftSelectedRow()][index.row()];
  return inf.name;
}

// operation like scanANewPath/AppendToCurrenAnalyzeList/SizeDeviation/DurationDeviation changed will invalidate left selection
void RightVideoDuplicatesModel::onInvalidateLeftSelection() {
  const int beforeRowCnt = rowCount();
  const int afterRowCnt = 0;
  RowsCountBeginChange(beforeRowCnt, afterRowCnt);
  invalidDataLeftSelectedRows();
  RowsCountEndChange();
}

