#include "VideoView.h"
#include "Logger.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"

VideoView::VideoView(bool bBasicMode, QWidget* parent) : QSplitter{Qt::Orientation::Horizontal, parent} {
  mBasicVideoView = new (std::nothrow) BasicVideoView{bBasicMode, this};

  const InteractiveVideoWidget& inst = *mBasicVideoView->GetVideoWidget();
  const QMediaPlaylist::PlaybackMode initPlaybackMode{inst.GetPlaybackMode()};
  mExtendedFunctionCtrlBar = inst.GetExtendedFunctionCtrlBar(this);

  mExtendLeftWidget = new (std::nothrow) QWidget{this};
  mExtendLeftWidget->setVisible(!bBasicMode);
  mExtendLeftLayout = new (std::nothrow) QVBoxLayout{mExtendLeftWidget};
  mExtendLeftLayout->addWidget(mBasicVideoView);
  mExtendLeftLayout->addWidget(mExtendedFunctionCtrlBar);
  mExtendLeftLayout->setContentsMargins(0, 0, 0, 0);

  mVideoList = new (std::nothrow) VideoTableView{this};
  mVideoList->setPlaybackMode(initPlaybackMode);

  addWidget(mExtendLeftWidget);
  addWidget(mVideoList);
  setContentsMargins(0, 0, 0, 0);

  setWindowIcon(QIcon{":/VideoPlayer/VIDEO_PLAYER"});
  setWindowTitle("Video Player");

  restoreState(Configuration().value("VIDEO_VIEW_STATE").toByteArray());
  subscribe();
}

VideoView::~VideoView() {
  if (this->parent() != nullptr) {
    Configuration().setValue("VIDEO_VIEW_STATE", saveState());
  }
}

void VideoView::onReqModeChange(bool bBasicMode) {
  mExtendedFunctionCtrlBar->setVisible(!bBasicMode);
  mVideoList->setVisible(!bBasicMode);
}

void VideoView::subscribe() {
  connect(mBasicVideoView, &BasicVideoView::reqFunctionModeChange, this, &VideoView::onReqModeChange);

  const InteractiveVideoWidget* inst = mBasicVideoView->GetVideoWidget();
  connect(inst->mPlayPrevAct, &QAction::triggered, mVideoList, &VideoTableView::PlayPreviousVideo);
  connect(inst->mPlayNextAct, &QAction::triggered, mVideoList, &VideoTableView::PlayNextVideo);
  connect(inst->mShowVideoList, &QAction::toggled, this, &VideoView::onChangeListVisibility);
  // InteractiveVideoWidget select folder, tableview&model filter medias, BasicVideoView play it
  connect(inst, &InteractiveVideoWidget::newFolderSelectedChangedByUser, this, &VideoView::PlayAPath);
  connect(mVideoList, &VideoTableView::reqPlayMedia, mBasicVideoView, &BasicVideoView::PlayAVideo);

  connect(inst, &InteractiveVideoWidget::playbackModeChanged, mVideoList, &VideoTableView::setPlaybackMode);
  connect(inst->mHideToolBarAct, &QAction::toggled, this, &VideoView::onChangeToolBarVisibility);
}

int VideoView::PlayAPath(const QString& folderPath, bool bPlayInstantly) {
  return mVideoList->setPlayPath(folderPath, bPlayInstantly);
}

int VideoView::PlayVideos(const QString& rootPath, const QStringList& mediafileNames, bool bPlayInstantly) {
  return mVideoList->setMediaFiles(rootPath, mediafileNames, bPlayInstantly);
}

void VideoView::StopPlay() {
  if (mBasicVideoView != nullptr) {
    mBasicVideoView->StopPlay();
  }
}

void VideoView::onChangeToolBarVisibility(bool bHide) {
  if (mExtendedFunctionCtrlBar->isHidden() != bHide) {
    mExtendedFunctionCtrlBar->setVisible(!bHide);
    mBasicVideoView->movePauseBtnToCenter();
  }
}

void VideoView::onChangeListVisibility(bool visibility) {
  if (mVideoList->isHidden() == visibility) {
    mVideoList->setVisible(visibility);
    mBasicVideoView->movePauseBtnToCenter();
  }
}
