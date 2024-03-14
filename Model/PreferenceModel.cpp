#include "PreferenceModel.h"
#include "PublicVariable.h"

const QStringList AlertItem::ALERT_TABLE_HEADER{"Number", "Name", "Value", "Note"};

QIcon AlertItem::GetColor(const AlertItem& alert) {
  bool isPass = false;
  const QString& keyName = alert.name;
  const QString& keyValue = alert.value;
#ifdef _WIN32
  if (keyName == MemoryKey::WIN32_PERFORMERS_TABLE.name) {
    isPass = MemoryKey::WIN32_PERFORMERS_TABLE.checker(keyValue);
  } else if (keyName == MemoryKey::WIN32_AKA_PERFORMERS.name) {
    isPass = MemoryKey::WIN32_AKA_PERFORMERS.checker(keyValue);
  } else if (keyName == MemoryKey::WIN32_STANDARD_STUDIO_NAME.name) {
    isPass = MemoryKey::WIN32_STANDARD_STUDIO_NAME.checker(keyValue);
  } else if (keyName == MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH.name) {
    isPass = MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH.checker(keyValue);
  } else if (keyName == MemoryKey::WIN32_RUNLOG.name) {
    isPass = MemoryKey::WIN32_RUNLOG.checker(keyValue);
  } else if (keyName == MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name) {
    isPass = MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.checker(keyValue);
  }
#else
  if (keyName == MemoryKey::LINUX_PERFORMERS_TABLE.name) {
    isPass = MemoryKey::LINUX_PERFORMERS_TABLE.checker(keyValue);
  } else if (keyName == MemoryKey::LINUX_AKA_PERFORMERS.name) {
    isPass = MemoryKey::LINUX_AKA_PERFORMERS.checker(keyValue);
  } else if (keyName == MemoryKey::LINUX_STANDARD_STUDIO_NAME.name) {
    isPass = MemoryKey::LINUX_STANDARD_STUDIO_NAME.checker(keyValue);
  } else if (keyName == MemoryKey::LINUX_RUNLOG.name) {
    isPass = MemoryKey::LINUX_RUNLOG.checker(keyValue);
  } else if (keyName == MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name) {
    isPass = MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.checker(keyValue);
  }
#endif
  return isPass ? QIcon(":/themes/PASS") : QIcon(":/themes/FAILED");
}

PreferenceModel::PreferenceModel(QObject* parent) : QAbstractTableModel{parent} {}

QModelIndex PreferenceModel::setRootPath(const QString&) {
#ifdef _WIN32

  m_alerts.append(AlertItem{10000, MemoryKey::WIN32_PERFORMERS_TABLE.name,
                            PreferenceSettings().value(MemoryKey::WIN32_PERFORMERS_TABLE.name, MemoryKey::WIN32_PERFORMERS_TABLE.v).toString(),
                            "Used in Json editor. Provide the ability to filter performers out from giving string"});
  m_alerts.append(AlertItem{10001, MemoryKey::WIN32_AKA_PERFORMERS.name,
                            PreferenceSettings().value(MemoryKey::WIN32_AKA_PERFORMERS.name, MemoryKey::WIN32_AKA_PERFORMERS.v).toString(),
                            "Used in Quick Where Window to join where clause."});
  m_alerts.append(
      AlertItem{10002, MemoryKey::WIN32_STANDARD_STUDIO_NAME.name,
                PreferenceSettings().value(MemoryKey::WIN32_STANDARD_STUDIO_NAME.name, MemoryKey::WIN32_STANDARD_STUDIO_NAME.v).toString(),
                "Used in Json Editor to guess studio name."});
  m_alerts.append(AlertItem{
      10003, MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH.name,
      PreferenceSettings().value(MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH.name, MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH.v).toString(),
      "Reveal in terminal (Windows)."});
  m_alerts.append(AlertItem{10004, MemoryKey::WIN32_RUNLOG.name,
                            PreferenceSettings().value(MemoryKey::WIN32_RUNLOG.name, MemoryKey::WIN32_RUNLOG.v).toString(),
                            "Used in log records when some file operation failed."});
  m_alerts.append(
      AlertItem{10005, MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name,
                PreferenceSettings().value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v).toString(),
                "Used in Performers Widget"});
#else
  m_alerts.append(AlertItem{10000, MemoryKey::LINUX_PERFORMERS_TABLE.name, MemoryKey::LINUX_PERFORMERS_TABLE.v.toString(),
                            "Used in Json editor. Provide the ability to filter performers out from giving string"});
  m_alerts.append(AlertItem{10001, MemoryKey::LINUX_AKA_PERFORMERS.name, MemoryKey::LINUX_AKA_PERFORMERS.v.toString(),
                            "Used in Quick Where Window to join where clause."});
  m_alerts.append(AlertItem{10002, MemoryKey::LINUX_STANDARD_STUDIO_NAME.name, MemoryKey::LINUX_STANDARD_STUDIO_NAME.v.toString(),
                            "Used in Json Editor to guess studio name."});
  m_alerts.append(
      AlertItem{10004, MemoryKey::LINUX_RUNLOG.name, MemoryKey::LINUX_RUNLOG.v.toString(), "Used in log records when some file operation failed."});
  m_alerts.append(AlertItem{10005, MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v.toString(),
                            "Used in Performers Widget"});

#endif
}

QVariant PreferenceModel::data(const QModelIndex& index, int role) const {
  if (not index.isValid()) {
    return QVariant();
  }
  if (not(0 <= index.row() and index.row() < rowCount())) {
    return QVariant();
  }
  if (role == Qt::DisplayRole or role == Qt::EditRole) {
    const auto& record = m_alerts[index.row()];
    const int section = index.column();
    switch (section) {
      case 0:
        return record.alarmID;
      case 1:
        return record.name;
      case 2:
        return record.value;
      case 3:
        return record.note;
      default:
        return QVariant();
    }
  } else if (role == Qt::DecorationRole) {
    if (index.column() == 0) {
      return AlertItem::GetColor(m_alerts[index.row()]);
    }
    return QVariant();
  } else if (role == Qt::TextAlignmentRole) {
    return int(Qt::AlignRight | Qt::AlignVCenter);
  }
  return QVariant();
}

bool PreferenceModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role == Qt::EditRole and index.column() == 2) {  // 2: value
    m_alerts[index.row()].value = value.toString();
    // check is value valid. if valid write into Preference setting;
    emit dataChanged(index, index, {Qt::DisplayRole});
  }
  return QAbstractItemModel::setData(index, value, role);
}
