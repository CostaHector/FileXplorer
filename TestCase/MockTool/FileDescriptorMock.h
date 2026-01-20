#ifndef FILEDESCRIPTORMOCK_H
#define FILEDESCRIPTORMOCK_H
#include <QStringList>

namespace FileDescriptorMock {
void PresetFileFds(QHash<QString, QByteArray> newFileFds);
QByteArray invokeGetFileUniquedId(const QString& fileAbsPath);
QList<QByteArray> invokeGetFileUniquedIds(const QStringList& files);
}

#endif  // FILEDESCRIPTORMOCK_H
