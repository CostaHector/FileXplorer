#ifndef BYTEARRAYIMAGEVIEWER_H
#define BYTEARRAYIMAGEVIEWER_H

#include "ImageViewer.h"

class ByteArrayImageViewer : public ImageViewer {
public:
  using ImageViewer::ImageViewer;

public slots:
  bool setPixmapByByteArrayData(const QByteArray& dataByteArray, const QString& formatStr);

private:
  QPixmap GetPixmapCore() const override;
  bool isCurImageGif() const override;
  std::unique_ptr<QMovie> GetMovieCore(QSize& movieSize) const override;
  qint64 GetImageFileSize() const override { return mDataFromArchive.size(); }

  mutable QByteArray mDataFromArchive;
  QString mFormatStr;
};

#endif // BYTEARRAYIMAGEVIEWER_H
