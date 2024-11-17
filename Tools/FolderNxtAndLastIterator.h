#ifndef FOLDERNXTANDLASTITERATOR_H
#define FOLDERNXTANDLASTITERATOR_H

#include <QString>
#include <QMap>

typedef QStringList (*FuncGetSortedDirNames)(const QString& path);

class FolderNxtAndLastIterator {
 public:
  explicit FolderNxtAndLastIterator(FuncGetSortedDirNames dirNamesGetter = nullptr);
  bool operator()(const QString& parentPath);

  QString next(const QString& parentPath, const QString& curDirName) {
    return lastNextCore(parentPath, curDirName, true);
  }
  QString last(const QString& parentPath, const QString& curDirName) {
    return lastNextCore(parentPath, curDirName, false);
  }
 private:
  QString lastNextCore(const QString& parentPath, const QString& curDirName, bool isNext = true);
  QString m_lastTimeParentPath;
  QStringList sameLevelPaths;
  FuncGetSortedDirNames m_dirNamesGetter;
};

#endif // FOLDERNXTANDLASTITERATOR_H
