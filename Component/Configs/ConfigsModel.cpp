#include "ConfigsModel.h"
#include "PublicVariable.h"
#include "Configuration.h"
#include "Logger.h"
#include "PathTool.h"
#include <QIcon>

constexpr const int ConfigsModel::EDITABLE_COLUMN;

constexpr const char* ConfigsModel::CONFIGS_TABLE_HEADER[];
constexpr const int ConfigsModel::COLUMNS_COUNT;

using namespace EditableConfigs;

int ConfigsModel::failCount() const {
  int succeedCnt = 0;
  for (const KV* record : mKVs) {
    const bool isPass{record->isCurValuePassChecker()};
    succeedCnt += isPass;
  }
  return rowCount() - succeedCnt;
}

QVariant ConfigsModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  if (index.row() < 0 || index.row() >= rowCount()) {
    return {};
  }
  const KV& record = *mKVs[index.row()];
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (index.column()) {
      case ColumnE::NAME: // name
        return record.name;
      case ColumnE::INITIAL_VALUE:
        return record.toVariant();
      case ColumnE::CURRENT_VALUE:
        return getConfig(record);
      case ColumnE::VALIDATIDATION_VALUE:
        return record.isCurValuePassChecker();
      default:
        return {};
    }
  } else if (role == Qt::DecorationRole) {
    if (index.column() == ColumnE::NAME) {
      return QIcon{record.iconUrlStr()};
    } else if (index.column() == ColumnE::VALIDATIDATION_VALUE) {
      static const QIcon PASS_OR_NOT_ICONS_ARR[]{QIcon{":img/WRONG"}, QIcon{":img/CORRECT"}};
      const bool isPass{record.isCurValuePassChecker()};
      return PASS_OR_NOT_ICONS_ARR[(int) isPass];
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
  const KV& record = *mKVs[index.row()];
  if (record.pChecker != nullptr && !record.pChecker(value)) {
    LOG_D("Edit %s[modifiedToValue] not pass the checker", qPrintable(record.name));
    return false;
  }
  setConfig(record, value);
  emit dataChanged(index.siblingAtColumn(VALIDATIDATION_VALUE), index.siblingAtColumn(VALIDATIDATION_VALUE), {Qt::DisplayRole, Qt::DecorationRole});
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
