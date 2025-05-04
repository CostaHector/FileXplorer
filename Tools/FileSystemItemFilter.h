#ifndef FILESYSTEMITEMFILTER_H
#define FILESYSTEMITEMFILTER_H
#include <QStringList>
namespace FileSystemItemFilter {
  struct ItemStatistic {
    int fileCnt = 0;
    int folderCnt = 0;
    qint64 fileSize = 0;
  };

  ItemStatistic ItemCounter(const QStringList& items);
  QStringList FilesOut(const QStringList& items, const QStringList& nameFilters = {});
  QStringList MP4Out(const QStringList& items);
}

#endif  // FILESYSTEMITEMFILTER_H
