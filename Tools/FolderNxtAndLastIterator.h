#ifndef FOLDERNXTANDLASTITERATOR_H
#define FOLDERNXTANDLASTITERATOR_H

#include <QString>
#include <QMap>

class FolderNxtAndLastIterator {
 public:
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
};

#endif // FOLDERNXTANDLASTITERATOR_H
