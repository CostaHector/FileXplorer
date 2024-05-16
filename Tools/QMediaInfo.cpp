#include "QMediaInfo.h"
#include <QFileInfo>
#include "PublicVariable.h"

QMediaInfo::QMediaInfo() {
#ifdef _WIN32
  QString libPath = PreferenceSettings().value(MemoryKey::WIN32_MEDIAINFO_LIB_PATH.name).toString();
#else
  QString libPath = PreferenceSettings().value(MemoryKey::LINUX_MEDIAINFO_LIB_PATH.name).toString();
#endif
  qDebug("%s", qPrintable(QFileInfo(libPath).absoluteFilePath()));
  if (!QLibrary::isLibrary(libPath)) {
    qDebug("%s is not a library", qPrintable(libPath));
    return;
  }
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
  if ((mopen(_pMedia, filename.toStdWString().c_str())) == 0)
    return false;
  return true;
}

QString QMediaInfo::Inform() const {
  MEDIAINFO_Inform inf = (MEDIAINFO_Inform)_lib->resolve("MediaInfo_Inform");
  if (!inf)
    return NULL;
  return QString::fromStdWString(inf(_pMedia, 0));
}

const wchar_t* QMediaInfo::Get(MediaInfo_stream_C streamKind,
                               int streamNumber,
                               wchar_t* parameter,
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
  wchar_t prop[] = L"Duration/String3";
  for (const QString& filename : vidsAbsPath) {
    if (mopen(_pMedia, filename.toStdWString().c_str()) == 0) {
      durationList.append(0);
    }
    const QString& dur = QString::fromStdWString(get(_pMedia, MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
    int durInt = QTime::fromString(dur, Qt::ISODateWithMs).msecsSinceStartOfDay();
    durationList.append(durInt);
  }
  return durationList;
}

QString QMediaInfo::Duration() const {
  wchar_t prop[] = L"Duration/String3";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

int QMediaInfo::DurationLength() const {
  const QString& dur = Duration();
  return QTime::fromString(dur, Qt::ISODateWithMs).msecsSinceStartOfDay();
}

QString QMediaInfo::VideoBitRate() const {
  wchar_t prop[] = L"BitRate/String";
  return QString::fromStdWString(Get(MediaInfo_Stream_Video, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::AudioBitRate() const {
  wchar_t prop[] = L"BitRate/String";
  return QString::fromStdWString(Get(MediaInfo_Stream_Audio, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::VideoResolution() const {
  wchar_t propH[] = L"Height";
  wchar_t propW[] = L"Width";
  QString sW = QString::fromStdWString(Get(MediaInfo_Stream_Video, 0, propW, MediaInfo_Info_Text, MediaInfo_Info_Name));
  QString sH = QString::fromStdWString(Get(MediaInfo_Stream_Video, 0, propH, MediaInfo_Info_Text, MediaInfo_Info_Name));
  if (sW.isEmpty() || sH.isEmpty())
    return "";
  return sW + "x" + sH;
}

QString QMediaInfo::VideoFrameRate() const {
  wchar_t prop[] = L"FrameRate/String";
  return QString::fromStdWString(Get(MediaInfo_Stream_Video, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::VideoCodec() const {
  wchar_t prop[] = L"InternetMediaType";
  wchar_t propF[] = L"Format";
  QString codec;
  codec = QString::fromStdWString(Get(MediaInfo_Stream_Video, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
  if (codec.isEmpty())
    return QString::fromStdWString(Get(MediaInfo_Stream_Video, 0, propF, MediaInfo_Info_Text, MediaInfo_Info_Name));
  return codec;
}

QString QMediaInfo::AudioCodec() const {
  wchar_t prop[] = L"InternetMediaType";
  wchar_t propF[] = L"Format";
  QString codec;
  codec = QString::fromStdWString(Get(MediaInfo_Stream_Audio, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
  if (codec.isEmpty())
    return QString::fromStdWString(Get(MediaInfo_Stream_Audio, 0, propF, MediaInfo_Info_Text, MediaInfo_Info_Name));
  return codec;
}

int QMediaInfo::StreamCount(MediaInfo_stream_C streamKind) const {
  wchar_t prop[] = L"StreamCount";
  return QString::fromStdWString(Get(streamKind, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name)).toInt();
}

QString QMediaInfo::AudioLanguages() const {
  wchar_t prop[] = L"Language";
  QString lang;

  int cs = StreamCount(MediaInfo_Stream_Audio);
  for (int i = 0; i < cs; i++) {
    if (i != 0)
      lang += "/";
    lang += QString::fromStdWString(Get(MediaInfo_Stream_Audio, i, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
  }
  if (lang.isEmpty())
    return QString::number(cs);
  return lang;
}

QString QMediaInfo::SubtitleLanguages() const {
  wchar_t prop[] = L"Language";
  QString lang;

  int cs = StreamCount(MediaInfo_Stream_Text);
  for (int i = 0; i < cs; i++) {
    if (i != 0)
      lang += "/";
    lang += QString::fromStdWString(Get(MediaInfo_Stream_Text, i, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
  }
  if (lang.isEmpty())
    return QString::number(cs);
  return lang;
}

QString QMediaInfo::Title() const {
  wchar_t prop[] = L"Title";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::TitleMore() const {
  wchar_t prop[] = L"Title/More";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::Domain() const {
  wchar_t prop[] = L"Domain";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::Collection() const {
  wchar_t prop[] = L"Collection";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::Season() const {
  wchar_t prop[] = L"Season";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::Movie() const {
  wchar_t prop[] = L"Movie";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::Description() const {
  wchar_t prop[] = L"Movie";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::FileSize() const {
  wchar_t prop[] = L"FileSize/String";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::FormatExt() const {
  wchar_t prop[] = L"Format/Extensions";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::CompleteName() const {
  wchar_t prop[] = L"CompleteName";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::Filename() const {
  wchar_t prop[] = L"FileName";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

QString QMediaInfo::FileExtension() const {
  wchar_t prop[] = L"FileExtension";
  return QString::fromStdWString(Get(MediaInfo_Stream_General, 0, prop, MediaInfo_Info_Text, MediaInfo_Info_Name));
}

bool QMediaInfo::IsLoaded() const {
  return (_lib->isLoaded());
}

QMediaInfo::~QMediaInfo() {
  MEDIAINFO_Delete d = (MEDIAINFO_Delete)_lib->resolve("MediaInfo_Delete");
  d(_pMedia);
  _lib->unload();
}
