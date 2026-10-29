#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QScrollArea>
#include <QMovie>
#include <memory>
#include "SelectableLabel.h"

class QFileInfo;

class ImageViewer : public QScrollArea {
  Q_OBJECT
public:
  static bool IsFileImage(const QFileInfo& fi);

  explicit ImageViewer(const QString& memoryKeyName, QWidget* parent = nullptr);
  ~ImageViewer();

  int GetCurImageSizeScale() const { return mCurIconScaledSizeIndex; }
  QString GetName() const { return m_memoryKeyName; }

signals:
  void onImageScaledIndexChanged(int newScaledIndex);
  void reqPreviousImage();
  void reqNextImage();

public slots:
  bool refreshPixmapSize() { return UpdatePixmapAndTitle(); }
  bool setIconSizeScaledIndex(int newScaledIndex);
  void clearPixmap() { mLabel->clear(); }

protected:
  void showEvent(QShowEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

  bool UpdatePixmapAndTitle();
  QPixmap GetPixmap(QString& winTitle) const;
  std::unique_ptr<QMovie> GetMovie(QString& winTitle) const;
  const SelectableLabel* GetLabel() const { return mLabel; }

  void setFormatAndImgSizeBytes(const QString& noDotFormat, const qint64& imgSizeBytes) {
    mNoDotFormat = noDotFormat;
    mImgSizeBytes = imgSizeBytes;
  }
  QString mNoDotFormat; // file extension without prefix dot
  qint64 mImgSizeBytes{0};

  QToolBar* mNaviToolBar{nullptr};
  QAction* m_selectARect{nullptr};

private:
  void ReadSetting();
  bool isScaledToWidth(const QSize& sz) const;
  QSize getDestSize(const QSize& sz) const;
  bool onDirectionKey(int directionKey, QScrollBar* scrollBar);

  QString GetResolutionAndSizeInWinTitle(const QSize& oldSz, const QSize& dstSz) const;
  virtual QString GetPathInfoInWinTitle() const { return ""; }
  virtual QPixmap GetPixmapCore() const = 0;

  virtual std::unique_ptr<QMovie> GetMovieCore(QSize& movieSize) const = 0;

  bool isCurImageGif() const;
  qint64 GetImageFileSize() const { return mImgSizeBytes; }

  int mCurIconScaledSizeIndex = 1;
  int mWidth = 404, mHeight = 250;
  const QString m_memoryKeyName;

  SelectableLabel* mLabel{nullptr};
  std::unique_ptr<QMovie> mPMovie;
};

#endif // IMAGEVIEWER_H
