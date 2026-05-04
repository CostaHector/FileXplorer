#ifndef THUMBNAILIMAGEVIEWER_H
#define THUMBNAILIMAGEVIEWER_H

#include <QToolBar>
#include <QAction>

#include "FolderNxtAndLastIterator.h"
#include "ImageViewer.h"

class QMenu;

class ThumbnailImageViewer : public ImageViewer {
  Q_OBJECT
public:
  explicit ThumbnailImageViewer(const QString& memoryKeyName, QWidget* parent = nullptr);

public slots:
  bool setPixmapByAbsFilePath(const QString& parentPath, const QString& rel2Img);

  bool NavigateImageNext() { return NavigateImageCore(FolderNxtAndLastIterator::NaviDirection::NEXT); }
  bool NavigateImagePrevious() { return NavigateImageCore(FolderNxtAndLastIterator::NaviDirection::PREV); }
  void NavigateIntoSubdirectoryChanged(bool bInclude);

  void onCustomContextMenuRequested(const QPoint& pos);

private:
  void subscribe();

  QPixmap GetPixmapCore() const override;
  std::unique_ptr<QMovie> GetMovieCore(QSize& movieSize) const override;
  QString GetPathInfoInWinTitle() const override;

  bool NavigateImageCore(FolderNxtAndLastIterator::NaviDirection direction = FolderNxtAndLastIterator::NaviDirection::NEXT);
  QString GetImageAbsPath() const;

  QString mParentPath;
  QString mRel2Image;
  FolderNxtAndLastIterator mImgIt{FolderNxtAndLastIterator::GetInstsNaviImages(false)};

  QToolBar* mNaviToolBar{nullptr};
  QAction* mNavigateIntoSub{nullptr};
  QAction *m_prevButton{nullptr}, *m_nextButton{nullptr};

  QAction *_OPEN_IN_SYSTEM_APPLICATION{nullptr}, *_REVEAL_IN_FILE_EXPLORER{nullptr}, *_COPY_FILE_NAME{nullptr};
  QMenu* mMenu{nullptr};
};

#endif // THUMBNAILIMAGEVIEWER_H
