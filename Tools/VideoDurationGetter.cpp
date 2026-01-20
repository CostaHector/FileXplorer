#include "VideoDurationGetter.h"
#include "Logger.h"

extern "C" {
#include <libavformat/avformat.h>
}

void IsFFmpegInstalledOK() {
  avformat_network_init();
  LOG_W("FFmpeg version:%u", avformat_version());
}

constexpr int VideoDurationGetter::MILLISECONDS_PER_SECOND;
constexpr int VideoDurationGetter::MICROSECONDS_PER_MILLISECOND;

int VideoDurationGetter::ReadAVideo(const QString& vidPath) {
  AVFormatContext* fmtCtx = nullptr;
  // open local video file(disable netword function)
  if (avformat_open_input(&fmtCtx, vidPath.toUtf8().constData(), nullptr, nullptr) != 0) {
    return -1;  // open failed
  }
  // quick parse stream information
  if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
    avformat_close_input(&fmtCtx);
    return -1;
  }
  // fmtCtx->duration unit is us
  int duration = (fmtCtx->duration == AV_NOPTS_VALUE) ? -1 : (double)fmtCtx->duration / MICROSECONDS_PER_MILLISECOND;
  avformat_close_input(&fmtCtx);
  return duration;
}

QList<int> VideoDurationGetter::ReadVideos(const QStringList& vidsPath) {
  QList<int> durations;
  AVFormatContext* fmtCtx = avformat_alloc_context();
  for (const QString& path : vidsPath) {
    if (avformat_open_input(&fmtCtx, path.toUtf8().constData(), nullptr, nullptr) != 0) {
      durations.push_back(-1);
      continue;
    }
    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
      durations.push_back(-1);
      avformat_close_input(&fmtCtx);
      continue;
    }
    int duration = (fmtCtx->duration == AV_NOPTS_VALUE) ? -1 : (double)fmtCtx->duration / MICROSECONDS_PER_MILLISECOND;
    durations.push_back(duration);
    avformat_close_input(&fmtCtx);
  }
  avformat_free_context(fmtCtx);
  return durations;
}

bool VideoDurationGetter::StartToGet() {
#ifdef _WIN32
  if (!mi.StartToGet()) {
    LOG_W("Video duration getter is nullptr, cannot get video duration");
    return false;
  }
#endif
  // for linux there is not need to set start to get
  return true;
}

int VideoDurationGetter::GetLengthQuickStatic(const VideoDurationGetter& self, const QString& vidPath) {
  return self.GetLengthQuick(vidPath);
}

QList<int> VideoDurationGetter::GetLengthsQuickStatic(const VideoDurationGetter& self, const QStringList& vidsPath) {
  return self.GetLengthsQuick(vidsPath);
}

int VideoDurationGetter::GetLengthQuick(const QString& vidPath) const {
#ifdef _WIN32
  return mi.VidDurationLengthQuick(vidPath);
#endif
  return ReadAVideo(vidPath);
}

QList<int> VideoDurationGetter::GetLengthsQuick(const QStringList& vidsPath) const {
#ifdef _WIN32
  return mi.batchVidsDurationLength(vidsPath);
#endif
  return ReadVideos(vidsPath);
}
