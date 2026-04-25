#include "ConfigsModel.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "Configuration.h"
#include "Logger.h"
#include "PathTool.h"
#include <QIcon>

constexpr int ConfigsModel::EDITABLE_COLUMN;
const QStringList ConfigsModel::CONFIGS_TABLE_HEADER{"Name", "Initial value", "Current value", "Validation"};

bool ConfigsModel::isCfgPass(const QSettings& curCfg, const KV& record) {
  return record.pChecker(curCfg.value(record.name, record.toVariant()));
}

int ConfigsModel::rowCount(const QModelIndex& /*parent*/) const {
  return KV::GetEditableKVs().size();
}

int ConfigsModel::failCount() const {
  const QSettings& curCfg = Configuration();
  int succeeCnt = 0;
  for (const KV* record : KV::GetEditableKVs()) {
    const bool isPass{isCfgPass(curCfg, *record)};
    succeeCnt += isPass;
  }
  return rowCount() - succeeCnt;
}

QVariant ConfigsModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  if (index.row() < 0 || index.row() >= rowCount()) {
    return {};
  }
  static const auto& kvs = KV::GetEditableKVs();
  const KV& record = *kvs[index.row()];
  const QSettings& curCfg = Configuration();
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (index.column()) {
      case ColumnE::NAME: // name
        return record.name;
      case ColumnE::INITIAL_VALUE:
        return record.toVariant();
      case ColumnE::CURRENT_VALUE:
        return curCfg.value(record.name, record.toVariant());
      case ColumnE::VALIDATIDATION_VALUE:
        return isCfgPass(curCfg, record);
      default:
        return {};
    }
  } else if (role == Qt::DecorationRole) {
    if (index.column() == ColumnE::NAME) {
      return QIcon{record.iconUrlStr()};
    } else if (index.column() == ColumnE::VALIDATIDATION_VALUE) {
      static const QIcon PASS_OR_NOT_ICONS_ARR[]{QIcon{":img/WRONG"}, QIcon{":img/CORRECT"}};
      const bool isPass{isCfgPass(curCfg, record)};
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
  if (role == Qt::EditRole && flags(index).testFlag(Qt::ItemFlag::ItemIsEditable)) {
    // for direct edit, using EditRole
    static const auto& kvs = KV::GetEditableKVs();
    const KV& record = *kvs[index.row()];
    if (!record.pChecker(value)) {
      LOG_D("Edit %s[modifiedToValue] not pass the checker", qPrintable(record.name));
      return false;
    }
    setConfig(record, value);
    emit dataChanged(index.siblingAtColumn(NAME), index.siblingAtColumn(NAME), {Qt::DecorationRole});
    return true;
  }
  return QAbstractItemModel::setData(index, value, role);
}

bool ConfigsModel::isPath(const QModelIndex& index) const {
  if (!index.isValid()) {
    LOG_W("index invalid");
    return false;
  }
  const auto& kvs = KV::GetEditableKVs();
  const KV& record = *kvs[index.row()];
  return record.isPath();
}

QString ConfigsModel::filePath(const QModelIndex& index) const {
  if (!index.isValid()) {
    LOG_W("index invalid");
    return "";
  }
  return index.siblingAtColumn(CURRENT_VALUE).data().toString();
}
