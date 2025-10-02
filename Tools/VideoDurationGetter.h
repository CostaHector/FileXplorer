#ifndef VIDEODURATIONGETTER_H
#define VIDEODURATIONGETTER_H
#include <QList>
#include <QString>
#ifdef _WIN32
#include "QMediaInfo.h"
#endif
void IsFFmpegInstalledOK();

class VideoDurationGetter {
 public:
  // units: ms
  static int ReadAVideo(const QString& vidPath);
  static QList<int> ReadVideos(const QStringList& vidsPath);
  bool StartToGet();
  int GetLengthQuick(const QString& vidPath);
  QList<int> GetLengthsQuick(const QStringList& vidsPath);
#ifdef _WIN32
  QMediaInfo mi;
#endif
 private:
  static constexpr int MILLISECONDS_PER_SECOND = 1000;
  static constexpr int MICROSECONDS_PER_MILLISECOND = 1000;
};

#endif  // VIDEODURATIONGETTER_H
