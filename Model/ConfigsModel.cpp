#include "ConfigsModel.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include <QDebug>
#include <QIcon>
const QStringList ConfigsModel::CONFIGS_TABLE_HEADER{"Name", "Current value", "Initial value"};
constexpr int ConfigsModel::VALUE_COLUMN;

ConfigsModel::ConfigsModel(QObject* parent) : QAbstractTableModel{parent} {}

int ConfigsModel::rowCount(const QModelIndex& /*parent*/) const {
  return KV::mEditableKVs.size();
}

int ConfigsModel::failCount() const {
  static const QSettings& curCfg = Configuration();
  int fails = 0;
  for (const KV* record: KV::mEditableKVs) {
    fails += (1 - record->checker(curCfg.value(record->name)));  // 1 - bool, revert
  }
  return fails;
}

QVariant ConfigsModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  if (index.row() < 0 || index.row() >= rowCount()) {
    return {};
  }
  const KV* record = KV::mEditableKVs[index.row()];
  static const QSettings& curCfg = Configuration();
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (index.column()) {
      case 0: // name
        return record->name;
      case 1: // current value
        return record->valueToString(curCfg.value(record->name, record->v));
      case 2: // inital value
        return record->InitialValueToString();
      default:
        return {};
    }
  } else if (role == Qt::DecorationRole && index.column() == 0) {
    static const QIcon PASS_OR_NOT_ICONS_ARR[] {QIcon{":img/WRONG"}, QIcon{":img/CORRECT"}};
    return PASS_OR_NOT_ICONS_ARR[(int)record->checker(curCfg.value(record->name, record->v))];
  }
  return {};
}

bool ConfigsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role == Qt::EditRole && index.column() == VALUE_COLUMN) {
    // for direct edit, using EditRole
    const KV* record = KV::mEditableKVs[index.row()];
    if (!record->checker(value)) {
      qDebug() << "Edit keyName[new value] not pass the checker: " << record->name << "[" << value << "]";
      return false;
    }
    static auto& curCfg = Configuration();
    curCfg.setValue(record->name, value);
    emit dataChanged(index, index, {Qt::DisplayRole});
  }
  return QAbstractItemModel::setData(index, value, role);
}

QString ConfigsModel::filePath(const QModelIndex& index) const {
  if (!index.isValid()) {
    return "";
  }
  return data(index.siblingAtColumn(VALUE_COLUMN), Qt::DisplayRole).toString();
}
