#ifndef FILETOOLMOCK_H
#define FILETOOLMOCK_H
#include <QString>
#include <QUrl>

namespace FileToolMock {
bool invokeOpenLocalFileUsingDesktopService(const QString& localFilePath);
bool invokeOpenUrl(const QUrl &url);
}

#endif  // FILETOOLMOCK_H
