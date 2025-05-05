#include "FileDescriptor.h"
#include <QList>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// 将FILETIME转换为Unix时间戳（秒）
qint64 FileTimeToUnixTimestamp(const FILETIME& ft) {
  constexpr const auto WINDOWS_TICK = 10000000LL;          // 10^7，FILETIME的计数间隔是100纳秒
  constexpr const auto SEC_TO_UNIX_EPOCH = 11644473600LL;  // 1601年到1970年的时间间隔（秒）
  // Windows FILETIME是从1601年开始的，而Unix时间戳是从1970年开始的
  // 1601年到1970年的秒数差为：11644473600秒
  // 100纳秒 = 0.0000001秒，所以需要将FILETIME的100纳秒单位转换为秒
  // 公式：(ft.dwHighDateTime * (2^32) + ft.dwLowDateTime) / 10000000 - 11644473600
  // 转换为秒并减去1601年到1970年的秒数差
  return (((qint64)ft.dwHighDateTime << 32) | ft.dwLowDateTime)  //
             / WINDOWS_TICK                                      //
         - SEC_TO_UNIX_EPOCH;
}

qint64 FileDescriptor::GetFileUniquedId(const QString& fileAbsPath) {
#ifndef Q_OS_WIN
  qDebug("only support in windows system");
  return -1;
#endif
  const HANDLE hFile = CreateFileW((wchar_t*)fileAbsPath.utf16(),       //
                                   0,                                   //
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,  //
                                   NULL,                                //
                                   OPEN_EXISTING,                       //
                                   FILE_ATTRIBUTE_NORMAL,               //
                                   NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
//#define ERROR_FILE_NOT_FOUND __MSABI_LONG(2)
//#define ERROR_PATH_NOT_FOUND __MSABI_LONG(3)
//#define ERROR_TOO_MANY_OPEN_FILES __MSABI_LONG(4)
    const DWORD errorNo = GetLastError();
    qWarning("Get fd[%s] error: %lu", qPrintable(fileAbsPath), errorNo);
    return -(qint64)errorNo;
  }
  BY_HANDLE_FILE_INFORMATION fileInfo{0};
  if (!GetFileInformationByHandle(hFile, &fileInfo)) {
    CloseHandle(hFile);
  }
  CloseHandle(hFile);
  return ((qint64)fileInfo.nFileIndexHigh << 32) | fileInfo.nFileIndexLow;
}

QList<qint64> FileDescriptor::GetFileUniquedIds(const QStringList& files) {
#ifndef Q_OS_WIN
  qWarning("only support in windows system");
  return {};
#endif
  QList<qint64> fds;
  foreach (const QString& fileAbsPath, files) {
    fds << GetFileUniquedId(fileAbsPath);
  }
  return fds;
}
