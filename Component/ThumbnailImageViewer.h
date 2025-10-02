#ifndef THUMBNAILIMAGEVIEWER_H
#define THUMBNAILIMAGEVIEWER_H

#include <QLabel>
class ThumbnailImageViewer : public QLabel {
  Q_OBJECT
 public:
  explicit ThumbnailImageViewer(const QString& momoryKeyName, QWidget* parent = nullptr);
  ~ThumbnailImageViewer();
  void wheelEvent(QWheelEvent* event) override;
  int GetCurImageSizeScale() const {return mCurIconScaledSizeIndex;}
 signals:
  void onImageScaledIndexChanged(int newScaledIndex);
 public slots:
  void refreshPixmapSize();
  void setPixmapByByteArrayData(const QByteArray& dataByteArray);
  bool setIconSizeScaledIndex(int newScaledIndex);
  void clearPixmap();
 private:
  int mCurIconScaledSizeIndex = 1;
  int mWidth = 404, mHeight = 250;
  QByteArray m_curDataByteArray;
  const QString m_memoryKeyName;
};

#endif  // THUMBNAILIMAGEVIEWER_H
