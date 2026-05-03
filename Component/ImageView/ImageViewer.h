#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QScrollArea>
#include <QLabel>
#include <QMovie>
#include <memory>

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
  const QLabel* GetLabel() const { return mLabel; }

private:
  void subscribe();
  void ReadSetting();
  QSize getDestSize(QSize sz) const;

  QString GetResolutionAndSizeInWinTitle(const QSize& oldSz, const QSize& dstSz) const;
  virtual QString GetPathInfoInWinTitle() const { return ""; }
  virtual QPixmap GetPixmapCore() const = 0;
  virtual bool isCurImageGif() const = 0;

  virtual std::unique_ptr<QMovie> GetMovieCore(QSize& movieSize) const = 0;
  virtual qint64 GetImageFileSize() const = 0;

  int mCurIconScaledSizeIndex = 1;
  int mWidth = 404, mHeight = 250;
  const QString m_memoryKeyName;

  QLabel* mLabel{nullptr};

  std::unique_ptr<QMovie> mPMovie;
};

#endif // IMAGEVIEWER_H
