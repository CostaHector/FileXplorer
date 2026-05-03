#include "ByteArrayImageViewer.h"
#include "ImageTool.h"
#include "Logger.h"
#include <QBuffer>
#include <QImageReader>

bool ByteArrayImageViewer::setPixmapByByteArrayData(const QByteArray& dataByteArray, const QString& formatStr) {
  mDataFromArchive = dataByteArray;
  mFormatStr = formatStr;
  return UpdatePixmapAndTitle();
}

QPixmap ByteArrayImageViewer::GetPixmapCore() const {
  QPixmap pixmap;
  if (!pixmap.loadFromData(mDataFromArchive, mFormatStr.toStdString().c_str())) {
    LOG_W("Image load from bytearray failed");
    return {};
  }
  return pixmap;
}

bool ByteArrayImageViewer::isCurImageGif() const {
  return ImageTool::IsGifFile('.' + mFormatStr);
}

std::unique_ptr<QMovie> ByteArrayImageViewer::GetMovieCore(QSize& movieSize) const {
  std::unique_ptr<QBuffer> buffer{new (std::nothrow) QBuffer{&mDataFromArchive}};
  if (!buffer || !buffer->open(QIODevice::ReadOnly)) {
    LOG_D("buffer[size:%d] open failed", mDataFromArchive.size());
    return nullptr;
  }

  QImageReader imgReader{buffer.get(), mFormatStr.toUtf8()};
  movieSize = imgReader.size();
  buffer->seek(0);

  std::unique_ptr<QMovie> upMovie{new (std::nothrow) QMovie};
  if (!upMovie) {
    return nullptr;
  }
  upMovie->setFormat(mFormatStr.toUtf8());

  QBuffer* pTemp{buffer.release()};
  upMovie->setDevice(pTemp);
  pTemp->setParent(upMovie.get());

  return upMovie;
}
