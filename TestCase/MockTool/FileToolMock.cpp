#include "FileToolMock.h"
#include <QFile>

namespace FileToolMock {

bool invokeOpenLocalFileUsingDesktopService(const QString& localFilePath) {
  return QFile::exists(localFilePath);
}

}
