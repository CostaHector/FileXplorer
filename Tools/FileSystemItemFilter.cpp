#include "FileSystemItemFilter.h"
#include "public/PublicVariable.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

FileSystemItemFilter::ItemStatistic FileSystemItemFilter::ItemCounter(const QStringList& items) {
  ItemStatistic itemStatistic;
  for (const QString& path : items) {
    const QFileInfo fi{path};
    if (fi.isFile()) {
      ++itemStatistic.fileCnt;
      itemStatistic.fileSize += fi.size();
    } else if (fi.isDir()) {
      ++itemStatistic.folderCnt;
      QDirIterator it{path,                                                                     //
                      {},                                                                       //
                      QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot,  //
                      QDirIterator::Subdirectories};
      while (it.hasNext()) {
        it.next();
        const QFileInfo subFi = it.fileInfo();
        if (subFi.isFile()) {
          ++itemStatistic.fileCnt;
          itemStatistic.fileSize += it.fileInfo().size();
        } else if (subFi.isDir()) {
          ++itemStatistic.folderCnt;
        }
      }
    }
  }
  return itemStatistic;
}

// for direct files in items must be "*.type" contained in nameFilters
// for files in items subdirs can be any "*" nameFilters
QStringList FileSystemItemFilter::FilesOut(const QStringList& items, const QStringList& nameFilters) {
  QStringList files;
  for (const QString& path : items) {
    const QFileInfo fi{path};
    if (fi.isFile()) {
      if (nameFilters.isEmpty() || nameFilters.contains("*." + fi.suffix())) {
        files << fi.absoluteFilePath();
      }
    } else if (fi.isDir()) {
      QDirIterator it(path, nameFilters, QDir::Filter::Files, QDirIterator::Subdirectories);
      while (it.hasNext()) {
        it.next();
        if (it.fileInfo().isFile()) {
          files << it.filePath();
        }
      }
    }
  }
  return files;
}

QStringList FileSystemItemFilter::MP4Out(const QStringList& items) {
  return FilesOut(items, TYPE_FILTER::VIDEO_TYPE_SET);
}
