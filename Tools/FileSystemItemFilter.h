#ifndef FILESYSTEMITEMFILTER_H
#define FILESYSTEMITEMFILTER_H
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
class FileSystemItemFilter {
 public:
  FileSystemItemFilter() = default;

  struct ItemStatistic {
    int fileCnt = 0;
    int folderCnt = 0;
    qint64 fileSize = 0;
  };

  static ItemStatistic ItemCounter(const QStringList& items);
  static QStringList FilesOut(const QStringList& items, const QStringList& nameFilters = {});
  static QStringList MP4Out(const QStringList& items);
};

#endif  // FILESYSTEMITEMFILTER_H
