#include "DuplicateVideosMetaInfoModel.h"
#include "Logger.h"
#include "PublicMacro.h"
#include <QIcon>

const QStringList DuplicateVideosMetaInfoModel::DUP_VIDEOS_METAINFO_HOR_HEADER{"Table name", "Count"};

auto DuplicateVideosMetaInfoModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  }
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Orientation::Horizontal) {
      return DUP_VIDEOS_METAINFO_HOR_HEADER[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant DuplicateVideosMetaInfoModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  const int row = index.row(), col = index.column();
  if (role == Qt::ItemDataRole::DisplayRole) {
    switch (col) {
      case 0:
        return m_data[row].tableName;
      case 1:
        return m_data[row].count;
      default:
        break;
    }
  } else if (role == Qt::ItemDataRole::DecorationRole && col == 0) {
    static const QIcon TABLE_ICON{":img/TABLE"};
    return TABLE_ICON;
  }
  return {};
}

QString DuplicateVideosMetaInfoModel::fileName(const QModelIndex& index) const {
  if (!index.isValid()) {
    LOG_W("invalid index");
    return {};
  }
  int row = index.row();
  if (!isIndexInrange(0, row, rowCount())) {
    LOG_W("row[%d] out of range[0, %d)", row, rowCount());
    return {};
  }
  return m_data[index.row()].tableName;
}

QStringList DuplicateVideosMetaInfoModel::fileNames(const QModelIndexList& indx) const {
  QStringList tbls;
  tbls.reserve(indx.size());
  for (const auto& ind : indx) {
    tbls.append(fileName(ind));
  }
  return tbls;
}

/* don't use input params newData after */
void DuplicateVideosMetaInfoModel::UpdateDupVideoTableMetaInfoList(DupVidTableName2RecordCountList& newData) {
  int beforeRow = m_data.size(), afterRow = newData.size();

  RowsCountBeginChange(beforeRow, afterRow);
  m_data.swap(newData);
  RowsCountEndChange();

  emit dataChanged(index(0, 0, {}), index(afterRow - 1, columnCount() - 1, {}), {Qt::ItemDataRole::DisplayRole});
  LOG_D("Ai Media rowCount %d->%d", beforeRow, afterRow);
}
