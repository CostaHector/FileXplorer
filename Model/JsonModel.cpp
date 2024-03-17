#include "JsonModel.h"
#include <QBrush>
#include <QDir>
#include <QDirIterator>

#include "Tools/JsonFileHelper.h"

JsonProperties::JsonProperties(const QString& path) : jsonPath{path}, perfsCount{getPerfsCount(path)} {}

int JsonProperties::getPerfsCount(const QString& pth) {
  const auto& dict = JsonFileHelper::MovieJsonLoader(pth);
  if (dict.isEmpty() or not dict.contains(DB_HEADER_KEY::Performers)) {
    return 0;
  }
  return dict[DB_HEADER_KEY::Performers].toJsonArray().size();
}

JsonModel::JsonModel(QObject* parent)
    : QAbstractListModel{parent},
      m_completeJsonPerfCount{
          PreferenceSettings()
              .value(MemoryKey::COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT.name, MemoryKey::COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT.v)
              .toInt()} {}

QModelIndex JsonModel::setRootPath(const QString& path) {
  // m_jsons increase delta
  m_rootPath = path;
  if (not QDir(path).exists()) {
    return QModelIndex();
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
  beginInsertRows(QModelIndex(), m_jsons.size(), m_jsons.size() + m_jsonsDelta.size() - 1);
  m_jsons += m_jsonsDelta;
  endInsertRows();
  return QModelIndex();
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

void JsonModel::updatePerfCount(int row) {
  const int count = JsonProperties::getPerfsCount(m_jsons[row].jsonPath);
  m_jsons[row].perfsCount = count;
  emit dataChanged(index(row), index(row), {Qt::ItemDataRole::ForegroundRole});
}
