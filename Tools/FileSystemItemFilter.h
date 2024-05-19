#ifndef FILESYSTEMITEMFILTER_H
#define FILESYSTEMITEMFILTER_H
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
class FileSystemItemFilter {
 public:
  FileSystemItemFilter();

  struct ItemStatistic {
    int fileCnt = 0;
    int folderCnt = 0;
    qint64 fileSize = 0;
  };

  static ItemStatistic ItemCounter(const QStringList& items) {
    ItemStatistic itemStatistic;
    for (const QString& path : items) {
      QFileInfo fi(path);
      if (fi.isFile()) {
        ++itemStatistic.fileCnt;
        itemStatistic.fileSize += fi.size();
      } else if (fi.isDir()) {
        ++itemStatistic.folderCnt;
        QDirIterator it(path, QDir::Filter::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
          it.next();
          QFileInfo subFi = it.fileInfo();
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

  static QStringList FilesOut(const QStringList& items, const QStringList& nameFilters = {});
  static QStringList MP4Out(const QStringList& items);
};

#endif  // FILESYSTEMITEMFILTER_H
