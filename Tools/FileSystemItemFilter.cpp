#include "FileSystemItemFilter.h"
#include "PublicVariable.h"
FileSystemItemFilter::FileSystemItemFilter() {}

QStringList FileSystemItemFilter::FilesOut(const QStringList& items, const QStringList& nameFilters) {
  QStringList files;
  for (const QString& path : items) {
    QFileInfo fi(path);
    if (fi.isFile()) {
      files << fi.absoluteFilePath();
      continue;
    }
    if (fi.isDir()) {
      QDirIterator it(path, nameFilters, QDir::Filter::Files, QDirIterator::Subdirectories);
      while (it.hasNext()) {
        it.next();
        if (it.fileInfo().isFile()) {
          files << it.filePath();
        }
      }
      continue;
    }
  }
  return files;
}

QStringList FileSystemItemFilter::MP4Out(const QStringList& items) { return FilesOut(items, TYPE_FILTER::VIDEO_TYPE_SET); }
