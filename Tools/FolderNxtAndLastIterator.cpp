#include "FolderNxtAndLastIterator.h"
#include <QFileInfo>
#include <QDir>

void FolderNxtAndLastIterator::operator()(const QString& currentPath) {
  const QString& parentPath = QFileInfo(currentPath).absolutePath();
  if (path2SameLevelPaths.contains(parentPath)) {
    return;
  }
  path2SameLevelPaths[parentPath] = QDir(parentPath, "", QDir::SortFlag::DirsFirst, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot).entryList();
}

QString FolderNxtAndLastIterator::lastNextCore(const QString& currentPath, bool isNext) {
  operator()(currentPath);
  const QFileInfo fi{currentPath};
  const QString& parentPath = fi.absolutePath();
  const QString& dirName = fi.fileName();
  QStringList::const_iterator beg = path2SameLevelPaths[parentPath].cbegin(), end = path2SameLevelPaths[parentPath].cend();
  if (beg == end) {
    qDebug("parent of folder(%s) contains nothing", qPrintable(currentPath));
    return "";
  }
  if (beg + 1 == end) {
    // contain only 1 directory
    return *beg;
  }
  if (isNext) {
    QStringList::const_iterator it = std::upper_bound(beg, end, dirName);
    if (it == end) {
      return *beg;
    }
    return *it;
  } else {
    QStringList::const_iterator it = std::lower_bound(beg, end, dirName);
    if (it == beg) {
      return *(end - 1);
    }
    return *(it - 1);
  }
}
