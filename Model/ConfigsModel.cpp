#include "ConfigsModel.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "Logger.h"
#include "PathTool.h"
#include <QIcon>

const QStringList ConfigsModel::CONFIGS_TABLE_HEADER{"Name", "Initial value", "Current value"};

bool ConfigsModel::isCfgPass(const QSettings& curCfg, const KV& record) {
  return record.checker(curCfg.value(record.name, record.v));
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
  const KV* record = kvs[index.row()];
  const QSettings& curCfg = Configuration();
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (index.column()) {
      case NAME:  // name
        return record->name;
      case INITIAL_VALUE:
        return record->v;
      case CURRENT_VALUE:
        return curCfg.value(record->name, record->v);
      default:
        return {};
    }
  } else if (role == Qt::DecorationRole && index.column() == NAME) {
    static const QIcon PASS_OR_NOT_ICONS_ARR[]{QIcon{":img/WRONG"}, QIcon{":img/CORRECT"}};
    const bool isPass{isCfgPass(curCfg, *record)};
    return PASS_OR_NOT_ICONS_ARR[(int)isPass];
  }
  return {};
}

bool ConfigsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role == Qt::EditRole && flags(index).testFlag(Qt::ItemFlag::ItemIsEditable)) {
    // for direct edit, using EditRole
    static const auto& kvs = KV::GetEditableKVs();
    const KV* record = kvs[index.row()];
    if (!record->checker(value)) {
      LOG_D("Edit %s[modifiedToValue] not pass the checker", qPrintable(record->name));
      return false;
    }
    QSettings& curCfg = Configuration();
    if (record->isPath()) {
      const QString& stdPath = PathTool::normPath(value.toString());
      curCfg.setValue(record->name, stdPath);
    } else {
      curCfg.setValue(record->name, value);
    }
    emit dataChanged(index.siblingAtColumn(NAME), index.siblingAtColumn(NAME), {Qt::DecorationRole});
    return true;
  }
  return QAbstractItemModel::setData(index, value, role);
}

QString ConfigsModel::filePath(const QModelIndex& index) const {
  if (!index.isValid()) {
    LOG_W("index invalid");
    return "";
  }
  return index.siblingAtColumn(CURRENT_VALUE).data().toString();
}
