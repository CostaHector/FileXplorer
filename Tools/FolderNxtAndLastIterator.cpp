#include "FolderNxtAndLastIterator.h"
#include <QFileInfo>
#include <QDir>

bool FolderNxtAndLastIterator::operator()(const QString& parentPath) {
  if (!m_lastTimeParentPath.isEmpty() && m_lastTimeParentPath == parentPath) {
    // not first time && parentPath unchange => no update
    return false;
  }
  // first time || parentPath changed => update needed
  m_lastTimeParentPath = parentPath;
  auto lvls = QDir(parentPath, "", QDir::SortFlag::DirsFirst, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot).entryList();
  sameLevelPaths.swap(lvls);
  return true;
}

QString FolderNxtAndLastIterator::lastNextCore(const QString& parentPath, const QString& curDirName, bool isNext) {
  operator()(parentPath);
  QStringList::const_iterator beg = sameLevelPaths.cbegin();
  QStringList::const_iterator end = sameLevelPaths.cend();
  if (beg == end) {
    qDebug("parent folder(%s) contains nothing", qPrintable(parentPath));
    return "";
  }
  if (beg + 1 == end) {
    // contain only 1 directory
    return *beg;
  }
  if (isNext) {
    QStringList::const_iterator it = std::upper_bound(beg, end, curDirName);
    if (it == end) {
      qDebug("[%s] is already the last folder, wrapped to the first one", qPrintable(curDirName));
      return *beg;
    }
    return *it;
  } else {
    QStringList::const_iterator it = std::lower_bound(beg, end, curDirName);
    if (it == beg) {
      qDebug("[%s] is already the first folder, wrapped to the last one", qPrintable(curDirName));
      return *(end - 1);
    }
    return *(it - 1);
  }
}
