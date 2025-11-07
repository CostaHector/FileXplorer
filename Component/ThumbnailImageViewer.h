#ifndef THUMBNAILIMAGEVIEWER_H
#define THUMBNAILIMAGEVIEWER_H

#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QFileInfo>
#include <QToolBar>
#include <QCheckBox>
#include "FolderNxtAndLastIterator.h"

class ThumbnailImageViewer : public QScrollArea {
  Q_OBJECT
 public:
  explicit ThumbnailImageViewer(const QString& memoryKeyName, QWidget* parent = nullptr);
  ~ThumbnailImageViewer();
  void wheelEvent(QWheelEvent* event) override;
  int GetCurImageSizeScale() const { return mCurIconScaledSizeIndex; }
  QPixmap pixmap() const { return mLabel->pixmap(Qt::ReturnByValue); }
  static bool IsFileAbsPathImage(const QString& fileAbsPath);
  static bool IsFileImage(const QFileInfo& fi);
  static bool IsGifFile(const QString& fileAbsPath);

  void adjustButtonPosition();

 signals:
  void onImageScaledIndexChanged(int newScaledIndex);

 public slots:
  void resizeEvent(QResizeEvent* event) override;
  void showEvent(QShowEvent* event) override;

  bool setPixmapByByteArrayData(const QByteArray& dataByteArray);
  bool setPixmapByAbsFilePath(const QString& parentPath, const QString& rel2Img);
  bool refreshPixmapSize();
  bool setIconSizeScaledIndex(int newScaledIndex);
  void clearPixmap();

  bool NavigateImageNext() { return NavigateImageCore(FolderNxtAndLastIterator::NaviDirection::NEXT); }
  bool NavigateImagePrevious() { return NavigateImageCore(FolderNxtAndLastIterator::NaviDirection::PREV); }

  bool NavigateIntoSubdirectoryChanged(bool bInclude);

 private:
  void ReadSetting();

  bool UpdatePixmapAndTitle();
  bool GetPixmap(QPixmap& pm, QString& winTitle) const;
  QMovie* GetMovie(QString& winTitle) const;

  bool NavigateImageCore(FolderNxtAndLastIterator::NaviDirection direction = FolderNxtAndLastIterator::NaviDirection::NEXT);

  int mCurIconScaledSizeIndex = 1;
  int mWidth = 404, mHeight = 250;
  const QString m_memoryKeyName;

  struct FromPath {
    QString parentPath;
    QString rel2image;
    qint64 imageBytes;
    bool operator==(const FromPath& rhs) const { return parentPath == rhs.parentPath && rel2image == rhs.rel2image && imageBytes == rhs.imageBytes; }

    QString GetImageAbsPath() const;
    void clear() {
      parentPath.clear();
      rel2image.clear();
      imageBytes = 0;
    }
  };

  enum class ImageFrom {
    ARCHIVE = 0,
    PATH = 1,
  };
  QByteArray mDataFromArchive;

  FromPath mDataFromPath;
  FolderNxtAndLastIterator mImgIt{FolderNxtAndLastIterator::GetInstsNaviImages(false)};
  ImageFrom mImageFrom{ImageFrom::PATH};

  QCheckBox* mNavigateIntoSub{nullptr};
  QToolBar* mControlToolBar{nullptr};
  QPushButton *m_prevButton{nullptr}, *m_nextButton{nullptr};
  QLabel* mLabel{nullptr};
  std::unique_ptr<QMovie> mPMovie;
};

#endif  // THUMBNAILIMAGEVIEWER_H
