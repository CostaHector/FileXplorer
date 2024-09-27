#include "FolderNxtAndLastIterator.h"
#include <QFileInfo>
#include <QDir>

void FolderNxtAndLastIterator::operator()(const QString& parentPath) {
  if (path2SameLevelPaths.contains(parentPath)) {
    return;
  }
  path2SameLevelPaths[parentPath] = QDir(parentPath, "", QDir::SortFlag::DirsFirst, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot).entryList();
}

QString FolderNxtAndLastIterator::lastNextCore(const QString& parentPath, const QString& curDirName, bool isNext) {
  operator()(parentPath);
  QStringList::const_iterator beg = path2SameLevelPaths[parentPath].cbegin(), end = path2SameLevelPaths[parentPath].cend();
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
      return *beg;
    }
    return *it;
  } else {
    QStringList::const_iterator it = std::lower_bound(beg, end, curDirName);
    if (it == beg) {
      return *(end - 1);
    }
    return *(it - 1);
  }
}
