#include "FileToolMock.h"
#include <QFile>

namespace FileToolMock {

bool invokeOpenLocalFile(const QString& localFilePath) {
  return QFile::exists(localFilePath);
}

bool invokeOpenLocalFileUsingDesktopService(const QString& localFilePath) {
  return QFile::exists(localFilePath);
}

bool invokeOpenUrl(const QUrl &url) {
  if (url.isLocalFile()) {
    return QFile::exists(url.toLocalFile());
  }
  return true;
}

}
