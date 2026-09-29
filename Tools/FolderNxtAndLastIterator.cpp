#include "FolderNxtAndLastIterator.h"
#include "Logger.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include <QDirIterator>

FolderNxtAndLastIterator FolderNxtAndLastIterator::GetInstsNaviFolders() {
  return FolderNxtAndLastIterator{};
}

FolderNxtAndLastIterator FolderNxtAndLastIterator::GetInstsNaviImages(bool bIncludingSubDir) {
  return FolderNxtAndLastIterator{TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, bIncludingSubDir};
}

FolderNxtAndLastIterator::FolderNxtAndLastIterator(const QStringList& nameFilters, QDir::Filters dirFilters, bool bIncludingSubDir)
  : mNameFilters{nameFilters}
  , mDirFilters{dirFilters}
  , mIncludingSubDirectory{bIncludingSubDir} {}

bool FolderNxtAndLastIterator::operator()(const QString& parentPath, bool bForce) {
  if (!QFile::exists(parentPath)) {
    return false;
  }
  if (!bForce && (!m_lastTimeParentPath.isEmpty() && m_lastTimeParentPath == parentPath)) {
    // not first time && parentPath unchange => no update
    return false;
  }
  // first time || parentPath changed => update needed
  m_lastTimeParentPath = parentPath;

  QStringList items;
  if (mIncludingSubDirectory) {
    const int PRE_LEN_END_WITH_SLASH = parentPath.size() + 1;
    // parentPath="C:/home", it.next()="C:/home/to/a.txt", return: "to/a.txt"
    QDirIterator it{parentPath, mNameFilters, mDirFilters, QDirIterator::IteratorFlag::Subdirectories};
    while (it.hasNext()) {
      items.push_back(it.next().mid(PRE_LEN_END_WITH_SLASH));
    }
    items.sort(Qt::CaseSensitivity::CaseInsensitive); // mNameFilters同时筛选了文件/文件夹时, 混合排序
  } else {
    QDir dir{parentPath}; // 同时存在文件和文件夹时， 先文件夹
    items = dir.entryList(mNameFilters, mDirFilters, QDir::SortFlag::Name | QDir::SortFlag::DirsFirst | QDir::SortFlag::IgnoreCase);
  }
  const int cnt = items.size();
  initSameLevelPaths(items);
  LOG_D("Parent folder[%s] contains %d item(s)", qPrintable(parentPath), cnt);
  return true;
}

bool FolderNxtAndLastIterator::operator()(const QString& parentPath, QStringList& itemsListDisposable) {
  if (!m_lastTimeParentPath.isEmpty() && m_lastTimeParentPath == parentPath) {
    // not first time && parentPath unchange => no update
    return false;
  }
  // first time || parentPath changed => update needed
  m_lastTimeParentPath = parentPath;
  initSameLevelPaths(itemsListDisposable);
  return true;
}

QString FolderNxtAndLastIterator::GetDestinationItem(const QString& parentPath, const QString& curItemName, NaviDirection direction) const {
  const QString curItemNameLower = curItemName.toLower();
  QStringList::const_iterator beg = mSameLevelPathsLowercase.cbegin();
  QStringList::const_iterator end = mSameLevelPathsLowercase.cend();
  if (beg == end) {
    LOG_D("parent folder(%s) contains nothing", qPrintable(parentPath));
    return "";
  }
  if (beg + 1 == end) {
    LOG_D("contain only 1 directory");
    return mSameLevelPaths.front();
  }
  if (direction == NaviDirection::NEXT) {
    QStringList::const_iterator it = std::upper_bound(beg, end, curItemNameLower);
    if (it == end) {
      LOG_D("[%s] is already the last folder, wrapped to the first one", qPrintable(curItemName));
      return mSameLevelPaths.front();
    }
    return mSameLevelPaths[it - beg];
  } else {
    QStringList::const_iterator it = std::lower_bound(beg, end, curItemNameLower);
    if (it == beg) {
      LOG_D("[%s] is already the first folder, wrapped to the last one", qPrintable(curItemName));
      return mSameLevelPaths.back();
    }
    return mSameLevelPaths[it - 1 - beg];
  }
}

QString FolderNxtAndLastIterator::lastNextCore(const QString& parentPath, const QString& curItemName, NaviDirection direction) {
  operator()(parentPath, false);
  QString destItem = GetDestinationItem(parentPath, curItemName, direction);
  if (!destItem.isEmpty() && !QFile::exists(PathTool::Path2Join(parentPath, destItem))) {
    // Some folder in parent path being removed, need refresh existed folders nowss.
    this->operator()(m_lastTimeParentPath, true);
    destItem = GetDestinationItem(parentPath, curItemName, direction);
  }
  return destItem;
}
