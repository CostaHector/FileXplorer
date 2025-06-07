#include "AdvanceSearchModel.h"
#include "Component/Notificator.h"
#include "public/PathTool.h"
#include "public/MemoryKey.h"
#include "public/PublicVariable.h"
#include <QMessageBox>

const QStringList AdvanceSearchModel::HORIZONTAL_HEADER_NAMES = {"name", "size", "type", "date", "relative path"};

AdvanceSearchModel::AdvanceSearchModel(QObject* parent)
    : QAbstractTableModelPub{parent},
      m_filters{PreferenceSettings().value(MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.name, MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v).toInt()},
      m_iteratorFlags{bool2IteratorFlag(PreferenceSettings().value(MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.name, MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.v).toBool())}  //
{
  m_ext2Icon[""] = m_iconProvider.icon(QFileIconProvider::IconType::Folder);
}

void AdvanceSearchModel::updateSearchResultList() {
  ClearRecycle();
  decltype(m_itemsLst) newPlanetList;
  QDirIterator it{m_rootPath, m_filters, m_iteratorFlags};
  QString fileName;
  const int ROOT_PATH_N = m_rootPath.size();
  while (it.hasNext()) {
    using namespace PathTool;
    it.next();
    QFileInfo fi = it.fileInfo();
    fileName = fi.fileName();
    newPlanetList.append(FileProperty{
        fileName, fi.size(),                                            //
        GetFileExtension(fileName),                                     //
        fi.lastModified(),                                              //
        RelativePath2File(ROOT_PATH_N, fi.filePath(), fileName.size())  //
    });
  }
  // C:/A/B/C
  // C:/A   file
  qDebug("%d item(s) find out under path [%s] with QDir::Filters[%d]", newPlanetList.size(), qPrintable(m_rootPath), int(m_filters));
  beginResetModel();
  newPlanetList.swap(m_itemsLst);
  endResetModel();
}

auto AdvanceSearchModel::checkPathNeed(const QString& path, const bool queryWhenSearchUnderLargeDirectory) -> bool {
  // when you need to call updateSearchResultList after checkPathNeed.
  // queryWhenSearchUnderLargeDirectory is most likely set to be true
  const QString& stdPath = PathTool::GetWinStdPath(path);
  if (stdPath.isEmpty()) {
    qWarning("reject search under empty path");
    return false;
  }
  if (!QFileInfo(stdPath).isDir()) {
    qWarning("reject search under inexist path[%s]", qPrintable(stdPath));
    return false;
  }
  if (queryWhenSearchUnderLargeDirectory && stdPath.count('/') < 2) {  // C:/A
    auto retBtn = QMessageBox::warning(nullptr,
                                       "Confirm search?",              //
                                       "large directory: " + stdPath,  //
                                       QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel);
    if (retBtn == QMessageBox::StandardButton::Yes) {
      return true;
    }
    qInfo("User cancel search under large directory[%s] as it may cause lag", qPrintable(stdPath));
    Notificator::information("User cancel search under large directory[%s] as it may cause lag", stdPath);
    return false;
  }
  return true;
}

auto AdvanceSearchModel::initRootPath(const QString& path) -> void {
  if (m_rootPath == path) {
    return;
  }
  if (!checkPathNeed(path, false)) {
    return;
  }
  m_rootPath = path;
  qDebug("init rootPath:%s", qPrintable(m_rootPath));
}

auto AdvanceSearchModel::setRootPath(const QString& path) -> void {
  if (!checkPathNeed(path)) {
    return;
  }
  initRootPath(path);
  qDebug("set rootPath:%s", qPrintable(m_rootPath));
  updateSearchResultList();
}

auto AdvanceSearchModel::initFilter(QDir::Filters initialFilters) -> void {
  m_filters = initialFilters;
}

auto AdvanceSearchModel::setFilter(QDir::Filters newFilters) -> void {
  initFilter(newFilters);
  if (!checkPathNeed(m_rootPath)) {
    return;
  }
  updateSearchResultList();
  qDebug("setFilter: %d", (int)m_filters);
}

void AdvanceSearchModel::setRootPathAndFilter(const QString& path, QDir::Filters filters) {
  if (m_rootPath == path && m_filters == filters) {
    return;
  }
  if (!checkPathNeed(path)) {
    return;
  }
  m_rootPath = path;
  m_filters = filters;
  updateSearchResultList();
}

QDirIterator::IteratorFlag AdvanceSearchModel::bool2IteratorFlag(const bool isIncludeEnabled) const {
  return isIncludeEnabled ? QDirIterator::IteratorFlag::Subdirectories : QDirIterator::IteratorFlag::NoIteratorFlags;
}

void AdvanceSearchModel::initIteratorFlag(QDirIterator::IteratorFlag initialFlags) {
  m_iteratorFlags = initialFlags;
}

QVariant AdvanceSearchModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }
  if (index.row() < 0 || index.row() >= rowCount()) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    switch (index.column()) {
      case 0:
        return m_itemsLst[index.row()].name;
      case 1:
        return m_itemsLst[index.row()].size;
      case 2:
        return m_itemsLst[index.row()].type;
      case 3:
        return m_itemsLst[index.row()].modifiedDate;
      case 4:
        return m_itemsLst[index.row()].relPath;
      default:
        return QVariant();
    }
  } else if (role == Qt::DecorationRole) {
    if (index.column() == 0) {
      const QString& extExtDot{m_itemsLst[index.row()].type};
      auto it = m_ext2Icon.constFind(extExtDot);
      if (it != m_ext2Icon.constEnd()) {
        return *it;
      }
      return m_ext2Icon[extExtDot] = m_iconProvider.icon(QFileInfo(extExtDot));
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
    return {};
  }
  return QVariant();
}

QVariant AdvanceSearchModel::headerData(int section, Qt::Orientation orientation, int role) const {
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
  return;
  for (auto it = tmp.cbegin(); it != tmp.cend(); ++it) {
    for (auto ind : it.value()) {
      if (checkIndex(ind, CheckIndexOption::DoNotUseParent))
        continue;
      emit dataChanged(ind, ind, {Qt::ItemDataRole::BackgroundRole});
    }
  }
}

void AdvanceSearchModel::ClearCopiedDict() {
  decltype(m_copiedMap) tmp;
  tmp.swap(m_copiedMap);
  for (auto it = tmp.cbegin(); it != tmp.cend(); ++it) {
    for (auto ind : it.value()) {
      if (checkIndex(ind, CheckIndexOption::DoNotUseParent))
        continue;
      emit dataChanged(ind, ind, {Qt::ItemDataRole::BackgroundRole});
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
  if (!m_disableList.contains(ind))
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
