#ifndef FILEDESCRIPTOR_H
#define FILEDESCRIPTOR_H

#include <QStringList>
namespace FileDescriptor {
  QByteArray GetFileUniquedId(const QString& fileAbsPath);
  QList<QByteArray> GetFileUniquedIds(const QStringList& files);
}

#endif  // FILEDESCRIPTOR_H
