#include "InteractiveVideoWidget.h"
#include "DualIconCheckableAction.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "Logger.h"
#include "MemoryKey.h"
#include <QMouseEvent>
#include <QContextMenuEvent>

constexpr QMediaPlaylist::PlaybackMode InteractiveVideoWidget::DEFAULT_PLAYBACK_MODE;
constexpr int InteractiveVideoWidget::TIMER_INTERVAL;

InteractiveVideoWidget::InteractiveVideoWidget(QWidget* parent)
  : QVideoWidget{parent} {
  const bool bPlayInstant
      = Configuration().value(MemoryKey::VIDEO_PLAYER_PLAY_SELECT_INSTANTLY.name, MemoryKey::VIDEO_PLAYER_PLAY_SELECT_INSTANTLY.v).toBool();

  mPlayInstantlyAct = new QAction{QIcon{""}, tr("instant"), this};
  mPlayInstantlyAct->setCheckable(true);
  mPlayInstantlyAct->setChecked(bPlayInstant);
  mPlayInstantlyAct->setToolTip("Automatically play selected file instantly in current view when enabled.");

  mPauseAct = DualIconCheckableAction::CreatePauseAction(this, !bPlayInstant);

  mStopAct = new QAction{QIcon{":/VideoPlayer/STOP_VIDEO"}, tr("stop"), this};
  mStopAct->setToolTip("Stop playing and move position to time 0.");
  mSeekBackwardAct = new QAction{QIcon{":/VideoPlayer/SEEK_BACKWARD"}, tr("seek backward 10s"), this};
  mSeekForwardAct = new QAction{QIcon{":/VideoPlayer/SEEK_FORWARD"}, tr("seek forward 10s"), this};
  mSeekBackwardHotAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/JUMP_LAST_HOT_SCENE"}, tr("previous hot point"), this};
  mSeekForwardHotAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/JUMP_NEXT_HOT_SCENE"}, tr("next hot point"), this};
  mPlayPrevAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VIDEO_PREVIOUS"}, tr("play previous video"), this};
  mPlayNextAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VIDEO_NEXT"}, tr("play next video"), this};

  mShowFrames = new (std::nothrow) QAction{QIcon{""}, tr("show frames"), this};
  mShowFrames->setCheckable(true);
  mShowFrames->setChecked(false);

  mShowVideoList = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VIDEO_LIST"}, tr("show list"), this};
  mShowVideoList->setCheckable(true);
  mShowVideoList->setChecked(true);

  mPlaybackMode_CurrentItemOnce = new (std::nothrow)
      QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_CURRENT_ITEM_ONCE"}, tr("current item once"), this};
  mPlaybackMode_CurrentItemOnce->setCheckable(true);
  mPlaybackMode_CurrentItemInLoop = new (std::nothrow)
      QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_CURRENT_ITEM_IN_LOOP"}, tr("current item in loop"), this};
  mPlaybackMode_CurrentItemInLoop->setCheckable(true);
  mPlaybackMode_Sequential = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_SEQUENTIAL"}, tr("sequential"), this};
  mPlaybackMode_Sequential->setCheckable(true);
  mPlaybackMode_Loop = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_LOOP"}, tr("loop"), this};
  mPlaybackMode_Loop->setCheckable(true);
  mPlaybackMode_Random = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_RANDOM"}, tr("random"), this};
  mPlaybackMode_Random->setCheckable(true);

  mPlaybackIntAction.init({{mPlaybackMode_CurrentItemOnce, QMediaPlaylist::PlaybackMode::CurrentItemOnce},     //
                           {mPlaybackMode_CurrentItemInLoop, QMediaPlaylist::PlaybackMode::CurrentItemInLoop}, //
                           {mPlaybackMode_Sequential, QMediaPlaylist::PlaybackMode::Sequential},
                           {mPlaybackMode_Loop, QMediaPlaylist::PlaybackMode::Loop},
                           {mPlaybackMode_Random, QMediaPlaylist::PlaybackMode::Random}},
                          DEFAULT_PLAYBACK_MODE,
                          QActionGroup::ExclusionPolicy::Exclusive);
  const int playbackModeInt
      = Configuration().value(MemoryKey::VIDEO_PLAYER_PLAYBACK_MODE.name, MemoryKey::VIDEO_PLAYER_PLAYBACK_MODE.v).toInt();
  QMediaPlaylist::PlaybackMode initPlaybackMode = mPlaybackIntAction.intVal2Enum(playbackModeInt);
  mPlaybackIntAction.setCheckedIfActionExist(initPlaybackMode);

  mBasicModeAct = new QAction(QIcon{":/VideoPlayer/VIDEO_PLAYER_BASIC"}, tr("basic mode"), this);
  mBasicModeAct->setCheckable(true);

  mHideToolBarAct = DualIconCheckableAction::CreateHideToolBarAction(this, false);
  mFullScreenAct = DualIconCheckableAction::CreateFullScreenAction(this, false);

  mSelectVideoFileAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/OPEN_A_VIDEO"}, tr("select video"), this};
  mSelectVideoFolder = new (std::nothrow) QAction{QIcon{":/VideoPlayer/OPEN_A_FOLDER"}, tr("select a folder"), this};

  mPlaybackModeMenu = new QMenu{tr("Playerback mode"), this};
  mPlaybackModeMenu->setToolTipsVisible(true);
  mPlaybackModeMenu->addActions(mPlaybackIntAction.getActionEnumAscendingList());

  mContextMenu = new QMenu{tr("Player Menu"), this};
  mContextMenu->setToolTipsVisible(true);
  mContextMenu->addAction(mFullScreenAct);
  mContextMenu->addAction(mHideToolBarAct);
  mContextMenu->addAction(mShowVideoList);
  mContextMenu->addSeparator();
  mContextMenu->addAction(mPauseAct);
  mContextMenu->addAction(mStopAct);
  mContextMenu->addAction(mPlayInstantlyAct);
  mContextMenu->addSeparator();
  mContextMenu->addAction(mSeekBackwardAct);
  mContextMenu->addAction(mSeekForwardAct);
  mContextMenu->addAction(mSeekBackwardHotAct);
  mContextMenu->addAction(mSeekForwardHotAct);
  mContextMenu->addSeparator();
  mContextMenu->addAction(mPlayPrevAct);
  mContextMenu->addAction(mPlayNextAct);
  mContextMenu->addSeparator();
  mContextMenu->addAction(mSelectVideoFileAct);
  mContextMenu->addAction(mSelectVideoFolder);
  mContextMenu->addSeparator();
  mContextMenu->addAction(mBasicModeAct);
  mContextMenu->addAction(mShowFrames);
  mContextMenu->addSeparator();
  mContextMenu->addMenu(mPlaybackModeMenu);

  mLongTimeNoClickTimer.setSingleShot(true);
  mLongTimeNoClickTimer.setInterval(TIMER_INTERVAL);

  connect(&mLongTimeNoClickTimer, &QTimer::timeout, this, &InteractiveVideoWidget::onLongTimeNoEventHappen);
  connect(mPlaybackIntAction.getActionGroup(), &QActionGroup::triggered, this, &InteractiveVideoWidget::onPlaybackModeTriggered);

  setFocusPolicy(Qt::FocusPolicy::ClickFocus);
}

InteractiveVideoWidget::~InteractiveVideoWidget() {
  Configuration().setValue(MemoryKey::VIDEO_PLAYER_PLAY_SELECT_INSTANTLY.name, isAutoPlay());
  Configuration().setValue(MemoryKey::VIDEO_PLAYER_PLAYBACK_MODE.name, mPlaybackIntAction.curVal());
}

MenuToolButton* InteractiveVideoWidget::GetPlaybackModelMenuToolButton(QWidget* notNullParent) const {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  MenuToolButton* playbackModeToolButton = new (std::nothrow) MenuToolButton{mPlaybackIntAction.getActionEnumAscendingList(),
                                                                             QToolButton::ToolButtonPopupMode::InstantPopup,
                                                                             Qt::ToolButtonStyle::ToolButtonTextBesideIcon,
                                                                             IMAGE_SIZE::TABS_ICON_IN_MENU_48,
                                                                             notNullParent};
  playbackModeToolButton->SetCaption(QIcon{""}, tr("Playback Mode"), "Change Playback Mode");
  return playbackModeToolButton;
}

void InteractiveVideoWidget::onIntoFullScreenMode() {
  setFocus();
  mLongTimeNoClickTimer.start();
}

void InteractiveVideoWidget::onQuitFullScreenMode() {
  mLongTimeNoClickTimer.stop();
  changeAllToolbarVisibility(true);
}

QMediaPlaylist::PlaybackMode InteractiveVideoWidget::GetPlaybackMode() const {
  return mPlaybackIntAction.curVal();
}

void InteractiveVideoWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::MouseButton::LeftButton) {
    mPauseAct->toggle();
    event->setAccepted(true);
    return;
  } else {
    if (mFullScreenAct->isChecked()) {
      // 全屏模式下, 左键外的其它按键->工具栏可见
      onMouseEventHappend();
    }
    event->setAccepted(true);
    return;
  }
  QVideoWidget::mousePressEvent(event);
}

void InteractiveVideoWidget::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key::Key_Left: {
      mSeekBackwardAct->trigger();
      break;
    }
    case Qt::Key::Key_Right: {
      mSeekForwardAct->trigger();
      break;
    }
    case Qt::Key::Key_Up: {
      LOG_OK_NP("volume", "+1");
      break;
    }
    case Qt::Key::Key_Down: {
      LOG_OK_NP("volume", "-1");
      break;
    }
    case Qt::Key::Key_PageUp: {
      mPlayPrevAct->trigger();
      break;
    }
    case Qt::Key::Key_PageDown: {
      mPlayNextAct->trigger();
      break;
    }
    case Qt::Key::Key_Space: {
      mPauseAct->toggle();
      break;
    }
    case Qt::Key::Key_Escape: {
      if (mFullScreenAct->isChecked()) {
        mFullScreenAct->setChecked(false);
      }
      break;
    }
    default: {
      QVideoWidget::keyPressEvent(event);
      return;
    }
  }
  event->setAccepted(true);
}

void InteractiveVideoWidget::contextMenuEvent(QContextMenuEvent* event) {
  mContextMenu->exec(event->globalPos());
  event->accept();
}

void InteractiveVideoWidget::onPlaybackModeTriggered(const QAction* newPlaybackModeAct) {
  const QMediaPlaylist::PlaybackMode newPlaybackMode = mPlaybackIntAction.act2Enum(newPlaybackModeAct);
  emit playbackModeChanged(newPlaybackMode);
}

void InteractiveVideoWidget::onMouseEventHappend() {
  // 全屏模式下 show everything, 重新开始计时
  if (mFullScreenAct->isChecked()) {
    changeAllToolbarVisibility(true);
    mLongTimeNoClickTimer.start();
  }
}

void InteractiveVideoWidget::changeAllToolbarVisibility(bool visible) {
  bool bAnyThingChanged{false};
  if (mHideToolBarAct->isChecked() == visible) {
    mHideToolBarAct->toggle();
    bAnyThingChanged = true;
  }
  if (mShowVideoList->isChecked() != visible) {
    mShowVideoList->toggle();
    bAnyThingChanged = true;
  }
  if (bAnyThingChanged) {
    emit layoutVisibilityChanged();
  }
}

void InteractiveVideoWidget::onLongTimeNoEventHappen() {
  // 全屏模式下 hide everything except video itself
  if (mFullScreenAct->isChecked()) {
    changeAllToolbarVisibility(false);
  }
}
