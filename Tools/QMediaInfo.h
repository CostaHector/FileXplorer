#ifndef QMEDIAINFO_H
#define QMEDIAINFO_H

#include <QLibrary>
#include <QTime>
#include <iostream>
#include <string>
#include "lib/MediaInfoDLL.h"


#ifdef _WIN32
typedef wchar_t MediaInfo_Char;
#else
typedef char MediaInfo_Char;
#endif

// typedef void* (*MEDIAINFO_New)();
// typedef void (*MEDIAINFO_Delete)(void*);
// typedef size_t (*MEDIAINFO_Open)(void*, const MediaInfo_Char *);
// typedef MediaInfo_Char* (*MEDIAINFO_Inform)(void*, long);
// typedef MediaInfo_Char* (*MEDIAINFO_Get)(void*, int, int, MediaInfo_Char *, int, int);


class QMediaInfo {
 public:
  QMediaInfo();
  bool IsLoaded() const;

  bool StartToGet();
  int VidDurationLengthQuick(const QString& vidAbsPath) const;
  void EndToGet();

  int VidDurationLength(const QString& vidAbsPath) const;
  QList<int> batchVidsDurationLength(const QStringList& vidsAbsPath) const;

  QString CompleteName() const;
  QString Filename() const;
  QString FileExtension() const;
  QString Title() const;
  QString TitleMore() const;
  QString Domain() const;
  QString Collection() const;
  QString Season() const;
  QString Movie() const;
  QString Description() const;
  QString Inform() const;
  QString Duration() const;
  int DurationLength() const;
  QString FileSize() const;
  QString FormatExt() const;
  QString VideoBitRate() const;
  QString AudioBitRate() const;
  QString VideoResolution() const;
  QString VideoFrameRate() const;
  QString VideoCodec() const;
  QString AudioCodec() const;
  QString AudioLanguages() const;
  QString SubtitleLanguages() const;

  bool Open(const QString& filename);
  ~QMediaInfo();

 private:
  const MediaInfo_Char* Get(MediaInfo_stream_C streamKind,
                     int streamNumber,
                     MediaInfo_Char* parameter,
                     MediaInfo_info_C infoKind,
                     MediaInfo_info_C searchKind) const;
  int StreamCount(MediaInfo_stream_C stream) const;

  QLibrary* _lib;
  void* _pMedia;

  MEDIAINFO_Get m_get{nullptr};
  MEDIAINFO_Open m_open{nullptr};
  static const MediaInfo_Char m_prop[];
};

#endif  // QMEDIAINFO_H
