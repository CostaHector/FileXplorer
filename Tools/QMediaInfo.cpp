#include "QMediaInfo.h"
#include <QFileInfo>
#include "PublicVariable.h"
#ifdef WIN32
#define QString2MediaInfoc_str(qstr) (qstr).toStdWString().c_str()
#define QStringFromMediaInfoc_str(cStr) QString::fromStdWString(cStr)
#define InitMediaInfo_CharArr(arrayName, initialLiteralString) MediaInfo_Char arrayName[] = L"" initialLiteralString
#else
#define QString2MediaInfoc_str(qstr) (qstr).toStdString().c_str()
#define QStringFromMediaInfoc_str(cStr) QString::fromStdString(cStr)
#define InitMediaInfo_CharArr(arrayName, initialLiteralString) MediaInfo_Char arrayName[] = initialLiteralString
#endif

QMediaInfo::QMediaInfo() {
#ifdef _WIN32
  QString libPath = PreferenceSettings().value(MemoryKey::WIN32_MEDIAINFO_LIB_PATH.name).toString();
#else
  QString libPath = PreferenceSettings().value(MemoryKey::LINUX_MEDIAINFO_LIB_PATH.name).toString();
#endif
  qDebug("%s", qPrintable(QFileInfo(libPath).absoluteFilePath()));
  _lib = new QLibrary(libPath);
  if (!_lib->load()) {
    qDebug("load lib failed: %s", qPrintable(_lib->errorString()));
    return;
  }
  qDebug("load[%s] success", qPrintable(_lib->fileName()));
  MEDIAINFO_New v = (MEDIAINFO_New)_lib->resolve("MediaInfo_New");
  _pMedia = v();
}

bool QMediaInfo::Open(const QString& filename) {
  if (!IsLoaded())
    return false;
  MEDIAINFO_Open mopen = (MEDIAINFO_Open)_lib->resolve("MediaInfo_Open");
  if ((mopen(_pMedia, QString2MediaInfoc_str(filename))) == 0)
    return false;
  return true;
}

QString QMediaInfo::Inform() const {
  MEDIAINFO_Inform inf = (MEDIAINFO_Inform)_lib->resolve("MediaInfo_Inform");
  if (!inf)
    return NULL;
  return QStringFromMediaInfoc_str(inf(_pMedia, 0));
}

const MediaInfo_Char* QMediaInfo::Get(MediaInfo_stream_C streamKind,
                                      int streamNumber,
                                      MediaInfo_Char* parameter,
                                      MediaInfo_info_C infoKind,
                                      MediaInfo_info_C searchKind) const {
  MEDIAINFO_Get get = (MEDIAINFO_Get)_lib->resolve("MediaInfo_Get");
  if (!get)
    return NULL;
  return get(_pMedia, streamKind, streamNumber, parameter, infoKind, searchKind);
}

QList<int> QMediaInfo::batchVidsDurationLength(const QStringList& vidsAbsPath) const {
  if (vidsAbsPath.isEmpty()) {
    qDebug("Input vids abs path list is empty");
    return {};
  }
  if (!IsLoaded()) {
    qWarning("_lib not loaded");
    return {};
  }
  MEDIAINFO_Get get = (MEDIAINFO_Get)_lib->resolve("MediaInfo_Get");
  if (!get) {
    qWarning("function named MediaInfo_Get not exist in lib");
    return {};
  }

  QList<int> durationList;
  durationList.reserve(vidsAbsPath.size());

  MEDIAINFO_Open mopen = (MEDIAINFO_Open)_lib->resolve("MediaInfo_Open");
  InitMediaInfo_CharArr(prop, "Duration/String3");
  for (const QString& filename : vidsAbsPath) {
    if (mopen(_pMedia, QString2MediaInfoc_str(filename)) == 0) {
      durationList.append(0);
    }
    const QString& dur = QStringFromMediaInfoc_str(get(_pMedia, MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
    int durInt = QTime::fromString(dur, Qt::ISODateWithMs).msecsSinceStartOfDay();
    durationList.append(durInt);
  }
  return durationList;
}

QString QMediaInfo::Duration() const {
  InitMediaInfo_CharArr(prop, "Duration/String3");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

int QMediaInfo::DurationLength() const {
  const QString& dur = Duration();
  return QTime::fromString(dur, Qt::ISODateWithMs).msecsSinceStartOfDay();
}

QString QMediaInfo::VideoBitRate() const {
  InitMediaInfo_CharArr(prop, "BitRate/String");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_Video, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::AudioBitRate() const {
  InitMediaInfo_CharArr(prop, "BitRate/String");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_Audio, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::VideoResolution() const {
  InitMediaInfo_CharArr(propH, "Height");
  InitMediaInfo_CharArr(propW, "Width");
  QString sW = QStringFromMediaInfoc_str(Get(MediaInfo_Stream_Video, 0, propW, MediaInfo_Info_Text, MediaInfo_Info_Name));
  QString sH = QStringFromMediaInfoc_str(Get(MediaInfo_Stream_Video, 0, propH, MediaInfo_Info_Text, MediaInfo_Info_Name));
  if (sW.isEmpty() || sH.isEmpty())
    return "";
  return sW + "x" + sH;
}

QString QMediaInfo::VideoFrameRate() const {
  InitMediaInfo_CharArr(prop, "FrameRate/String");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_Video, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::VideoCodec() const {
  InitMediaInfo_CharArr(prop, "InternetMediaType");
  InitMediaInfo_CharArr(propF, "Format");
  QString codec;
  codec = QStringFromMediaInfoc_str(Get(MediaInfo_Stream_Video, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
  if (codec.isEmpty())
    return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_Video, 0, propF, MediaInfo_Info_Text, MediaInfo_Info_Name));
  return codec;
}

QString QMediaInfo::AudioCodec() const {
  InitMediaInfo_CharArr(prop, "InternetMediaType");
  InitMediaInfo_CharArr(propF, "Format");
  QString codec;
  codec = QStringFromMediaInfoc_str(Get(MediaInfo_Stream_Audio, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
  if (codec.isEmpty())
    return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_Audio, 0, propF, MediaInfo_Info_Text, MediaInfo_Info_Name));
  return codec;
}

int QMediaInfo::StreamCount(MediaInfo_stream_C streamKind) const {
  InitMediaInfo_CharArr(prop, "StreamCount");
  return QStringFromMediaInfoc_str(Get(streamKind, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name)).toInt();
}

QString QMediaInfo::AudioLanguages() const {
  InitMediaInfo_CharArr(prop, "Language");
  QString lang;

  int cs = StreamCount(MediaInfo_Stream_Audio);
  for (int i = 0; i < cs; i++) {
    if (i != 0)
      lang += "/";
    lang += QStringFromMediaInfoc_str(Get(MediaInfo_Stream_Audio, i, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
  }
  if (lang.isEmpty())
    return QString::number(cs);
  return lang;
}

QString QMediaInfo::SubtitleLanguages() const {
  InitMediaInfo_CharArr(prop, "Language");
  QString lang;

  int cs = StreamCount(MediaInfo_Stream_Text);
  for (int i = 0; i < cs; i++) {
    if (i != 0)
      lang += "/";
    lang += QStringFromMediaInfoc_str(Get(MediaInfo_Stream_Text, i, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
  }
  if (lang.isEmpty())
    return QString::number(cs);
  return lang;
}

QString QMediaInfo::Title() const {
  InitMediaInfo_CharArr(prop, "Title");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::TitleMore() const {
  InitMediaInfo_CharArr(prop, "Title/More");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::Domain() const {
  InitMediaInfo_CharArr(prop, "Domain");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::Collection() const {
  InitMediaInfo_CharArr(prop, "Collection");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::Season() const {
  InitMediaInfo_CharArr(prop, "Season");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::Movie() const {
  InitMediaInfo_CharArr(prop, "Movie");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::Description() const {
  InitMediaInfo_CharArr(prop, "Movie");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::FileSize() const {
  InitMediaInfo_CharArr(prop, "FileSize/String");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::FormatExt() const {
  InitMediaInfo_CharArr(prop, "Format/Extensions");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::CompleteName() const {
  InitMediaInfo_CharArr(prop, "CompleteName");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::Filename() const {
  InitMediaInfo_CharArr(prop, "FileName");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::FileExtension() const {
  InitMediaInfo_CharArr(prop, "FileExtension");
  return QStringFromMediaInfoc_str(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

bool QMediaInfo::IsLoaded() const {
  return (_lib->isLoaded());
}

QMediaInfo::~QMediaInfo() {
  MEDIAINFO_Delete d = (MEDIAINFO_Delete)_lib->resolve("MediaInfo_Delete");
  d(_pMedia);
  _lib->unload();
}
