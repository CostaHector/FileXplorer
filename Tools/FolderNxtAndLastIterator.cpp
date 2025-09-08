#include "FolderNxtAndLastIterator.h"
#include "Logger.h"
#include <QFileInfo>
#include <QDir>

bool FolderNxtAndLastIterator::operator()(const QString& parentPath) {
  if (!m_lastTimeParentPath.isEmpty() && m_lastTimeParentPath == parentPath) {
    // not first time && parentPath unchange => no update
    return false;
  }
  // first time || parentPath changed => update needed
  m_lastTimeParentPath = parentPath;

  sameLevelPaths = QDir(parentPath, "", QDir::SortFlag::DirsFirst, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot).entryList();
  LOG_D("folders[%s] count %d", qPrintable(parentPath), sameLevelPaths.size());
  return true;
}

bool FolderNxtAndLastIterator::operator()(const QString& parentPath, const QStringList& pathList) {
  if (!m_lastTimeParentPath.isEmpty() && m_lastTimeParentPath == parentPath) {
    // not first time && parentPath unchange => no update
    return false;
  }
  // first time || parentPath changed => update needed
  m_lastTimeParentPath = parentPath;
  sameLevelPaths = pathList;
  return true;
}

QString FolderNxtAndLastIterator::lastNextCore(const QString& parentPath, const QString& curDirName, bool isNext) {
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
  if (isNext) {
    QStringList::const_iterator it = std::upper_bound(beg, end, curDirName);
    if (it == end) {
      LOG_D("[%s] is already the last folder, wrapped to the first one", qPrintable(curDirName));
      return *beg;
    }
    return *it;
  } else {
    QStringList::const_iterator it = std::lower_bound(beg, end, curDirName);
    if (it == beg) {
      LOG_D("[%s] is already the first folder, wrapped to the last one", qPrintable(curDirName));
      return *(end - 1);
    }
    return *(it - 1);
  }
}
