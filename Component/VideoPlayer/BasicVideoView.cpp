#include "BasicVideoView.h"
#include "DataFormatter.h"
#include "DualIconCheckableAction.h"
#include "Logger.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "FileTool.h"
#include "RateHelper.h"
#include "RateActions.h"
#include "PathTool.h"
#include <QFile>
#include <QResizeEvent>

BasicVideoView::BasicVideoView(bool bBasicMode, QWidget* parent) : QWidget{parent} {
  mProgressSlider = new (std::nothrow) ClickableSlider{Qt::Orientation::Horizontal, this};
  mProgressSlider->setRange(0, 0);
  mProgressSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  mProgressSlider->setSingleStep(10 * 1000); // 10 second

  mVolumeWid = new VolumeWidget{QBoxLayout::Direction::LeftToRight, this};
  mVolumeWid->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  mPlayer = new (std::nothrow) QMediaPlayer{this};                             // never swap these 2 lines, must desctruct first
  mVideoWidget = new (std::nothrow) InteractiveVideoWidget{bBasicMode, this};  // never swap these 2 lines, must desctruct behind player
  mPlayer->setVolume(mVolumeWid->volumeValLog());
  mPlayer->setMuted(mVolumeWid->isMuted());
  mPlayer->setVideoOutput(mVideoWidget);

  MenuToolButton* playbackTriggerModeBtn = mVideoWidget->GetPlaybackTriggerModelMenuToolButton(this);
  playbackTriggerModeBtn->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
  setPlaybackTriggerMode(mVideoWidget->GetPlaybackTriggerMode());

  mFunctionCtrlBar = new (std::nothrow) ToolBarWidget{QBoxLayout::Direction::LeftToRight, this};
  mFunctionCtrlBar->addWidget(playbackTriggerModeBtn);
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
  mFunctionCtrlBar->addWidget(mVolumeWid);
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

  mProgressSliderUpdateTimer.setInterval(1 * 1000); // refresh frequency: 1Hz
  mProgressSliderUpdateTimer.setSingleShot(false);
  QTimer::singleShot(0, this, [this]() { movePauseBtnToCenter(); });

  mProgressSlider->installEventFilter(this);
  mVideoWidget->installEventFilter(this);
}

BasicVideoView::~BasicVideoView() {
  onStopPlaying();

  // if (mPlayer) {
  //   onStopPlaying();
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
  connect(this, &BasicVideoView::userMousePressOrKeyPressHappened, mVideoWidget, &InteractiveVideoWidget::onUserMouseClickOrKeyPressEvent);

  connect(mVideoWidget->mPauseAct, &QAction::toggled, this, &BasicVideoView::onPauseActionToggled);
  connect(mVideoWidget->mStopAct, &QAction::triggered, this, &BasicVideoView::onStopPlaying);
  // -10 second, +10 second
  connect(mVideoWidget->mSeekBackwardAct, &QAction::triggered, this, &BasicVideoView::deviatePositionPrevious);
  connect(mVideoWidget->mSeekForwardAct, &QAction::triggered, this, &BasicVideoView::deviatePositionNext);

  connect(mVideoWidget->mOpenInSystemApplication, &QAction::triggered, this, &BasicVideoView::reqPlayInSystemApplication);

  connect(mPlayer, &QMediaPlayer::durationChanged, this, &BasicVideoView::onDurationChanged);
  // 直接点击滑动条->更新滑动块到点击位置, 并播放
  // 滑动滑动条->更新位置
  mProgressSlider->regMouseEventProcessor(std::bind(&QMediaPlayer::setPosition, mPlayer, std::placeholders::_1));
  // 播放过程中滑动块位置增加, 更新当前时间点标签, 更新滑动块的当前时间
  connect(&mProgressSliderUpdateTimer, &QTimer::timeout, this, &BasicVideoView::onUpdateProgressSliderPosition);

  connect(mVolumeWid, &VolumeWidget::mutedStateToggled, mPlayer, &QMediaPlayer::setMuted);
  connect(mVolumeWid, &VolumeWidget::sliderVolumeChanged, mPlayer, &QMediaPlayer::setVolume);
  connect(mVideoWidget->mVolumePlus, &QAction::triggered, mVolumeWid, &VolumeWidget::reqLogVolumeIncrease);
  connect(mVideoWidget->mVolumeMinus, &QAction::triggered, mVolumeWid, &VolumeWidget::reqLogVolumeDecrease);

  connect(mPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &BasicVideoView::onError);

  connect(mPlayer, &QMediaPlayer::audioAvailableChanged, this, &BasicVideoView::onAudioAvailableChanged);

  connect(mPlayer, &QMediaPlayer::stateChanged, this, &BasicVideoView::onStateChanged);
  connect(mPlayer, &QMediaPlayer::mediaStatusChanged, this, &BasicVideoView::onMediaStatusChanged);

  connect(mVideoWidget, &InteractiveVideoWidget::newFileSelectedByUser, this, &BasicVideoView::PlayAVideo);
  connect(mVideoWidget, &InteractiveVideoWidget::fullScreenModeToggled, this, &BasicVideoView::emitFullScreenModeReq);
  connect(mVideoWidget->mBasicModeAct, &QAction::toggled, this, &BasicVideoView::reqFunctionModeChange);

  connect(mVideoWidget->mHideToolBarAct, &QAction::toggled, this, &BasicVideoView::onChangeToolBarVisibility);

  connect(mVideoWidget, &InteractiveVideoWidget::playbackTriggerModeChanged, this, &BasicVideoView::setPlaybackTriggerMode);

  connect(mVideoWidget, &InteractiveVideoWidget::layoutVisibilityChanged, this, &BasicVideoView::movePauseBtnToCenter);

  RateActions* rateActions = mVideoWidget->GetRateActions();
  connect(rateActions, &RateActions::RateMovieReq, this, &BasicVideoView::rateCurrentVideo);
  connect(rateActions, &RateActions::RateMovieRecursivelyReq, this, &BasicVideoView::rateAllVideoSameLevelAsCurrentVideo);
  connect(rateActions, &RateActions::AdjustRateMovieReq, this, &BasicVideoView::adjustRateCurrentVideo);
  connect(rateActions, &RateActions::AdjustRateMovieRecursivelyReq, this, &BasicVideoView::adjustRateAllVideoSameLevelAsCurrentVideo);
}

void BasicVideoView::emitFullScreenModeReq(bool bFullScreen) {
  LOG_OK_P("Fullscreen Mode Changed", "bFullScreen:%d", bFullScreen);
  emit reqFullscreenModeChange(bFullScreen);
  movePauseBtnToCenter();
}

bool BasicVideoView::PlayAVideo(const QString& filePath, bool forcePlayInstantly) {
  if (!bPauseButtonCenterInit) {
    bPauseButtonCenterInit = true;
    movePauseBtnToCenter();
  }
  mCurrentPlayingMediaPath = "";
  setWindowTitle(mCurrentPlayingMediaPath);
  if (!QFile::exists(filePath)) {
    return false;
  }
  mCurrentPlayingMediaPath = filePath;
  setWindowTitle(mCurrentPlayingMediaPath);

  using namespace VideoPlayTool;
  const PlaybackTriggerMode playTriggerMode{forcePlayInstantly ? PlaybackTriggerMode::AUTO : GetPlayTriggerMode()};
  switch (playTriggerMode) {
    case VideoPlayTool::PlaybackTriggerMode::MANUAL: {  // 仅仅设置
      SetMediaCore(this, filePath);
      mVideoWidget->updatePauseActionState(true);
      movePauseBtnToCenter();
      break;
    }
    case VideoPlayTool::PlaybackTriggerMode::AUTO: {  // 设置+播放
      SetMediaCore(this, filePath);
      if (!mVideoWidget->updatePauseActionState(false)) {
        // mPauseAct状态无需更新时, 需要重新播放
        PlayCore(this->mPlayer);
      }
      break;
    }
    case VideoPlayTool::PlaybackTriggerMode::DISABLED: {
      mVideoWidget->updatePauseActionState(true);
      movePauseBtnToCenter();
      break;
    }
    default: {
      LOG_W("playTriggerMode[%d] invalid, forcePlayInstantly[%d], should never ran into here", (int)playTriggerMode, forcePlayInstantly);
      break;
    }
  }
  return true;
}

void BasicVideoView::onChangeToolBarVisibility(bool bHide) {
  if (mFunctionCtrlBar->isHidden() != bHide) {
    mFunctionCtrlBar->setVisible(!bHide);
    movePauseBtnToCenter();
  }
}

bool BasicVideoView::isVideoFullScreen() const {
  return mVideoWidget != nullptr && mVideoWidget->isVideoFullScreen();
}

bool BasicVideoView::registerFullScreenToggleCallback(TFuncFullScreenToggleCallback funcCallback) {
  CHECK_NULLPTR_RETURN_FALSE(funcCallback);
  if (mFullScreenCallback != nullptr) {
    LOG_W("Full screen callback already set, cannot reset");
    return false;
  }
  mFullScreenCallback = funcCallback;
  connect(this, &BasicVideoView::reqFullscreenModeChange, mFullScreenCallback);
  return true;
}

bool BasicVideoView::rateCurrentVideo(int score) const {
  const QString curMedia = GetCurrentPlayingMediaPath();
  if (!QFile::exists(curMedia)) {
    LOG_WARN_P("Cannot rate", "Media file[%s] not exist", qPrintable(curMedia));
    return false;
  }
  if (!RateHelper::RateMovie(curMedia, score)) {
    LOG_WARN_P("Cannot rate", "Media file[%s], see details in log", qPrintable(curMedia));
    return false;
  }
  LOG_OK_P("Rate succeed", "Media file[%s] rate[%d]", qPrintable(curMedia), score);
  return true;
}

bool BasicVideoView::adjustRateCurrentVideo(int delta) const {
  if (delta == 0) {
    return true;
  }
  const QString curMedia = GetCurrentPlayingMediaPath();
  if (!QFile::exists(curMedia)) {
    LOG_WARN_P("Cannot rate", "Media file[%s] not exist", qPrintable(curMedia));
    return false;
  }
  if (!RateHelper::AdjustRateMovie(curMedia, delta)) {
    LOG_WARN_P("Cannot adjust rate", "Media file[%s], see details in log", qPrintable(curMedia));
    return false;
  }
  LOG_OK_P("Adjust Rate succeed", "Media file[%s] rate adjust[%d]", qPrintable(curMedia), delta);
  return true;
}

int BasicVideoView::rateAllVideoSameLevelAsCurrentVideo(bool bOverrideForce) const {
  const QString& curMedia = GetCurrentPlayingMediaPath();
  const QString sameLevelPath{PathTool::absolutePath(curMedia)};
  if (!QFile::exists(sameLevelPath)) {
    LOG_WARN_P("Cannot rate", "Media folder[%s] of file[%s] not exist", qPrintable(sameLevelPath), qPrintable(curMedia));
    return 0;
  }
  RateActions* rateActions = mVideoWidget->GetRateActions();
  return rateActions->onRateMoviesRecursively(sameLevelPath, bOverrideForce, nullptr);  // no need modal widget
}

int BasicVideoView::adjustRateAllVideoSameLevelAsCurrentVideo(int delta) const {
  if (delta == 0) {
    return 0;
  }
  const QString& curMedia = GetCurrentPlayingMediaPath();
  const QString sameLevelPath{PathTool::absolutePath(curMedia)};
  if (!QFile::exists(sameLevelPath)) {
    LOG_WARN_P("Cannot adjust rate", "Media folder[%s] of file[%s] not exist", qPrintable(sameLevelPath), qPrintable(curMedia));
    return 0;
  }
  return RateHelper::AdjustRateMovieRecursively(sameLevelPath, delta);  // no need widget
}

bool BasicVideoView::deviatePositionPrevious() {
  return DeviatePositionCore(mPlayer, -10);
}

bool BasicVideoView::deviatePositionNext() {
  return DeviatePositionCore(mPlayer, 10);
}

bool BasicVideoView::DeviatePositionCore(QMediaPlayer* mPlayer, int deviationInSeconds) {
  CHECK_NULLPTR_RETURN_FALSE(mPlayer);
  qint64 newPosition = std::max((qint64)0, GetPositionCore(mPlayer) + deviationInSeconds * 1000);
  return SetPositionCore(mPlayer, newPosition);
}

bool BasicVideoView::SetPositionCore(QMediaPlayer* mPlayer, int newPosition) {
  CHECK_NULLPTR_RETURN_FALSE(mPlayer);
  mPlayer->setPosition(newPosition);
  return true;
}

qint64 BasicVideoView::GetPositionCore(QMediaPlayer* mPlayer) {
  CHECK_NULLPTR_RETURN_INT(mPlayer, (qint64)0);
  return mPlayer->position();
}

bool BasicVideoView::reqPlayInSystemApplication() const {
  const QString& mediaPath = GetCurrentPlayingMediaPath();
  return FileTool::OpenLocalFileUsingDesktopService(mediaPath);
}

QMediaPlayer::Error BasicVideoView::onError(QMediaPlayer::Error error) const {
  LOG_E("Player error:%d", error);
  return mError = error;
}

int BasicVideoView::onAudioAvailableChanged(bool available) const {
  int volValue = mPlayer->volume();
  LOG_D("Audio available: %d, volume:%d", available, volValue);
  return volValue;
}

void BasicVideoView::setMediaWithStatus(const QString &filePath, QIODevice *stream) {
  CHECK_NULLPTR_RETURN_VOID(mPlayer);
  mIsMediaCleared = filePath.isEmpty();
  mPlayer->setMedia(QUrl::fromLocalFile(filePath), stream);
}

void BasicVideoView::onDurationChanged(qint64 duration) {
  mDurationLabel->setText(DataFormatter::formatDurationISO(duration));
  mProgressSlider->setRange(0, duration);
}

bool BasicVideoView::onUpdateProgressSliderPosition() {
  const qint64 newPosition = GetPositionCore(mPlayer);
  if (mProgressSlider->value() == newPosition) {
    return false;
  }
  mCurrentTimeLabel->setText(DataFormatter::formatDurationISO(newPosition));
  mProgressSlider->setValue(newPosition);
  return true;
}

void BasicVideoView::onStopPlaying() {
  CHECK_NULLPTR_RETURN_VOID(mPlayer);
  if (!mVideoWidget->mPauseAct->isChecked()) {
    mVideoWidget->mPauseAct->toggle();
  }
  if (mPlayer->state() != QMediaPlayer::StoppedState) {
    mPlayer->stop();
    SetMediaCore(this, "");
  }
}

void BasicVideoView::onPauseActionToggled(bool pauseChecked) {
  CHECK_NULLPTR_RETURN_VOID(mPlayer);
  if (pauseChecked) {
    mPlayer->pause();
  } else {
    if (mIsMediaCleared) {
      const QString pth = GetCurrentPlayingMediaPath();
      if (!QFile::exists(pth)) {
        return;
      }
      setMediaWithStatus(pth);
    }
    PlayCore(mPlayer);
  }
}

void BasicVideoView::onStateChanged(QMediaPlayer::State state) {
  if (state == QMediaPlayer::PausedState) {
    mPauseShieldButton->raise();
    mPauseShieldButton->setVisible(true);
  } else {
    mPauseShieldButton->lower();
    mPauseShieldButton->setVisible(false);
  }

  if (state == QMediaPlayer::PlayingState) {
    mProgressSliderUpdateTimer.start();
  } else {
    mProgressSliderUpdateTimer.stop();
  }
}

void BasicVideoView::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
  if (status == QMediaPlayer::MediaStatus::EndOfMedia) {
    const QMediaPlaylist::PlaybackMode curPlaybackMode{mVideoWidget->GetPlaybackMode()};
    if (curPlaybackMode != QMediaPlaylist::PlaybackMode::CurrentItemOnce) {
      emit reqPlayNextOneMedia();
    }
  }
}

bool BasicVideoView::eventFilter(QObject *watched, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::KeyPress) {
    emit userMousePressOrKeyPressHappened();
  }
  return QWidget::eventFilter(watched, event);
}

void BasicVideoView::resizeEvent(QResizeEvent* e) {
  CHECK_NULLPTR_RETURN_VOID(e);
  movePauseBtnToCenter();
}

void BasicVideoView::movePauseBtnToCenter() {
  mPauseShieldButton->move(mVideoWidget->width() / 2 - mPauseShieldButton->width() / 2,  //
                           mVideoWidget->height() / 2 - mPauseShieldButton->height() / 2);
}

bool BasicVideoView::SetMediaCore(BasicVideoView* self, const QString& mediaAbsPath) {
  if (self == nullptr) {
    return false;
  }
  self->setMediaWithStatus(mediaAbsPath);
  return true;
}

bool BasicVideoView::PlayCore(QMediaPlayer* mediaPlayer) {
  if (mediaPlayer == nullptr) {
    return false;
  }
  mediaPlayer->play();
  return true;
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
