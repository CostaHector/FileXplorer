#include "FolderNxtAndLastIterator.h"
#include "Logger.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include <QDir>
#include <QDirIterator>

FolderNxtAndLastIterator FolderNxtAndLastIterator::GetInstsNaviFolders() {
  return {};
}

FolderNxtAndLastIterator FolderNxtAndLastIterator::GetInstsNaviImages(bool bIncludingSubDir) {
  return FolderNxtAndLastIterator{TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, bIncludingSubDir};
}

FolderNxtAndLastIterator::FolderNxtAndLastIterator(const QStringList& nameFilters, QDir::Filters dirFilters, bool bIncludingSubDir)
    : mNameFilters{nameFilters}, mDirFilters{dirFilters}, mIncludingSubDirectory{bIncludingSubDir} {}

bool FolderNxtAndLastIterator::operator()(const QString& parentPath, bool bFalse) {
  if (!bFalse && (!m_lastTimeParentPath.isEmpty() && m_lastTimeParentPath == parentPath)) {
    // not first time && parentPath unchange => no update
    return false;
  }
  // first time || parentPath changed => update needed
  m_lastTimeParentPath = parentPath;

  sameLevelPaths.clear();
  if (mIncludingSubDirectory) {
    const int PRE_LEN_END_WITH_SLASH = parentPath.size() + 1;
    // parentPath="C:/home", it.next()="C:/home/to/a.txt", return: "to/a.txt"
    QDirIterator it{parentPath, mNameFilters, mDirFilters, QDirIterator::IteratorFlag::Subdirectories};
    while (it.hasNext()) {
      sameLevelPaths.push_back(it.next().mid(PRE_LEN_END_WITH_SLASH));
    }
    std::sort(sameLevelPaths.begin(), sameLevelPaths.end());
  } else {
    QDir dir{parentPath};
    sameLevelPaths = dir.entryList(mNameFilters, mDirFilters, QDir::SortFlag::Name | QDir::SortFlag::DirsFirst);
  }
  LOG_D("Parent folder[%s] contains %d item(s)", qPrintable(parentPath), sameLevelPaths.size());
  return true;
}

bool FolderNxtAndLastIterator::operator()(const QString& parentPath, const QStringList& itemsList) {
  if (!m_lastTimeParentPath.isEmpty() && m_lastTimeParentPath == parentPath) {
    // not first time && parentPath unchange => no update
    return false;
  }
  // first time || parentPath changed => update needed
  m_lastTimeParentPath = parentPath;
  sameLevelPaths = itemsList;
  return true;
}

QString FolderNxtAndLastIterator::lastNextCore(const QString& parentPath, const QString& curItemName, NaviDirection direction) {
  operator()(parentPath);
  QStringList::const_iterator beg = sameLevelPaths.cbegin();
  QStringList::const_iterator end = sameLevelPaths.cend();
  if (beg == end) {
    LOG_D("parent folder(%s) contains nothing", qPrintable(parentPath));
    return "";
  }
  if (beg + 1 == end) {
    LOG_D("contain only 1 directory");
    return *beg;
  }
  if (direction == NaviDirection::NEXT) {
    QStringList::const_iterator it = std::upper_bound(beg, end, curItemName);
    if (it == end) {
      LOG_D("[%s] is already the last folder, wrapped to the first one", qPrintable(curItemName));
      return *beg;
    }
    return *it;
  } else {
    QStringList::const_iterator it = std::lower_bound(beg, end, curItemName);
    if (it == beg) {
      LOG_D("[%s] is already the first folder, wrapped to the last one", qPrintable(curItemName));
      return *(end - 1);
    }
    return *(it - 1);
  }
}
