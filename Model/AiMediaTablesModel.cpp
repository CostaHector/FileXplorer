#include "AiMediaTablesModel.h"
#include "Logger.h"

const QStringList AiMediaTablesModel::AITABLE_HOR_HEADER{"table name", "count"};
auto AiMediaTablesModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  }
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Orientation::Horizontal) {
      return AITABLE_HOR_HEADER[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant AiMediaTablesModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole) {
    return {};
  }
  if (not index.isValid()) {
    return {};
  }
  const int row = index.row();
  const int column = index.column();
  switch (column) {
    case 0:
      return m_data[row].tableName;
    case 1:
      return m_data[row].count;
    default:
      break;
  }
  return {};
}

QString AiMediaTablesModel::fileName(const QModelIndex& index) const {
  if (index.isValid() && 0 <= index.row() and index.row() < rowCount()) {
    return m_data[index.row()].tableName;
  }
  LOG_W("invalid index");
  return {};
}

void AiMediaTablesModel::UpdateData(QList<DupTableModelData> newData) {
  int beforeRow = m_data.size(), afterRow = newData.size();
  if (beforeRow < afterRow) {
    beginInsertRows({}, beforeRow, afterRow - 1);
    m_data.swap(newData);
    endInsertRows();
  } else if (beforeRow > afterRow) {
    beginRemoveRows({}, afterRow, beforeRow - 1);
    m_data.swap(newData);
    endRemoveRows();
  } else {
    m_data.swap(newData);
  }
  emit dataChanged(index(0, 0, {}), index(afterRow - 1, columnCount() - 1, {}), {Qt::ItemDataRole::DisplayRole});
  LOG_D("Ai Media rowCount %d->%d", beforeRow, afterRow);
}
