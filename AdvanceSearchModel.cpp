#include "AdvanceSearchModel.h"
#include "Component/NotificatorFrame.h"
#include "PublicVariable.h"

const QStringList AdvanceSearchModel::HORIZONTAL_HEADER_NAMES = {"name", "size", "type", "date", "relative path"};

AdvanceSearchModel::AdvanceSearchModel(QObject* parent)
    : QAbstractTableModel(parent),
      m_filters{PreferenceSettings().value(MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.name, MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v).toInt()},
      m_iteratorFlags{bool2IteratorFlag(
          PreferenceSettings().value(MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.name, MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.v).toBool())} {}

void AdvanceSearchModel::BindLogger(CustomStatusBar* logger) {
  if (logger == nullptr) {
    qWarning("Don't try to bind nullptr to _logger");
    return;
  }
  if (_logger != nullptr) {
    qWarning("Don't try to rebind logger to non nullptr _logger");
    return;
  }
  _logger = logger;
}

auto AdvanceSearchModel::_updatePlanetList() -> void {
  ClearRecycle();
  if (m_rootPath.isEmpty()) {
    qDebug("reject do under path \"\"");
    return;
  }
  if (not QFileInfo(m_rootPath).isDir()) {
    return;
  }

  //  emit dataChanged(index(0, 0), index(r, c), {Qt::DisplayRole, Qt::DecorationRole});
  this->beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
  m_planetList.clear();
  this->endRemoveRows();

  decltype(m_planetList) newPlanetList;
  QDirIterator it(m_rootPath, m_filters, m_iteratorFlags);
  while (it.hasNext()) {
    it.next();
    QFileInfo fi = it.fileInfo();
    const auto name = fi.fileName();
    const auto absPath = fi.absoluteFilePath();
    newPlanetList.append(FileProperty{name, fi.size(), fi.suffix(), fi.lastModified(), absPath.left(absPath.size() - name.size())});
  }
  // C:/A/B/C
  // C:/A   file
  qDebug("> %d item(s) | QDir::Filters: %d | under [%s]", newPlanetList.size(), int(m_filters), qPrintable(m_rootPath));
  if (_logger) {
    _logger->pathInfo(newPlanetList.size(), 0);
    _logger->msg(QString("QDir::Filters: %1 | under [%2]").arg(int(m_filters)).arg(m_rootPath));
  }
  this->beginInsertRows(QModelIndex(), 0, newPlanetList.size() - 1);
  newPlanetList.swap(m_planetList);
  this->endInsertRows();
}

auto AdvanceSearchModel::checkPathNeed(const QString& path) const -> bool {
  if (path.isEmpty()) {
    return false;
  }
  if (not QFileInfo(path).isDir()) {
    return false;
  }
  if (path.count('/') < 2) {
    qWarning("[Search] Skip. Search under path[%s] will cause lag.", qPrintable(path));
    Notificator::warning("Search skip. Search under path[%s] will cause lag", path);
    return false;
  }

  return true;
}

auto AdvanceSearchModel::initRootPath(const QString& path) -> void {
  if (m_rootPath == path) {
    return;
  }
  if (not checkPathNeed(path)) {
    return;
  }
  m_rootPath = path;
  qDebug() << "AdvanceSearchModel::initRootPath(%s)" << m_rootPath;
}

auto AdvanceSearchModel::setRootPath(const QString& path) -> void {
  if (not checkPathNeed(path)) {
    return;
  }
  initRootPath(path);
  _updatePlanetList();
  qDebug() << "AdvanceSearchModel::setRootPath" << m_rootPath;
}

auto AdvanceSearchModel::initFilter(QDir::Filters initialFilters) -> void {
  m_filters = initialFilters;
}

auto AdvanceSearchModel::setFilter(QDir::Filters newFilters) -> void {
  initFilter(newFilters);
  if (not checkPathNeed(m_rootPath)) {
    return;
  }
  _updatePlanetList();
  qDebug() << "AdvanceSearchModel::setFilter" << m_filters;
}

void AdvanceSearchModel::setRootPathAndFilter(const QString& path, QDir::Filters filters) {
  if (m_rootPath == path and m_filters == filters) {
    return;
  }
  if (not checkPathNeed(path)) {
    return;
  }
  m_rootPath = path;
  m_filters = filters;
  _updatePlanetList();
}

QDirIterator::IteratorFlag AdvanceSearchModel::bool2IteratorFlag(const bool isIncludeEnabled) const {
  return isIncludeEnabled ? QDirIterator::IteratorFlag::Subdirectories : QDirIterator::IteratorFlag::NoIteratorFlags;
}

void AdvanceSearchModel::initIteratorFlag(QDirIterator::IteratorFlag initialFlags) {
  m_iteratorFlags = initialFlags;
}

QVariant AdvanceSearchModel::data(const QModelIndex& index, int role) const {
  if (not index.isValid()) {
    return QVariant();
  }
  if (not(0 <= index.row() and index.row() < rowCount())) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    switch (index.column()) {
      case 0:
        return m_planetList[index.row()].name;
      case 1:
        return m_planetList[index.row()].size;
      case 2:
        return m_planetList[index.row()].type;
      case 3:
        return m_planetList[index.row()].modifiedDate;
      case 4:
        return m_planetList[index.row()].relPath;
      default:
        return QVariant();
    }
  } else if (role == Qt::DecorationRole) {
    if (index.column() == 0) {
      return m_iconProvider.icon(QFileInfo(m_planetList[index.row()].relPath + m_planetList[index.row()].name));
    }
    return QVariant();
  } else if (role == Qt::TextAlignmentRole) {
    if (index.column() == 1) {
      // Todo  | Qt::AlignVCenter
      return Qt::AlignRight;
    }
    return int(Qt::AlignLeft | Qt::AlignTop);
  } else if (role == Qt::BackgroundRole) {
    if (m_cutMap.contains(rootPath()) and m_cutMap[rootPath()].contains(index)) {
      return QBrush(Qt::GlobalColor::darkGray, Qt::BrushStyle::Dense4Pattern);
    }
    if (m_copiedMap.contains(rootPath()) and m_copiedMap[rootPath()].contains(index)) {
      return QBrush(Qt::GlobalColor::yellow, Qt::BrushStyle::CrossPattern);
    }
    return QBrush(Qt::transparent);
  } else if (role == Qt::ForegroundRole) {
    if (m_disableList.contains(index)) {
      return QBrush(Qt::GlobalColor::lightGray);
    }
    if (m_recycleSet.contains(index)) {
      return QBrush(Qt::GlobalColor::red);
    }
    return QBrush(Qt::GlobalColor::black);
  }
  return QVariant();
}

auto AdvanceSearchModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  }
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Orientation::Horizontal) {
      return HORIZONTAL_HEADER_NAMES[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void AdvanceSearchModel::ClearCopyAndCutDict() {
  ClearCutDict();
  ClearCopiedDict();
}

void AdvanceSearchModel::ClearCutDict() {
  decltype(m_cutMap) tmp;
  tmp.swap(m_cutMap);
  // continous cut made index not exist but valid can emit not exist make it crash down
  return;
  for (auto it = tmp.cbegin(); it != tmp.cend(); ++it) {
    for (auto index : it.value()) {
      if (not index.isValid())
        continue;  // this index may by cut away. so may be invalid here
      emit dataChanged(index, index, {Qt::ItemDataRole::BackgroundRole});
    }
  }
}

void AdvanceSearchModel::ClearCopiedDict() {
  decltype(m_copiedMap) tmp;
  tmp.swap(m_copiedMap);
  for (auto it = tmp.cbegin(); it != tmp.cend(); ++it) {
    for (auto index : it.value()) {
      if (not index.isValid())
        continue;
      emit dataChanged(index, index, {Qt::ItemDataRole::BackgroundRole});
    }
  }
}

void AdvanceSearchModel::CutSomething(const QModelIndexList& cutIndexes, bool appendMode) {
  if (not appendMode) {
    ClearCutDict();
  }
  ClearCopiedDict();
  if (not m_cutMap.contains(rootPath())) {
    m_cutMap[rootPath()] = {};
  }
  m_cutMap[rootPath()] += cutIndexes;
}

void AdvanceSearchModel::CopiedSomething(const QModelIndexList& copiedIndexes, bool appendMode) {
  if (not appendMode) {
    ClearCopiedDict();
  }
  ClearCutDict();
  if (not m_copiedMap.contains(rootPath())) {
    m_copiedMap[rootPath()] = {};
  }
  m_copiedMap[rootPath()] += copiedIndexes;
}

void AdvanceSearchModel::appendDisable(const QModelIndex& ind) {
  if (m_disableList.contains(ind))
    return;
  m_disableList.insert(ind);
  emit dataChanged(ind, ind, {Qt::ForegroundRole});
}

void AdvanceSearchModel::removeDisable(const QModelIndex& ind) {
  if (not m_disableList.contains(ind))
    return;
  m_disableList.remove(ind);
  emit dataChanged(ind, ind, {Qt::ForegroundRole});
}

void AdvanceSearchModel::clearDisables() {
  decltype(m_disableList) tmp;
  tmp.swap(m_disableList);
  m_disableList.clear();
  foreach (const QModelIndex& ind, tmp) {
    emit dataChanged(ind, ind, {Qt::ForegroundRole});
  }
}

void AdvanceSearchModel::RecycleSomething(const QSet<QModelIndex>& recycleIndexes) {
  m_recycleSet += recycleIndexes;
}

void AdvanceSearchModel::ClearRecycle() {
  decltype(m_recycleSet) tmp;
  tmp.swap(m_recycleSet);
  foreach (const QModelIndex& ind, tmp) {
    emit dataChanged(ind, ind, {Qt::ItemDataRole::BackgroundRole});
  }
}
