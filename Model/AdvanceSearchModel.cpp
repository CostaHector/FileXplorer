#include "AdvanceSearchModel.h"
#include "NotificatorMacro.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "DataFormatter.h"
#include <QMessageBox>

void AdvanceSearchModel::updateSearchResultList() {
  using namespace FilePropertyHelper;
  ClearRecycle();
  FilePropertyInfoList allItemsUnderThisPath;
  QDirIterator it{m_rootPath, m_filters, m_iteratorFlags};
  QString rel2searchItem;
  QString fileName;
  const int ROOT_PATH_N_WITH_NO_TRAILING_SLASH = m_rootPath.size();
  using namespace PathTool;
  while (it.hasNext()) {
    it.next();
    QFileInfo fi{it.fileInfo()};
    fileName = fi.fileName();
    rel2searchItem = GetRelPathFromRootRelName(ROOT_PATH_N_WITH_NO_TRAILING_SLASH, fi.filePath(), fileName.size());
    allItemsUnderThisPath.append(FilePropertyInfo{fileName, fi.size(), GetFileExtension(fileName), fi.lastModified(), rel2searchItem});
  }
  // C:/A/B/C
  // C:/A   file
  LOG_D("%d item(s) find out under path [%s] with QDir::Filters[%d]", allItemsUnderThisPath.size(), qPrintable(m_rootPath), int(m_filters));
  beginResetModel();
  allItemsUnderThisPath.swap(m_itemsLst);
  endResetModel();
}

void AdvanceSearchModel::forceRefresh() {
  if (!checkPathNeed(m_rootPath, true)) {
    return;
  }
  updateSearchResultList();
}

bool AdvanceSearchModel::checkPathNeed(const QString& path, const bool queryWhenSearchUnderLargeDirectory) {
  // when you need to call updateSearchResultList after checkPathNeed.
  // queryWhenSearchUnderLargeDirectory is most likely set to be true
  const QString& stdPath = PathTool::GetWinStdPath(path);
  if (stdPath.isEmpty()) {
    LOG_WARN_NP("[Abort]Search under empty path", stdPath);
    return false;
  }
  if (!QFileInfo(stdPath).isDir()) {
    LOG_WARN_NP("[Abort]Search under inexist path", stdPath);
    return false;
  }
  if (queryWhenSearchUnderLargeDirectory && stdPath.count('/') < 2) {              // C:/A
    auto retBtn = QMessageBox::warning(nullptr, "Confirm search(May cause lag)?",  //
                                       "A Large Directory: " + stdPath,            //
                                       QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel);
    if (retBtn != QMessageBox::StandardButton::Yes) {
      LOG_INFO_NP("User cancel search under large directory(may lag)", stdPath);
      return false;
    }
  }
  return true;
}

void AdvanceSearchModel::setRootPath(const QString& path) {
  ClearCopyAndCutDict();
  if (!checkPathNeed(path, true)) {  // first time
    return;
  }
  if (m_rootPath == path) {
    return;
  }
  m_rootPath = path;
  LOG_D("set rootPath:%s", qPrintable(m_rootPath));
  updateSearchResultList();
}

void AdvanceSearchModel::setFilter(QDir::Filters newFilters) {
  initFilter(newFilters);
  if (!checkPathNeed(m_rootPath, true)) {
    return;
  }
  updateSearchResultList();
  LOG_D("setFilter: %d", (int)m_filters);
}

QVariant AdvanceSearchModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  const int r = index.row();
  if (r < 0 || r >= rowCount()) {
    return {};
  }
  using namespace FilePropertyHelper;
  const auto& item = m_itemsLst[r];
  if (role == Qt::DisplayRole) {
    switch (index.column()) {
#define SEARCH_OUT_FILE_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) \
  case FilePropertyHelper::enu:                                              \
    return formatter(item.m_##enu);          //
      SEARCH_OUT_FILE_INFO_KEY_MAPPING_MAIN  //
#undef SEARCH_OUT_FILE_INFO_KEY_ITEM         //
          default : return {};
    }
  } else if (role == Qt::DecorationRole && index.column() == PropColumnE::Name) {
    if (mCutIndexes.contains(rootPath(), r)) {
      static const QIcon CUT_ICON{":img/CUT_ITEM"};
      return CUT_ICON;
    } else if (mCopyIndexes.contains(rootPath(), r)) {
      static const QIcon COPY_ICON{":img/COPY_ITEM"};
      return COPY_ICON;
    } else {
      static QHash<QString, QIcon> ext2Icon{{"", m_iconProvider.icon(QFileIconProvider::IconType::Folder)}};
      const QString& extExtDot{item.m_Type};
      auto it = ext2Icon.constFind(extExtDot);
      if (it == ext2Icon.constEnd()) {
        return ext2Icon[extExtDot] = m_iconProvider.icon(QFileInfo{extExtDot});
      }
      return it.value();
    }
  } else if (role == Qt::TextAlignmentRole) {
    if (index.column() == PropColumnE::Size) {
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
// headerData Template use me
QVariant AdvanceSearchModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Vertical) {
    if (role == Qt::DisplayRole) {
      return section + 1;
    } else if (role == Qt::TextAlignmentRole) {
      return Qt::AlignRight;
    }
  } else if (0 <= section && section < columnCount() && role == Qt::DisplayRole) {
    return FilePropertyHelper::SEARCH_TABLE_HEADERS[section];
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
