#ifndef QMEDIAINFO_H
#define QMEDIAINFO_H

#include <iostream>
#include <string>
#include <QLibrary>
#include "MediaInfoDLL.h"
#include <QTime>

//typedef void* (*MEDIAINFO_New)();
//typedef void (*MEDIAINFO_Delete)(void*);
//typedef size_t (*MEDIAINFO_Open)(void*, const wchar_t *);
//typedef wchar_t* (*MEDIAINFO_Inform)(void*, long);
//typedef wchar_t* (*MEDIAINFO_Get)(void*, int, int, wchar_t *, int, int);

class QMediaInfo
{
 public:
  QMediaInfo();
  bool IsLoaded() const;

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

  bool Open(const QString &filename);
  ~QMediaInfo();
 private:
  const wchar_t *Get(MediaInfo_stream_C streamKind, int streamNumber, wchar_t* parameter,
               MediaInfo_info_C infoKind, MediaInfo_info_C searchKind) const;
  int StreamCount(MediaInfo_stream_C stream) const;

  QLibrary *_lib;
  void *_pMedia;
};

#endif // QMEDIAINFO_H
