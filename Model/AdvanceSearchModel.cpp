#include "AdvanceSearchModel.h"
#include "NotificatorMacro.h"
#include "PathTool.h"
#include "MemoryKey.h"
#include "PublicVariable.h"
#include <QMessageBox>

const QStringList AdvanceSearchModel::HORIZONTAL_HEADER_NAMES = {"Name", "Size", "Type", "Date", "Relative path"};

AdvanceSearchModel::AdvanceSearchModel(QObject* parent)
  : QAbstractTableModelPub{parent},
  m_filters{Configuration().value(MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.name, MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v).toInt()},
  m_iteratorFlags{bool2IteratorFlag(Configuration().value(MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.name, MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.v).toBool())}  //
{
}

void AdvanceSearchModel::updateSearchResultList() {
  ClearRecycle();
  decltype(m_itemsLst) newPlanetList;
  QDirIterator it{m_rootPath, m_filters, m_iteratorFlags};
  QString fileName;
  const int ROOT_PATH_N = m_rootPath.size() + 1;
  using namespace PathTool;
  while (it.hasNext()) {
    it.next();
    QFileInfo fi{it.fileInfo()};
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

bool AdvanceSearchModel::checkPathNeed(const QString& path, const bool queryWhenSearchUnderLargeDirectory) {
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
    LOG_INFO_NP("User cancel search under large directory(may lag)", stdPath);
    return false;
  }
  return true;
}

void AdvanceSearchModel::initRootPath(const QString& path) {
  if (m_rootPath == path) {
    return;
  }
  if (!checkPathNeed(path, false)) {
    return;
  }
  m_rootPath = path;
  qDebug("init rootPath:%s", qPrintable(m_rootPath));
}

void AdvanceSearchModel::setRootPath(const QString& path) {
  ClearCopyAndCutDict();
  if (!checkPathNeed(path)) {
    return;
  }
  initRootPath(path);
  qDebug("set rootPath:%s", qPrintable(m_rootPath));
  updateSearchResultList();
}

void AdvanceSearchModel::initFilter(QDir::Filters initialFilters) {
  m_filters = initialFilters;
}

void AdvanceSearchModel::setFilter(QDir::Filters newFilters) {
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
    return {};
  }
  const int r = index.row();
  if (r < 0 || r >= rowCount()) {
    return {};
  }
  if (role == Qt::DisplayRole) {
    switch (index.column()) {
      case 0:
        return m_itemsLst[r].name;
      case 1:
        return m_itemsLst[r].size;
      case 2:
        return m_itemsLst[r].type;
      case 3:
        return m_itemsLst[r].modifiedDate;
      case 4:
        return m_itemsLst[r].relPath;
      default:
        return {};
    }
  } else if (role == Qt::DecorationRole && index.column() == 0) {
    if (mCutIndexes.contains(rootPath(), r)) {
      static const QIcon CUT_ICON{":img/CUT_ITEM"};
      return CUT_ICON;
    } else if (mCopyIndexes.contains(rootPath(), r)) {
      static const QIcon COPY_ICON{":img/COPY_ITEM"};
      return COPY_ICON;
    } else {
      static QHash<QString, QIcon> ext2Icon{{"", m_iconProvider.icon(QFileIconProvider::IconType::Folder)}};
      const QString& extExtDot{m_itemsLst[r].type};
      auto it = ext2Icon.constFind(extExtDot);
      if (it == ext2Icon.constEnd()) {
        return ext2Icon[extExtDot] = m_iconProvider.icon(QFileInfo{extExtDot});
      }
      return it.value();
    }
  } else if (role == Qt::TextAlignmentRole) {
    if (index.column() == 1) {
      // Todo  | Qt::AlignVCenter
      return Qt::AlignRight;
    }
    return int(Qt::AlignLeft | Qt::AlignTop);
  } else if (role == Qt::ForegroundRole) {
    if (m_disableList.contains(index)) {
      static const QColor DISABLE_COLOR{Qt::GlobalColor::lightGray};
      return DISABLE_COLOR;
    } else if (m_recycleSet.contains(index)) {
      static const QColor RECYCLE_COLOR{Qt::GlobalColor::red};
      return RECYCLE_COLOR;
    }
  }
  return {};
}

QVariant AdvanceSearchModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Orientation::Horizontal) {
    if (role == Qt::DisplayRole) {
      return HORIZONTAL_HEADER_NAMES[section];
    }
  } else if (orientation == Qt::Vertical) {
    if (role == Qt::TextAlignmentRole) {
      return Qt::AlignRight;
    } else if (role == Qt::DisplayRole){
      return section + 1;
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
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
