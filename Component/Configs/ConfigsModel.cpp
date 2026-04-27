#include "ConfigsModel.h"
#include "PublicVariable.h"
#include "Configuration.h"
#include "Logger.h"
#include "PathTool.h"
#include "ImageTool.h"
#include <QIcon>

constexpr const int ConfigsModel::EDITABLE_COLUMN;

constexpr const char* ConfigsModel::CONFIGS_TABLE_HEADER[];
constexpr const int ConfigsModel::COLUMNS_COUNT;

using namespace EditableConfigs;

ConfigsModel::ConfigsModel(QObject* parent)
  : QAbstractTableModel{parent} {
  RecheckAllCfg();
}

QVariant ConfigsModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  }
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Orientation::Horizontal) {
      return ConfigsModel::CONFIGS_TABLE_HEADER[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant ConfigsModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  const int r = index.row();
  if (r < 0 || r >= rowCount()) {
    return {};
  }
  const KV& record = *mKVs[r];
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (index.column()) {
      case ColumnE::NAME: // name
        return record.name;
      case ColumnE::INITIAL_VALUE:
        return record.toVariant();
      case ColumnE::CURRENT_VALUE:
        return getConfig(record);
      case ColumnE::VALIDATIDATION_VALUE:
        return mCfgCheckResult[r];
      default:
        return {};
    }
  } else if (role == Qt::DecorationRole) {
    if (index.column() == ColumnE::NAME) {
      return QIcon{record.iconUrlStr()};
    } else if (index.column() == ColumnE::VALIDATIDATION_VALUE) {
      return ImageTool::GetCheckResultIcon(mCfgCheckResult[r]);
    }
    return {};
  } else if (role == Qt::ToolTipRole) {
    return record.toolTipStr();
  } else if (role == ColumnE::DATA_TYPE_ROLE) {
    return record.dataType();
  }
  return {};
}

bool ConfigsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid()) {
    return false;
  }
  if (role != Qt::EditRole) {
    return false;
  }
  if (!flags(index).testFlag(Qt::ItemIsEditable)) {
    return false;
  }
  // for direct edit, using EditRole
  const int r = index.row();
  const KV& record = *mKVs[r];

  const bool bNewValuePassChecker{record.pChecker == nullptr || record.pChecker(value)};
  if (!bNewValuePassChecker) {
    LOG_W("new value(key:%s) not pass the checker. reject change", qPrintable(record.name));
    return false;
  }

  setConfig(record, value);

  if (mCfgCheckResult[r] != bNewValuePassChecker) {
    mCfgCheckResult[r] = bNewValuePassChecker;
    emit dataChanged(index.siblingAtColumn(VALIDATIDATION_VALUE), index.siblingAtColumn(VALIDATIDATION_VALUE), {Qt::DisplayRole, Qt::DecorationRole});
    emit failedCountChanged(failCount());
  }

  emit dataChanged(index, index, {Qt::DisplayRole, Qt::DecorationRole, Qt::EditRole});
  return true;
}

bool ConfigsModel::isPath(const QModelIndex& index) const {
  if (!index.isValid()) {
    LOG_W("index invalid");
    return false;
  }
  return mKVs[index.row()]->isPath();
}

QString ConfigsModel::filePath(const QModelIndex& index) const {
  if (!isPath(index)) {
    LOG_D("not path related row");
    return "";
  }
  return index.siblingAtColumn(CURRENT_VALUE).data().toString();
}

void ConfigsModel::RecheckAllCfg() {
  const int befCnt = failCount();
  for (int i = 0; i < EditableConfigs::EDTIABLE_ROWS_COUNT; ++i) {
    mCfgCheckResult[i] = EditableConfigs::mKVs[i]->isCurValuePassChecker();
  }
  const int aftCnt = failCount();
  emit dataChanged(index(0, VALIDATIDATION_VALUE), index(rowCount() - 1, VALIDATIDATION_VALUE), {Qt::DisplayRole, Qt::DecorationRole});
  if (aftCnt != befCnt) {
    emit failedCountChanged(aftCnt);
  }
}
