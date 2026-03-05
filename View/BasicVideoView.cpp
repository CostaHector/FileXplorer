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

BasicVideoView::BasicVideoView(bool bBasicMode, QWidget *parent)
  : QWidget{parent} {
  mFunctionCtrlBar = new ToolBarWidget{QBoxLayout::Direction::LeftToRight, this};
  mFunctionCtrlBar->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
  mSelectVideoFileBtn = mFunctionCtrlBar->createToolButton(QIcon{":/VideoPlayer/OPEN_A_VIDEO"}, tr("open file"));

  const bool bPlayInstant
      = Configuration().value(MemoryKey::VIDEO_PLAYER_PLAY_SELECT_INSTANTLY.name, MemoryKey::VIDEO_PLAYER_PLAY_SELECT_INSTANTLY.v).toBool();
  mPlayInstantlyBtn = mFunctionCtrlBar->createToolButton(QIcon{""},
                                                         tr("instant"),
                                                         "Automatically play selected file instantly in current view when enabled.",
                                                         true);
  mPlayInstantlyBtn->setChecked(bPlayInstant);

  mPauseAct = DualIconCheckableAction::CreatePauseAction(this, !bPlayInstant);
  mPauseBtn = mFunctionCtrlBar->createToolButton(mPauseAct);

  mStopBtn = mFunctionCtrlBar->createToolButton(QIcon{":/VideoPlayer/STOP_VIDEO"}, tr("stop"), "Stop playing and move position to time 0.");
  mSeekBackwardBtn = mFunctionCtrlBar->createToolButton(QIcon{":/VideoPlayer/SEEK_BACKWARD"}, tr("seek backward 10s"));
  mSeekForwardBtn = mFunctionCtrlBar->createToolButton(QIcon{":/VideoPlayer/SEEK_FORWARD"}, tr("seek forward 10s"));
  mProgressSlider = new ClickableSlider{Qt::Orientation::Horizontal, this};
  mProgressSlider->setRange(0, 0);
  mProgressSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  mCurrentTimeLabel = new QLabel{"00:00:00", this};
  mDurationLabel = new QLabel{"00:00:00", this};

  mVolumeWid = new VolumeWidget{QBoxLayout::Direction::LeftToRight, this};
  mVolumeWid->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  mBasicModeBtn = mFunctionCtrlBar->createToolButton(QIcon{":/VideoPlayer/VIDEO_PLAYER_BASIC"}, tr("basic mode"), "", true);
  mBasicModeBtn->setChecked(bBasicMode);
  auto* pFullScreenTemp = DualIconCheckableAction::CreateFullSceenAction(this, false);
  mFullScreenBtn = mFunctionCtrlBar->createToolButton(pFullScreenTemp);

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
  mFunctionCtrlBar->addWidget(mCurrentTimeLabel);
  mFunctionCtrlBar->addString("/");
  mFunctionCtrlBar->addWidget(mDurationLabel);
  mFunctionCtrlBar->addWidget(mVolumeWid, 2);
  mFunctionCtrlBar->addSeparator();
  mFunctionCtrlBar->addWidget(mBasicModeBtn);
  mFunctionCtrlBar->addWidget(mFullScreenBtn);

  mVideoWidget = new QVideoWidget{this};
  mPauseShieldButton = new QToolButton{this};
  mPauseShieldButton->setDefaultAction(mPauseAct);
  mPauseShieldButton->setMinimumSize(64, 64);
  mPauseShieldButton->setAutoRaise(true);
  mPauseShieldButton->raise();

  mLeftLayout = new QVBoxLayout{this};
  mLeftLayout->addWidget(mVideoWidget);
  // mLeftLayout->addWidget(mFunctionCtrlBar);
  mLeftLayout->setContentsMargins(0, 0, 0, 0);

  mPlayer = new QMediaPlayer;
  mPlayer->setVolume(mVolumeWid->volumeVal());
  mPlayer->setMuted(mVolumeWid->isMuted());
  mPlayer->setVideoOutput(mVideoWidget);

  subscribe();
  setWindowIcon(QIcon{":/VideoPlayer/VIDEO_PLAYER_BASIC"});
  setWindowTitle("Basic Video Player");
  mFunctionCtrlBar->move(0, 0);
  mFunctionCtrlBar->raise();
}

BasicVideoView::~BasicVideoView() {
  StopPlay();
  if (mPlayer != nullptr) {
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_PLAY_SELECT_INSTANTLY.name, mPlayInstantlyBtn->isChecked());

    mPlayer->disconnect();
    mPlayer->setVideoOutput((QVideoWidget *) nullptr);
  }
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
  connect(mFullScreenBtn, &QToolButton::toggled, this, &BasicVideoView::reqFullscreenModeChange);
  connect(mBasicModeBtn, &QToolButton::toggled, this, &BasicVideoView::reqFunctionModeChange);

  mVideoWidget->installEventFilter(this);
}

bool BasicVideoView::PlayAVideo(const QString &filePath, bool forcePlayInstantly) {
  if (!QFile::exists(filePath)) {
    return false;
  }
  mPlayer->setMedia(QUrl::fromLocalFile(filePath));
  if (isAutoPlay() || forcePlayInstantly) {
    mPlayer->play();
  } else {
    mPauseAct->setChecked(true);
  }
  return true;
}

void BasicVideoView::StopPlay() {
  if (mPlayer != nullptr) {
    mPlayer->stop();
  }
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
    movePauseBtnToCenter();
    mPauseShieldButton->show();
  } else {
    mPauseShieldButton->hide();
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

bool BasicVideoView::eventFilter(QObject *watched, QEvent *event) {
  if (watched == mVideoWidget && event->type() == QEvent::MouseButtonPress) {
    const auto *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() == Qt::LeftButton) {
      const bool bNewPause = !mPauseAct->isChecked();
      mPauseAct->setChecked(bNewPause);
      emit mPauseAct->toggled(bNewPause);
      return true;
    }
  }
  return QObject::eventFilter(watched, event);
}

void BasicVideoView::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key::Key_Space) {
    const bool bNewPause = !mPauseAct->isChecked();
    mPauseAct->setChecked(bNewPause);
    event->setAccepted(true);
    return;
  } else if (event->key() == Qt::Key::Key_Escape) {
    if (mFullScreenBtn->isChecked()) {
      mFullScreenBtn->setChecked(false);
      event->setAccepted(true);
      return;
    }
  }
  QWidget::keyPressEvent(event);
}

void BasicVideoView::resizeEvent(QResizeEvent *e) {
  if (mPauseShieldButton == nullptr) {
    return;
  }
  if (mPauseShieldButton->isVisible()) {
    movePauseBtnToCenter();
  }
}

void BasicVideoView::movePauseBtnToCenter() {
  mPauseShieldButton->move(mVideoWidget->width() / 2 - mPauseShieldButton->width() / 2, //
                     mVideoWidget->height() / 2 - mPauseShieldButton->height() / 2);
}
