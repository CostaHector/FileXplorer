#include "BasicVideoView.h"
#include "DataFormatter.h"
#include "DualIconCheckableAction.h"
#include "Logger.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include <QMouseEvent>
#include <QFile>
#include <QResizeEvent>
#include <QFileDialog>

constexpr int PausableVideoWidget::TIMER_INTERVAL;

PausableVideoWidget::PausableVideoWidget(QWidget* parent)
  : QVideoWidget{parent} {
  mLongTimeNoClickTimer.setSingleShot(true);
  mLongTimeNoClickTimer.setInterval(TIMER_INTERVAL);

  connect(&mLongTimeNoClickTimer, &QTimer::timeout, this, &PausableVideoWidget::onTimerTimeout);
}

void PausableVideoWidget::onIntoFullScreenMode() {
  mLongTimeNoClickTimer.start();
}

void PausableVideoWidget::onQuitFullScreenMode() {
  mLongTimeNoClickTimer.stop();
}

void PausableVideoWidget::onTimerTimeout() {
  emit reqToolBarVisibilityChange(false);
}

void PausableVideoWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::MouseButton::LeftButton) {
    emit reqPausePlayStatusToggle();
    event->setAccepted(true);
    return;
  } else {
    emit reqToolBarVisibilityChange(true);
    mLongTimeNoClickTimer.start(); // 已经在运行中则会保持间隔重启
    event->setAccepted(true);
    return;
  }
  QVideoWidget::mousePressEvent(event);
}

BasicVideoView::BasicVideoView(bool bBasicMode, QWidget* parent)
  : QWidget{parent} {
  mFunctionCtrlBar = new (std::nothrow) ToolBarWidget{QBoxLayout::Direction::LeftToRight, this};
  mSelectVideoFileBtn = mFunctionCtrlBar->createToolButton(QIcon{":/VideoPlayer/OPEN_A_VIDEO"}, tr("select video"));

  const bool bPlayInstant
      = Configuration().value(MemoryKey::VIDEO_PLAYER_PLAY_SELECT_INSTANTLY.name, MemoryKey::VIDEO_PLAYER_PLAY_SELECT_INSTANTLY.v).toBool();
  mPlayInstantlyBtn = mFunctionCtrlBar->createToolButton(QIcon{""},
                                                         tr("instant"),
                                                         "Automatically play selected file instantly in current view when enabled.",
                                                         true);
  mPlayInstantlyBtn->defaultAction()->setChecked(bPlayInstant);

  mPauseAct = DualIconCheckableAction::CreatePauseAction(this, !bPlayInstant);
  mPauseBtn = mFunctionCtrlBar->createToolButton(mPauseAct);

  mStopBtn = mFunctionCtrlBar->createToolButton(QIcon{":/VideoPlayer/STOP_VIDEO"}, tr("stop"), "Stop playing and move position to time 0.");
  mSeekBackwardBtn = mFunctionCtrlBar->createToolButton(QIcon{":/VideoPlayer/SEEK_BACKWARD"}, tr("seek backward 10s"));
  mSeekForwardBtn = mFunctionCtrlBar->createToolButton(QIcon{":/VideoPlayer/SEEK_FORWARD"}, tr("seek forward 10s"));
  mProgressSlider = new (std::nothrow) ClickableSlider{Qt::Orientation::Horizontal, this};
  mProgressSlider->setRange(0, 0);
  mProgressSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  mVolumeWid = new VolumeWidget{QBoxLayout::Direction::LeftToRight, this};
  mVolumeWid->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  mBasicModeBtn = mFunctionCtrlBar->createToolButton(QIcon{":/VideoPlayer/VIDEO_PLAYER_BASIC"}, tr("basic mode"), "", true);
  mBasicModeBtn->defaultAction()->setChecked(bBasicMode);
  mFullScreenAct = DualIconCheckableAction::CreateFullSceenAction(this, false);
  mFullScreenBtn = mFunctionCtrlBar->createToolButton(mFullScreenAct);

  mFunctionCtrlBar->addWidget(mPlayInstantlyBtn);
  mFunctionCtrlBar->addSeparator();
  mFunctionCtrlBar->addWidget(mSelectVideoFileBtn);
  mFunctionCtrlBar->addWidget(mPauseBtn);
  mFunctionCtrlBar->addWidget(mStopBtn);
  mFunctionCtrlBar->addSeparator();
  mFunctionCtrlBar->addWidget(mSeekBackwardBtn);
  mFunctionCtrlBar->addWidget(mSeekForwardBtn);
  mFunctionCtrlBar->addSeparator();
  mFunctionCtrlBar->addWidget(mProgressSlider, 10);
  mCurrentTimeLabel = mFunctionCtrlBar->addString("00:00:00");
  mFunctionCtrlBar->addString("/");
  mDurationLabel = mFunctionCtrlBar->addString("00:00:00");
  mFunctionCtrlBar->addWidget(mVolumeWid, 2);
  mFunctionCtrlBar->addSeparator();
  mFunctionCtrlBar->addWidget(mBasicModeBtn);
  mFunctionCtrlBar->addWidget(mFullScreenBtn);

  mPlayer = new (std::nothrow) QMediaPlayer{this};             // never swap these 2 lines, must desctruct first
  mVideoWidget = new (std::nothrow) PausableVideoWidget{this}; // never swap these 2 lines, must desctruct behind player
  mPlayer->setVolume(mVolumeWid->volumeVal());
  mPlayer->setMuted(mVolumeWid->isMuted());
  mPlayer->setVideoOutput(mVideoWidget);

  mPauseShieldButton = new (std::nothrow) QToolButton{this};
  mPauseShieldButton->setObjectName("VideoPlayerPauseShieldButton");
  mPauseShieldButton->setDefaultAction(mPauseAct);
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
  Configuration().setValue(MemoryKey::VIDEO_PLAYER_PLAY_SELECT_INSTANTLY.name, mPlayInstantlyBtn->isChecked());
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
  connect(mPauseAct, &QAction::toggled, this, &BasicVideoView::onPauseActionToggled);
  connect(mStopBtn, &QToolButton::triggered, this, &BasicVideoView::onStopPlaying);
  connect(mSeekBackwardBtn, &QToolButton::triggered, this, [this]() { // -10 second
    mPlayer->setPosition(std::max((qint64) 0, mPlayer->position() - 10 * 1000));
  });
  connect(mSeekForwardBtn, &QToolButton::triggered, this, [this]() { // +10 second
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
  connect(mSelectVideoFileBtn, &QToolButton::triggered, this, &BasicVideoView::onSelectAFile);
  connect(mFullScreenAct, &QAction::toggled, this, &BasicVideoView::emitFullScreenModeReq);
  connect(mBasicModeBtn, &QToolButton::toggled, this, &BasicVideoView::reqFunctionModeChange);

  connect(mVideoWidget, &PausableVideoWidget::reqPausePlayStatusToggle, this, [this]() {
    const bool bNewPause = !mPauseAct->isChecked();
    mPauseAct->setChecked(bNewPause);
  });
  connect(GetVideoWidget(), &PausableVideoWidget::reqToolBarVisibilityChange, this, &BasicVideoView::onChangeToolBarVisibility);
}

void BasicVideoView::emitFullScreenModeReq(bool bFullScreen) {
  emit reqFullscreenModeChange(bFullScreen);
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
  if (isAutoPlay() || forcePlayInstantly) {
    mPauseAct->setChecked(false);
    mPlayer->play();
  } else {
    mPauseAct->setChecked(true);
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

void BasicVideoView::onChangeToolBarVisibility(bool visibility) {
  // mFunctionCtrlBar->setVisible(visibility);
  mFunctionCtrlBar->setHidden(!visibility);
  movePauseBtnToCenter();
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
  mPauseAct->setChecked(true);
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

void BasicVideoView::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key::Key_Space) {
    const bool bNewPause = !mPauseAct->isChecked();
    mPauseAct->setChecked(bNewPause);
    event->setAccepted(true);
    return;
  } else if (event->key() == Qt::Key::Key_Escape) {
    if (isFullScreen()) {
      mFullScreenAct->setChecked(false);
      event->setAccepted(true);
      return;
    }
  }
  QWidget::keyPressEvent(event);
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
