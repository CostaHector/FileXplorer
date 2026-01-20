#include "FileDescriptorMock.h"
#include <QHash>

namespace {
QHash<QString, QByteArray> g_abspath2Fd;
}

namespace FileDescriptorMock {
void PresetFileFds(QHash<QString, QByteArray> newFileFds) {
  g_abspath2Fd.swap(newFileFds);
}

QByteArray invokeGetFileUniquedId(const QString& fileAbsPath) {
  return g_abspath2Fd.value(fileAbsPath, "");
}
QList<QByteArray> invokeGetFileUniquedIds(const QStringList& files) {
  QList<QByteArray> fdsList;
  fdsList.reserve(files.size());
  for (const QString& file: files) {
    fdsList.push_back(g_abspath2Fd.value(file, ""));
  }
  return fdsList;
}

}
