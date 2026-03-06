#include "BasicVideoView.h"
#include "DataFormatter.h"
#include "DualIconCheckableAction.h"
#include "Logger.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include <QFile>
#include <QResizeEvent>
#include <QFileDialog>

BasicVideoView::BasicVideoView(bool bBasicMode, QWidget* parent)
  : QWidget{parent} {
  mProgressSlider = new (std::nothrow) ClickableSlider{Qt::Orientation::Horizontal, this};
  mProgressSlider->setRange(0, 0);
  mProgressSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  mVolumeWid = new VolumeWidget{QBoxLayout::Direction::LeftToRight, this};
  mVolumeWid->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  mPlayer = new (std::nothrow) QMediaPlayer{this};                // never swap these 2 lines, must desctruct first
  mVideoWidget = new (std::nothrow) InteractiveVideoWidget{this}; // never swap these 2 lines, must desctruct behind player
  mVideoWidget->mBasicModeAct->setChecked(bBasicMode);
  mPlayer->setVolume(mVolumeWid->volumeVal());
  mPlayer->setMuted(mVolumeWid->isMuted());
  mPlayer->setVideoOutput(mVideoWidget);

  mFunctionCtrlBar = new (std::nothrow) ToolBarWidget{QBoxLayout::Direction::LeftToRight, this};
  mFunctionCtrlBar->addAction(mVideoWidget->mPlayInstantlyAct);
  mFunctionCtrlBar->addSeparator();
  mFunctionCtrlBar->addAction(mVideoWidget->mSelectVideoFileAct);
  mFunctionCtrlBar->addAction(mVideoWidget->mPauseAct);
  mFunctionCtrlBar->addAction(mVideoWidget->mStopAct);
  mFunctionCtrlBar->addSeparator();
  mFunctionCtrlBar->addAction(mVideoWidget->mSeekBackwardAct);
  mFunctionCtrlBar->addAction(mVideoWidget->mSeekForwardAct);
  mFunctionCtrlBar->addSeparator();
  mFunctionCtrlBar->addWidget(mProgressSlider, 10);
  mCurrentTimeLabel = mFunctionCtrlBar->addString("00:00:00");
  mFunctionCtrlBar->addString("/");
  mDurationLabel = mFunctionCtrlBar->addString("00:00:00");
  mFunctionCtrlBar->addWidget(mVolumeWid, 2);
  mFunctionCtrlBar->addSeparator();
  mFunctionCtrlBar->addAction(mVideoWidget->mBasicModeAct);
  mFunctionCtrlBar->addAction(mVideoWidget->mHideToolBarAct);
  mFunctionCtrlBar->addAction(mVideoWidget->mFullScreenAct);

  mPauseShieldButton = new (std::nothrow) QToolButton{this};
  mPauseShieldButton->setObjectName("VideoPlayerPauseShieldButton");
  mPauseShieldButton->setDefaultAction(mVideoWidget->mPauseAct);
  mPauseShieldButton->setMinimumSize(64, 64);
  mPauseShieldButton->setAutoRaise(true);

  mLeftLayout = new (std::nothrow) QVBoxLayout{this};
  mLeftLayout->addWidget(mVideoWidget);
  mLeftLayout->addWidget(mFunctionCtrlBar);
  mLeftLayout->setContentsMargins(0, 0, 0, 0);

  subscribe();
  setWindowIcon(QIcon{":/VideoPlayer/VIDEO_PLAYER_BASIC"});
  setWindowTitle("Basic Video Player");
  QTimer::singleShot(0, this, [this]() { movePauseBtnToCenter(); });
}

BasicVideoView::~BasicVideoView() {
  StopPlay();

  // if (mPlayer) {
  //   StopPlay();
  //   mPlayer->disconnect();
  //   delete mPlayer;
  //   mPlayer = nullptr;
  // }
  // if (mVideoWidget) {
  //   delete mVideoWidget;
  //   mVideoWidget = nullptr;
  // }
}

void BasicVideoView::subscribe() {
  connect(mVideoWidget->mPauseAct, &QAction::toggled, this, &BasicVideoView::onPauseActionToggled);
  connect(mVideoWidget->mStopAct, &QAction::triggered, this, &BasicVideoView::onStopPlaying);
  connect(mVideoWidget->mSeekBackwardAct, &QAction::triggered, this, [this]() { // -10 second
    mPlayer->setPosition(std::max((qint64) 0, mPlayer->position() - 10 * 1000));
  });
  connect(mVideoWidget->mSeekForwardAct, &QAction::triggered, this, [this]() { // +10 second
    mPlayer->setPosition(std::min(mPlayer->duration(), mPlayer->position() + 10 * 1000));
  });

  connect(mPlayer, &QMediaPlayer::durationChanged, this, &BasicVideoView::durationChanged);
  // 直接点击滑动条->更新滑动块到点击位置, 并播放
  mProgressSlider->reg(std::bind(&QMediaPlayer::setPosition, mPlayer, std::placeholders::_1));
  // 滑动滑动条->更新位置
  connect(mProgressSlider, &QAbstractSlider::sliderMoved, mPlayer, &QMediaPlayer::setPosition);
  // 播放过程中滑动块位置增加, 更新当前时间点标签, 更新滑动块的当前时间
  connect(mPlayer, &QMediaPlayer::positionChanged, this, &BasicVideoView::onPlaying);

  connect(mVolumeWid, &VolumeWidget::onMutedChanged, mPlayer, &QMediaPlayer::setMuted);
  connect(mVolumeWid, &VolumeWidget::onVolumeChanged, mPlayer, &QMediaPlayer::setVolume);

  connect(mPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, [](QMediaPlayer::Error error) {
    LOG_E("Player error:%d", error);
  });

  connect(mPlayer, &QMediaPlayer::audioAvailableChanged, this, [this](bool available) {
    LOG_D("Audio available: %d, volume:%d", available, mPlayer->volume());
  });

  connect(mPlayer, &QMediaPlayer::stateChanged, this, &BasicVideoView::onMediaPlayStateChanged);
  connect(mVideoWidget->mSelectVideoFileAct, &QAction::triggered, this, &BasicVideoView::onSelectAFile);
  connect(mVideoWidget->mFullScreenAct, &QAction::toggled, this, &BasicVideoView::emitFullScreenModeReq);
  connect(mVideoWidget->mBasicModeAct, &QAction::toggled, this, &BasicVideoView::reqFunctionModeChange);

  connect(mVideoWidget->mHideToolBarAct, &QAction::toggled, this, &BasicVideoView::onChangeToolBarVisibility);

  connect(mVideoWidget, &InteractiveVideoWidget::layoutVisibilityChanged, this, &BasicVideoView::movePauseBtnToCenter);
}

void BasicVideoView::emitFullScreenModeReq(bool bFullScreen) {
  LOG_OK_P("Fullscreen Mode Changed", "bFullScreen:%d", bFullScreen);
  emit reqFullscreenModeChange(bFullScreen);
  movePauseBtnToCenter();
  if (bFullScreen) {
    mVideoWidget->onIntoFullScreenMode();
  } else {
    mVideoWidget->onQuitFullScreenMode();
  }
}

bool BasicVideoView::PlayAVideo(const QString& filePath, bool forcePlayInstantly) {
  if (!bPauseButtonCenterInit) {
    bPauseButtonCenterInit = true;
    movePauseBtnToCenter();
  }
  if (!QFile::exists(filePath)) {
    return false;
  }
  mPlayer->setMedia(QUrl::fromLocalFile(filePath));
  mCurrentPlayingMediaPath = filePath;
  if (mVideoWidget->isAutoPlay() || forcePlayInstantly) {
    mVideoWidget->mPauseAct->setChecked(false);
    mPlayer->play();
  } else {
    mVideoWidget->mPauseAct->setChecked(true);
  }
  return true;
}

void BasicVideoView::StopPlay() {
  if (mPlayer != nullptr) {
    if (mPlayer->state() != QMediaPlayer::StoppedState) {
      mPlayer->stop();
    }
  }
}

void BasicVideoView::onChangeToolBarVisibility(bool bHide) {
  const bool newVisibility = !bHide;
  if (mFunctionCtrlBar->isVisible() != newVisibility) {
    mFunctionCtrlBar->setVisible(newVisibility);
    movePauseBtnToCenter();
  }
}

bool BasicVideoView::isVideoFullScreen() const {
  return mVideoWidget != nullptr && mVideoWidget->isVideoFullScreen();
}

void BasicVideoView::durationChanged(qint64 duration) {
  mDurationLabel->setText(DataFormatter::formatDurationISO(duration));
  mProgressSlider->setRange(0, duration);
}

void BasicVideoView::onPlaying(qint64 position) {
  mCurrentTimeLabel->setText(DataFormatter::formatDurationISO(position));
  mProgressSlider->setValue(position);
}

void BasicVideoView::onStopPlaying() {
  mVideoWidget->mPauseAct->setChecked(true);
  StopPlay();
}

void BasicVideoView::onPauseActionToggled(bool pauseChecked) {
  if (mPlayer != nullptr) {
    if (pauseChecked) {
      mPlayer->pause();
    } else {
      mPlayer->play();
    }
  }
}

void BasicVideoView::onMediaPlayStateChanged(QMediaPlayer::State state) {
  if (state == QMediaPlayer::PausedState) {
    mPauseShieldButton->raise();
    mPauseShieldButton->setVisible(true);
  } else {
    mPauseShieldButton->lower();
    mPauseShieldButton->setVisible(false);
  }
}

bool BasicVideoView::onSelectAFile() {
  QString defaultOpenPathLocatedIn
      = Configuration().value(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.v).toString();
  if (!QFile::exists(defaultOpenPathLocatedIn)) {
    defaultOpenPathLocatedIn = SystemPath::HOME_PATH();
  }
  static const QString filterStr = "Video Files (" + TYPE_FILTER::VIDEO_TYPE_SET.join(" ") + ")";
  QString fileSelected = QFileDialog::getOpenFileName(this,
                                                      "Select a video file", //
                                                      defaultOpenPathLocatedIn,
                                                      filterStr);
  if (fileSelected.isEmpty()) {
    return false;
  }
  Configuration().setValue(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, PathTool::absolutePath(fileSelected));
  return PlayAVideo(fileSelected, true);
}

void BasicVideoView::resizeEvent(QResizeEvent* e) {
  if (mPauseShieldButton->isVisible()) {
    movePauseBtnToCenter();
  }
}

void BasicVideoView::movePauseBtnToCenter() {
  mPauseShieldButton->move(mVideoWidget->width() / 2 - mPauseShieldButton->width() / 2, //
                           mVideoWidget->height() / 2 - mPauseShieldButton->height() / 2);
}

/*
 * in ubuntu system when you try to play a .vob file, it may say
Warning: "No decoder available for type 'video/mpeg, systemstream=(boolean)true, mpegversion=(int)2'."
Error: "Your GStreamer installation is missing a plug-in."

检查版本
$ gst-launch-1.0 --version
gst-launch-1.0 version 1.16.3
GStreamer 1.16.3
https://launchpad.net/distros/ubuntu/+source/gstreamer1.0

检查如下的插件: 没有时会报错 No such element or plugin 'mpeg2dec'
$ gst-inspect-1.0 mpeg2dec
$ gst-inspect-1.0 mpegpsdemux

# 启用仓库
sudo add-apt-repository multiverse
# 需要安装如下内容
sudo apt install gstreamer1.0-plugins-ugly
sudo apt install gstreamer1.0-plugins-bad
sudo apt install gstreamer1.0-libav

sudo apt install libdvd-pkg
sudo dpkg-reconfigure libdvd-pkg
sudo apt install vlc-plugin-notify

sudo apt install ubuntu-restricted-extras
 */
