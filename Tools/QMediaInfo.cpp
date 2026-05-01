#include "QMediaInfo.h"
#include "MediaInfoDLL.h"
#include "Logger.h"
#include <QTime>
#include <QFile>

inline MediaInfoDLL::String ToMediaInfoString(const QString& qstr) {
#if defined(UNICODE) || defined(_UNICODE)
  return qstr.toStdWString();
#else
  return qstr.toStdString();
#endif
}

inline QString fromMediaInfoString(const MediaInfoDLL::String& mStr) {
#if defined(UNICODE) || defined(_UNICODE)
  return QString::fromStdWString(mStr);
#else
  return QString::fromStdString(mStr);
#endif
}
constexpr const char* LIBRARY_NAME{
#ifdef Q_OS_WINDOWS
    "MediaInfo.dll"
#elif defined(Q_OS_LINUX)
    "libmediainfo.so"
#elif defined(Q_OS_MACOS)
    "libmediainfo.dylib"
#else
    "mediainfo"
#endif
};

#include <QLibrary>
QMediaInfo::QMediaInfo()
  : m_mediaInfo{new MediaInfoDLL::MediaInfo}
  , m_bLoadDllResult{QLibrary{LIBRARY_NAME}.load()} {
  if (m_bLoadDllResult) {
    return;
  }
  LOG_E("libname[%s] load failed. isExist[%d], isLib[%d]", //
        LIBRARY_NAME,                                      //
        QFile::exists(LIBRARY_NAME),
        QLibrary::isLibrary(LIBRARY_NAME));
}

QMediaInfo& QMediaInfo::GetInst() {
  static QMediaInfo inst;
  return inst;
}

bool QMediaInfo::Open(const QString& filename) {
  if (!isLoadDllOk()) {
    return false;
  }
  return m_mediaInfo->Open(ToMediaInfoString(filename)) != 0;
}

int QMediaInfo::DurationLengthQuick(const QString& fileAbsPath) {
  if (!Open(fileAbsPath)) {
    return -1;
  }

  using namespace MediaInfoDLL;
  String mediaInfoDur = m_mediaInfo->Get(MediaInfoDLL::stream_t::Stream_General,
                                         0,                       //
                                         __T("Duration/String3"), //
                                         info_t::Info_Text,
                                         info_t::Info_Name);
  const QString& qStringDur = fromMediaInfoString(mediaInfoDur);
  return QTime::fromString(qStringDur, Qt::ISODateWithMs).msecsSinceStartOfDay();
}

QList<int> QMediaInfo::batchVidsDurationLength(const QStringList& vidsAbsPath) {
  if (vidsAbsPath.isEmpty()) {
    LOG_D("Input vids abs path list is empty");
    return {};
  }

  QList<int> durationList;
  durationList.reserve(vidsAbsPath.size());
  for (const QString& filename : vidsAbsPath) {
    durationList.push_back(DurationLengthQuick(filename));
  }
  return durationList;
}
