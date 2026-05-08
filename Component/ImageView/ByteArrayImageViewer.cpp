#include "ByteArrayImageViewer.h"
#include "ImageTool.h"
#include "Logger.h"
#include <QBuffer>

bool ByteArrayImageViewer::setPixmapByByteArrayData(const QByteArray& dataByteArray, const QString& noDotFormat) {
  mDataFromArchive = dataByteArray;
  setFormatAndImgSizeBytes(noDotFormat, mDataFromArchive.size());
  return UpdatePixmapAndTitle();
}

QPixmap ByteArrayImageViewer::GetPixmapCore() const {
  return GetPixmapCoreStatic(mDataFromArchive, mNoDotFormat.toStdString().c_str());
}

std::unique_ptr<QMovie> ByteArrayImageViewer::GetMovieCore(QSize& movieSize) const {
  return GetMovieCoreStatic(mDataFromArchive, mNoDotFormat, movieSize);
}

QPixmap ByteArrayImageViewer::GetPixmapCoreStatic(const QByteArray& rawData, const char* noDotFormat) {
  QPixmap pixmap;
  if (!pixmap.loadFromData(rawData, noDotFormat)) {
    LOG_W("Image load from bytearray failed");
    return {};
  }
  return pixmap;
}

std::unique_ptr<QMovie> ByteArrayImageViewer::GetMovieCoreStatic(QByteArray& rawData, const QString& noDotFormat, QSize& movieSize) {
  std::unique_ptr<QBuffer> buffer{new (std::nothrow) QBuffer{&rawData}};
  if (!buffer || !buffer->open(QIODevice::ReadOnly)) {
    LOG_D("buffer[size:%d] open failed", rawData.size());
    return nullptr;
  }

  movieSize = ImageTool::GetImageDimensionPixel(buffer.get(), noDotFormat);
  buffer->seek(0);

  std::unique_ptr<QMovie> upMovie{new (std::nothrow) QMovie};
  if (!upMovie) {
    return nullptr;
  }
  upMovie->setFormat(noDotFormat.toUtf8());

  QBuffer* pTemp{buffer.release()};
  upMovie->setDevice(pTemp);
  pTemp->setParent(upMovie.get());

  return upMovie;
}
