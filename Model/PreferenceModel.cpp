#include "PreferenceModel.h"
#include "PublicVariable.h"

const QStringList AlertItem::ALERT_TABLE_HEADER{"ID", "Name", "Value", "Note", "isOk"};

AlertItem::AlertItem(int alarmID_, const KV* kv_, QString note_)
    : m_kv(kv_), alarmID(alarmID_), name(m_kv->name), value{getStoredValue()}, note(note_), checkRes{isStoredValuePass()} {}

QVariant AlertItem::getStoredValue() {
  if (m_kv == nullptr) {
    return "";
  }
  return PreferenceSettings().value(m_kv->name, m_kv->v);
}

bool AlertItem::isStoredValuePass() const {
  if (m_kv == nullptr) {
    return true;
  }
  const QVariant& v = PreferenceSettings().value(m_kv->name, m_kv->v);
  return m_kv->checker(v);
}

bool AlertItem::isPass(const QVariant& userInput) const {
  if (m_kv == nullptr) {
    return false;
  }
  return m_kv->checker(userInput);
}

QIcon AlertItem::GetAlertIcon(const AlertItem& alert) {
  return alert.checkRes ? QIcon(":/themes/PASS") : QIcon(":/themes/FAILED");
}

void AlertItem::setValue(const QVariant& newValue) {
  value = newValue;  // change value and pass
  checkRes = isPass(newValue);
  if (checkRes) {
    PreferenceSettings().setValue(name, newValue);
  }
}

QString AlertItem::defaultValue2Str() const {
  return m_kv->valueToString();
}
QString AlertItem::value2Str() const {
  return m_kv->valueToString(value);
}

PreferenceModel::PreferenceModel(QObject* parent) : QAbstractTableModel{parent} {}

QModelIndex PreferenceModel::setRootPath(const QString&) {
#ifdef _WIN32
  m_alerts.append(
      AlertItem{10000, &MemoryKey::WIN32_PERFORMERS_TABLE, "Used in Json editor. Provide the ability to filter performers out from giving string"});
  m_alerts.append(AlertItem{10001, &MemoryKey::WIN32_AKA_PERFORMERS, "Used in Quick Where Window to join where clause."});
  m_alerts.append(AlertItem{10002, &MemoryKey::WIN32_STANDARD_STUDIO_NAME, "Used in Json Editor to guess studio name."});
  m_alerts.append(AlertItem{10003, &MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH, "Reveal in terminal (Windows)."});
  m_alerts.append(AlertItem{10004, &MemoryKey::WIN32_RUNLOG, "Used in log records when some file operation failed."});
  m_alerts.append(AlertItem{10005, &MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE, "Used in Performers Widget"});
  m_alerts.append(AlertItem{10006, &MemoryKey::WIN32_MEDIAINFO_LIB_PATH, "Used in videos duration get"});
  m_alerts.append(AlertItem{10007, &MemoryKey::WIN32_RUND_IMG_PATH, "Used in redundant images find"});
#else
  m_alerts.append(
      AlertItem{10000, &MemoryKey::LINUX_PERFORMERS_TABLE, "Used in Json editor. Provide the ability to filter performers out from giving string"});
  m_alerts.append(AlertItem{10001, &MemoryKey::LINUX_AKA_PERFORMERS, "Used in Quick Where Window to join where clause."});
  m_alerts.append(AlertItem{10002, &MemoryKey::LINUX_STANDARD_STUDIO_NAME, "Used in Json Editor to guess studio name."});
  m_alerts.append(AlertItem{10004, &MemoryKey::LINUX_RUNLOG, "Used in log records when some file operation failed."});
  m_alerts.append(AlertItem{10005, &MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE, "Used in Performers Widget"});
  m_alerts.append(AlertItem{10006, &MemoryKey::LINUX_MEDIAINFO_LIB_PATH, "Used in videos duration get"});
  m_alerts.append(AlertItem{10007, &MemoryKey::LINUX_RUND_IMG_PATH, "Used in redundant images find"});
#endif
  return QModelIndex();
}

int PreferenceModel::failCount() const {
  int fails = 0;
  foreach (const AlertItem& item, m_alerts) {
    fails += (1 - item.checkRes);  // 1 - bool, revert
  }
  return fails;
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
      case 4:
        return record.checkRes;
      default:
        return QVariant();
    }
  } else if (role == Qt::DecorationRole) {
    if (index.column() == 0) {
      return AlertItem::GetAlertIcon(m_alerts[index.row()]);
    }
    return QVariant();
  } else if (role == Qt::TextAlignmentRole) {
    return int(Qt::AlignRight | Qt::AlignVCenter);
  }
  return QVariant();
}

bool PreferenceModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role == Qt::EditRole and index.column() == 2) {  // 2: value
    // for direct edit, using EditRole
    m_alerts[index.row()].setValue(value);
    // check is value valid. if valid write into Preference setting;
    emit dataChanged(index, index, {Qt::DisplayRole});
  }
  return QAbstractItemModel::setData(index, value, role);
}
