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
  QPixmap pixmap;
  if (!pixmap.loadFromData(mDataFromArchive, mNoDotFormat.toStdString().c_str())) {
    LOG_W("Image load from bytearray failed");
    return {};
  }
  return pixmap;
}

std::unique_ptr<QMovie> ByteArrayImageViewer::GetMovieCore(QSize& movieSize) const {
  std::unique_ptr<QBuffer> buffer{new (std::nothrow) QBuffer{&mDataFromArchive}};
  if (!buffer || !buffer->open(QIODevice::ReadOnly)) {
    LOG_D("buffer[size:%d] open failed", mDataFromArchive.size());
    return nullptr;
  }

  movieSize = ImageTool::GetImageDimensionPixel(buffer.get(), mNoDotFormat);
  buffer->seek(0);

  std::unique_ptr<QMovie> upMovie{new (std::nothrow) QMovie};
  if (!upMovie) {
    return nullptr;
  }
  upMovie->setFormat(mNoDotFormat.toUtf8());

  QBuffer* pTemp{buffer.release()};
  upMovie->setDevice(pTemp);
  pTemp->setParent(upMovie.get());

  return upMovie;
}
