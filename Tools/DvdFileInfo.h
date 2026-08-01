#ifndef DVDFILEINFO_H
#define DVDFILEINFO_H

#include <QString>

namespace DvdFileInfo {
// dvd file structure
// totalSize:
// totalDuration
// totalMD5
// size seperated by comma
// duration seperated by comma
// md5 seperated by comma

enum class DvdFileLineE {
  TOTAL_SIZE     = 0,   // 第1行：总大小
  TOTAL_DURATION = 1,   // 第2行：总时长
  TOTAL_MD5      = 2,   // 第3行：总MD5
};

QString ReadDvdFileLine(const QString& dvdFilePath, DvdFileLineE line);

inline int ReadTotalDurationFromDvdFile(const QString& dvdFilePath) {
  QString line = ReadDvdFileLine(dvdFilePath, DvdFileLineE::TOTAL_DURATION);
  return line.toInt();
}

inline qint64 ReadTotalFileSizeFromDvdFile(const QString& dvdFilePath) {
  QString line = ReadDvdFileLine(dvdFilePath, DvdFileLineE::TOTAL_SIZE);
  return line.toLongLong();
}

inline QByteArray ReadTotalMD5FromDvdFile(const QString& dvdFilePath) {
  QString line = ReadDvdFileLine(dvdFilePath, DvdFileLineE::TOTAL_MD5);
  return line.toLatin1();
}

bool GenerateDvdFile(const QString& videoTsPath, bool bOverrideWhenExist = false, bool* bFailed = nullptr);
};

#endif // DVDFILEINFO_H
