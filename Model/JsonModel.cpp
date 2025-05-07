#include "JsonModel.h"
#include <QBrush>
#include <QDir>
#include <QDirIterator>
#include "public/PublicVariable.h"
#include "public/PublicMacro.h"
#include "public/MemoryKey.h"
#include "Tools/JsonFileHelper.h"

JsonProperties::JsonProperties(const QString& path) : jsonPath{path}, perfsCount{getPerfsCount(path)} {}

int JsonProperties::getPerfsCount(const QString& pth) {
  const auto& dict = JsonFileHelper::MovieJsonLoader(pth);
  const auto it = dict.find(ENUM_TO_STRING(Cast));
  if (it == dict.cend()) {
    return 0;
  }
  return it.value().toStringList().size();
}

JsonModel::JsonModel(QObject* parent)
    : DifferRootFileSystemModel{parent},
      m_completeJsonPerfCount{PreferenceSettings().value(MemoryKey::COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT.name, MemoryKey::COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT.v).toInt()} {}

int JsonModel::appendAPath(const QString& path) {
  // m_jsons increase delta
  m_rootPath = path;
  if (not QDir(path).exists()) {
    qWarning("Cannot read jsons from inexist path[%s]", qPrintable(path));
    return 0;
  }
  QDirIterator it(path, {"*.json"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);

  decltype(m_jsons) m_jsonsDelta;
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    if (not m_uniqueSet.contains(jsonPath)) {
      m_uniqueSet.insert(jsonPath);
      m_jsonsDelta.append(JsonProperties{jsonPath});
    }
  }
  if (m_jsonsDelta.isEmpty()) {
    qWarning("No unique json found from path[%s]", qPrintable(path));
    return 0;
  }

  beginInsertRows(QModelIndex(), m_jsons.size(), m_jsons.size() + m_jsonsDelta.size() - 1);
  m_jsons += m_jsonsDelta;
  endInsertRows();
  return m_jsonsDelta.size();
}

int JsonModel::appendRows(const QStringList& lst) {
  decltype(m_jsons) m_jsonsDelta;
  for (const auto& jsonPath : lst) {
    if (not m_uniqueSet.contains(jsonPath)) {
      m_uniqueSet.insert(jsonPath);
      m_jsonsDelta.append(JsonProperties{jsonPath});
    }
  }
  if (m_jsonsDelta.isEmpty()) {
    qWarning("No unique json found from input [%d] item(s) list", lst.size());
    return 0;
  }
  beginInsertRows(QModelIndex(), m_jsons.size(), m_jsons.size() + m_jsonsDelta.size() - 1);
  m_jsons += m_jsonsDelta;
  endInsertRows();
  return m_jsonsDelta.size();
}

QVariant JsonModel::data(const QModelIndex& index, int role) const {
  if (not index.isValid()) {
    return QVariant();
  }
  if (not(0 <= index.row() and index.row() < rowCount())) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    const int section = index.column();
    switch (section) {
      case 0:
        return m_jsons[index.row()].jsonPath;
      default:
        return QVariant();
    }
  } else if (role == Qt::ItemDataRole::ForegroundRole) {
    if (m_jsons[index.row()].perfsCount < m_completeJsonPerfCount) {
      return QBrush(Qt::GlobalColor::red);
    }
    return QBrush(Qt::GlobalColor::black);
  } else if (role == Qt::TextAlignmentRole) {
    return int(Qt::AlignLeft);
  }
  return QVariant();
}

bool JsonModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role == Qt::DisplayRole and index.column() == 0) {  // 0: value
    m_jsons[index.row()].jsonPath = value.toString();
    emit dataChanged(index, index, {Qt::DisplayRole});
  }
  return QAbstractItemModel::setData(index, value, role);
}

void JsonModel::clear() {
  beginRemoveRows(QModelIndex(), 0, m_jsons.size() - 1);
  m_jsons.clear();
  m_uniqueSet.clear();
  endRemoveRows();
}

void JsonModel::SetCompletePerfCount(int newCount) {
  PreferenceSettings().setValue(MemoryKey::COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT.name, newCount);
  m_completeJsonPerfCount = newCount;
  emit dataChanged(index(0), index(rowCount() - 1), {Qt::ItemDataRole::ForegroundRole});
}

void JsonModel::updatePerfCount(int row, int newCnt) {
  if (m_jsons[row].perfsCount == newCnt) {
    return;
  }
  m_jsons[row].perfsCount = newCnt;
  emit dataChanged(index(row), index(row), {Qt::ItemDataRole::ForegroundRole});
}
