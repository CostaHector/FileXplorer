#include "VideoView.h"
#include "Logger.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include <QFileDialog>

VideoView::VideoView(bool bBasicMode, QWidget* parent)
  : QSplitter{Qt::Orientation::Horizontal, parent} {
  mBasicVideoView = new (std::nothrow) BasicVideoView{false, this};

  const InteractiveVideoWidget& inst = *mBasicVideoView->GetVideoWidget();
  MenuToolButton* playbackModeBtn = inst.GetPlaybackModelMenuToolButton(this);

  mExtendedFunctionCtrlBar = new (std::nothrow) ToolBarWidget{QBoxLayout::Direction::LeftToRight, this};
  mExtendedFunctionCtrlBar->setVisible(!bBasicMode);
  mExtendedFunctionCtrlBar->addAction(inst.mSelectVideoFolder);
  mExtendedFunctionCtrlBar->addSeparator();
  mExtendedFunctionCtrlBar->addAction(inst.mSeekBackwardHotAct);
  mExtendedFunctionCtrlBar->addAction(inst.mSeekForwardHotAct);
  mExtendedFunctionCtrlBar->addSeparator();
  mExtendedFunctionCtrlBar->addAction(inst.mPlayPrevAct);
  mExtendedFunctionCtrlBar->addAction(inst.mPlayNextAct);
  mExtendedFunctionCtrlBar->addSeparator();
  mExtendedFunctionCtrlBar->addWidget(playbackModeBtn);
  mExtendedFunctionCtrlBar->addAction(inst.mShowFrames);
  mExtendedFunctionCtrlBar->addStretch();
  mExtendedFunctionCtrlBar->addAction(inst.mShowVideoList);

  mExtendLeftWidget = new (std::nothrow) QWidget{this};
  mExtendLeftWidget->setVisible(!bBasicMode);
  mExtendLeftLayout = new (std::nothrow) QVBoxLayout{mExtendLeftWidget};
  mExtendLeftLayout->addWidget(mBasicVideoView);
  mExtendLeftLayout->addWidget(mExtendedFunctionCtrlBar);
  mExtendLeftLayout->setContentsMargins(0, 0, 0, 0);

  mVideoList = new (std::nothrow) VideoTableView{this};
  mVideoList->setPlaybackMode(inst.GetPlaybackMode());

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
  connect(mVideoList, &VideoTableView::reqPlayMedia, mBasicVideoView, &BasicVideoView::PlayAVideo);

  const InteractiveVideoWidget* inst = mBasicVideoView->GetVideoWidget();
  connect(inst->mPlayPrevAct, &QAction::triggered, mVideoList, &VideoTableView::PlayPreviousVideo);
  connect(inst->mPlayNextAct, &QAction::triggered, mVideoList, &VideoTableView::PlayNextVideo);
  connect(inst->mShowVideoList, &QAction::toggled, this, &VideoView::onChangeListVisibility);

  connect(inst, &InteractiveVideoWidget::playbackModeChanged, mVideoList, &VideoTableView::setPlaybackMode);  
  connect(inst->mHideToolBarAct, &QAction::toggled, this, &VideoView::onChangeToolBarVisibility);
}

int VideoView::PlayAPath(const QString& folderPath) {
  return mVideoList->setPlayPath(folderPath);
}

int VideoView::PlayVideos(const QString& rootPath, const QStringList& mediafileNames) {
  return mVideoList->setMediaFiles(rootPath, mediafileNames);
}

int VideoView::onSelectAFolder() {
  QString defaultOpenPathLocatedIn
      = Configuration().value(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.v).toString();
  if (!QFile::exists(defaultOpenPathLocatedIn)) {
    defaultOpenPathLocatedIn = SystemPath::HOME_PATH();
  }
  const QString dirSelected = QFileDialog::getExistingDirectory(this,
                                                                "Select a media folder", //
                                                                defaultOpenPathLocatedIn);
  if (dirSelected.isEmpty()) {
    return -1;
  }
  Configuration().setValue(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, dirSelected);
  const int mediaFilesCnt = mVideoList->setPlayPath(dirSelected);
  return mediaFilesCnt;
}

void VideoView::StopPlay() {
  if (mBasicVideoView) {
    mBasicVideoView->StopPlay();
  }
}

void VideoView::onChangeToolBarVisibility(bool bHide) {
  const bool newVisibility = !bHide;
  if (mExtendedFunctionCtrlBar->isVisible() != newVisibility) {
    mExtendedFunctionCtrlBar->setVisible(newVisibility);
    mBasicVideoView->movePauseBtnToCenter();
  }
}

void VideoView::onChangeListVisibility(bool visibility) {
  if (mVideoList->isVisible() != visibility) {
    mVideoList->setVisible(visibility);
    mBasicVideoView->movePauseBtnToCenter();
  }
}
