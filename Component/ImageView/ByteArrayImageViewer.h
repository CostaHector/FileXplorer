#ifndef BYTEARRAYIMAGEVIEWER_H
#define BYTEARRAYIMAGEVIEWER_H

#include "ImageViewer.h"

class ByteArrayImageViewer : public ImageViewer {
public:
  using ImageViewer::ImageViewer;

public slots:
  bool setPixmapByByteArrayData(const QByteArray& dataByteArray, const QString& noDotFormat);

private:
  QPixmap GetPixmapCore() const override;
  std::unique_ptr<QMovie> GetMovieCore(QSize& movieSize) const override;

  mutable QByteArray mDataFromArchive;
};

#endif // BYTEARRAYIMAGEVIEWER_H
