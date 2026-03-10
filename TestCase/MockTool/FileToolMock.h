#ifndef FILETOOLMOCK_H
#define FILETOOLMOCK_H
#include <QString>
#include <QUrl>

namespace FileToolMock {
bool invokeOpenLocalFile(const QString& localFilePath);
bool invokeOpenLocalFileUsingDesktopService(const QString& localFilePath);
bool invokeOpenUrl(const QUrl &url);
}

#endif  // FILETOOLMOCK_H
