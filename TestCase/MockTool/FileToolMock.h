#ifndef FILETOOLMOCK_H
#define FILETOOLMOCK_H
#include <QString>

namespace FileToolMock {
bool invokeOpenLocalFileUsingDesktopService(const QString& localFilePath);
}

#endif  // FILETOOLMOCK_H
