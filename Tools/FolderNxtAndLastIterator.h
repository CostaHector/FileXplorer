#ifndef FOLDERNXTANDLASTITERATOR_H
#define FOLDERNXTANDLASTITERATOR_H

#include <QString>
#include <QMap>

class FolderNxtAndLastIterator {
 public:
  void operator()(const QString& currentPath);

  QString next(const QString& currentPath) {
    return lastNextCore(currentPath, true);
  }
  QString last(const QString& currentPath) {
    return lastNextCore(currentPath, false);
  }
 private:
  QString lastNextCore(const QString& currentPath, bool isNext = true);
  QMap<QString, QStringList> path2SameLevelPaths;
};

#endif // FOLDERNXTANDLASTITERATOR_H
