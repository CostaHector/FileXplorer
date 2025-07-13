#include "VideoDurationGetter.h"

extern "C" {
#include <libavformat/avformat.h>
}

double VideoDurationGetter::ReadAVideo(const QString& vidPath) {
  AVFormatContext* fmtCtx = nullptr;

  // 打开本地视频文件（禁用网络功能）
  if (avformat_open_input(&fmtCtx, vidPath.toUtf8().constData(), nullptr, nullptr) != 0) {
    return -1;  // 打开失败
  }

  // 快速解析流信息
  if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
    avformat_close_input(&fmtCtx);
    return -1;
  }

  // 计算时长（秒）
  double duration = (fmtCtx->duration == AV_NOPTS_VALUE) ? -1 : (double)fmtCtx->duration / AV_TIME_BASE;

  avformat_close_input(&fmtCtx);
  return duration;
}

QList<double> VideoDurationGetter::ReadVideos(const QStringList& vidsPath) {
  QList<double> durations;
  AVFormatContext* fmtCtx = avformat_alloc_context();

  for (const QString& path : vidsPath) {
    if (avformat_open_input(&fmtCtx, path.toUtf8().constData(), nullptr, nullptr) != 0) {
      durations << -1;
      continue;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
      durations << -1;
      avformat_close_input(&fmtCtx);
      continue;
    }

    double duration = (fmtCtx->duration == AV_NOPTS_VALUE) ? -1 : (double)fmtCtx->duration / AV_TIME_BASE;
    durations << duration;

    avformat_close_input(&fmtCtx);
  }

  avformat_free_context(fmtCtx);
  return durations;
}
