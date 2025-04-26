#ifndef FILEDESCRIPTOR_H
#define FILEDESCRIPTOR_H

#include <QString>
class FileDescriptor {
 public:
  qint64 GetFileUniquedId(const QString& fileAbsPath);
  QList<qint64> GetFileUniquedIds(const QStringList& files);
};

#endif  // FILEDESCRIPTOR_H
