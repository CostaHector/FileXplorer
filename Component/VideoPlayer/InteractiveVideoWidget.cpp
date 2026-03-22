#include "InteractiveVideoWidget.h"
#include "RateActions.h"
#include "DualIconCheckableAction.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include "Logger.h"
#include "MemoryKey.h"
#include "ToolBarWidget.h"
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QFileDialog>

constexpr QMediaPlaylist::PlaybackMode InteractiveVideoWidget::DEFAULT_PLAYBACK_MODE;
constexpr int InteractiveVideoWidget::TIMER_INTERVAL;

InteractiveVideoWidget::InteractiveVideoWidget(bool bBasicMode, QWidget* parent) : QVideoWidget{parent} {
  mPlaybackTrigger_MANUAL = new QAction{QIcon{":/VideoPlayer/PLAY_TRIGGER_MANUAL"}, tr("Manual play"), this};
  mPlaybackTrigger_MANUAL->setCheckable(true);
  mPlaybackTrigger_AUTO = new QAction{QIcon{":/VideoPlayer/PLAY_TRIGGER_AUTO"}, tr("Auto play"), this};
  mPlaybackTrigger_AUTO->setCheckable(true);
  mPlaybackTrigger_DISABLED = new QAction{QIcon{":/VideoPlayer/PLAY_TRIGGER_DISABLED"}, tr("Disabled play"), this};
  mPlaybackTrigger_DISABLED->setCheckable(true);

  mPauseAct = DualIconCheckableAction::CreatePauseAction(this, true);

  mStopAct = new QAction{QIcon{":/VideoPlayer/STOP_VIDEO"}, tr("stop"), this};
  mStopAct->setToolTip("Stop playing and move position to time 0.");
  mSeekBackwardAct = new QAction{QIcon{":/VideoPlayer/SEEK_BACKWARD"}, tr("seek backward 10s"), this};
  mSeekForwardAct = new QAction{QIcon{":/VideoPlayer/SEEK_FORWARD"}, tr("seek forward 10s"), this};
  mSeekBackwardHotAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/JUMP_LAST_HOT_SCENE"}, tr("previous hot point"), this};
  mSeekForwardHotAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/JUMP_NEXT_HOT_SCENE"}, tr("next hot point"), this};
  mPlayPrevAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VIDEO_PREVIOUS"}, tr("play previous video"), this};
  mPlayNextAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VIDEO_NEXT"}, tr("play next video"), this};
  mVolumePlus = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VOLUME_PLUS"}, tr("+Volume"), this};
  mVolumeMinus = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VOLUME_MINUS"}, tr("-Volume"), this};

  mShowFrames = new (std::nothrow) QAction{QIcon{""}, tr("show frames"), this};
  mShowFrames->setCheckable(true);
  mShowFrames->setChecked(false);

  mOpenInSystemApplication = new (std::nothrow) QAction{QIcon{":img/SYSTEM_APPLICATION_VIDEO"}, tr("open in system application"), this};

  mShowVideoList = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VIDEO_LIST"}, tr("show list"), this};
  mShowVideoList->setCheckable(true);
  mShowVideoList->setChecked(true);

  mPlaybackMode_CurrentItemOnce = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_CURRENT_ITEM_ONCE"}, tr("current item once"), this};
  mPlaybackMode_CurrentItemOnce->setCheckable(true);
  mPlaybackMode_CurrentItemInLoop =
      new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_CURRENT_ITEM_IN_LOOP"}, tr("current item in loop"), this};
  mPlaybackMode_CurrentItemInLoop->setCheckable(true);
  mPlaybackMode_Sequential = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_SEQUENTIAL"}, tr("sequential"), this};
  mPlaybackMode_Sequential->setCheckable(true);
  mPlaybackMode_Loop = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_LOOP"}, tr("loop"), this};
  mPlaybackMode_Loop->setCheckable(true);
  mPlaybackMode_Random = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_RANDOM"}, tr("random"), this};
  mPlaybackMode_Random->setCheckable(true);

  mPlaybackModeIntAction.init({{mPlaybackMode_CurrentItemOnce, QMediaPlaylist::PlaybackMode::CurrentItemOnce},      //
                               {mPlaybackMode_CurrentItemInLoop, QMediaPlaylist::PlaybackMode::CurrentItemInLoop},  //
                               {mPlaybackMode_Sequential, QMediaPlaylist::PlaybackMode::Sequential},
                               {mPlaybackMode_Loop, QMediaPlaylist::PlaybackMode::Loop},
                               {mPlaybackMode_Random, QMediaPlaylist::PlaybackMode::Random}},
                              DEFAULT_PLAYBACK_MODE, QActionGroup::ExclusionPolicy::Exclusive);
  const int playbackModeInt = Configuration().value(MemoryKey::VIDEO_PLAYER_PLAYBACK_MODE.name, MemoryKey::VIDEO_PLAYER_PLAYBACK_MODE.v).toInt();
  const QMediaPlaylist::PlaybackMode initPlaybackMode = mPlaybackModeIntAction.intVal2Enum(playbackModeInt);
  mPlaybackModeIntAction.setCheckedIfActionExist(initPlaybackMode);

  {
    using namespace VideoPlayTool;
    mPlaybackTriggerIntAction.init({{mPlaybackTrigger_MANUAL, PlaybackTriggerMode::MANUAL},  //
                                    {mPlaybackTrigger_AUTO, PlaybackTriggerMode::AUTO},      //
                                    {mPlaybackTrigger_DISABLED, PlaybackTriggerMode::DISABLED}},
                                   DEFAULT_PLAYBACK_TRIGGER_MODE, QActionGroup::ExclusionPolicy::Exclusive);
    const int playbackTriggerModeInt =
        Configuration().value(MemoryKey::VIDEO_PLAYER_PLAYBACK_TRIGGER_MODE.name, MemoryKey::VIDEO_PLAYER_PLAYBACK_TRIGGER_MODE.v).toInt();
    const PlaybackTriggerMode initPlaybackTriggerMode = mPlaybackTriggerIntAction.intVal2Enum(playbackTriggerModeInt);
    mPlaybackTriggerIntAction.setCheckedIfActionExist(initPlaybackTriggerMode);
  }

  mBasicModeAct = new QAction(QIcon{":/VideoPlayer/VIDEO_PLAYER_BASIC"}, tr("basic mode"), this);
  mBasicModeAct->setCheckable(true);
  mBasicModeAct->setChecked(bBasicMode);
  mBasicModeAct->setToolTip("Compact interface with only essential functions when enabled, otherwise standard interface with all features");

  mHideToolBarAct = DualIconCheckableAction::CreateHideToolBarAction(this, false);
  mFullScreenAct = DualIconCheckableAction::CreateFullScreenAction(this, false);

  mSelectVideoFileAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/OPEN_A_VIDEO"}, tr("select video"), this};
  mSelectVideoFolder = new (std::nothrow) QAction{QIcon{":/VideoPlayer/OPEN_A_FOLDER"}, tr("select a folder"), this};

  mPlaybackModeMenu = new QMenu{tr("Playerback mode"), this};
  mPlaybackModeMenu->setToolTipsVisible(true);
  mPlaybackModeMenu->addActions(mPlaybackModeIntAction.getActionEnumAscendingList());

  mRateActions = new (std::nothrow) RateActions(this);
  mRateMenu = mRateActions->GetRateMenu(this);

  mContextMenu = new QMenu{tr("Player Menu"), this};
  mContextMenu->setToolTipsVisible(true);
  mContextMenu->addAction(mOpenInSystemApplication);
  mContextMenu->addSeparator();
  mContextMenu->addMenu(mRateMenu);
  mContextMenu->addSeparator();
  mContextMenu->addAction(mFullScreenAct);
  mContextMenu->addAction(mHideToolBarAct);
  mContextMenu->addAction(mShowVideoList);
  mContextMenu->addSeparator();
  mContextMenu->addAction(mPauseAct);
  mContextMenu->addAction(mStopAct);
  mContextMenu->addSeparator();
  mContextMenu->addAction(mSeekBackwardAct);
  mContextMenu->addAction(mSeekForwardAct);
  mContextMenu->addAction(mSeekBackwardHotAct);
  mContextMenu->addAction(mSeekForwardHotAct);
  mContextMenu->addSeparator();
  mContextMenu->addAction(mPlayPrevAct);
  mContextMenu->addAction(mPlayNextAct);
  mContextMenu->addSeparator();
  mContextMenu->addAction(mVolumePlus);
  mContextMenu->addAction(mVolumeMinus);
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
  connect(mFullScreenAct, &QAction::toggled, this, &InteractiveVideoWidget::onFullScreenActionToggled);
  connect(mSelectVideoFileAct, &QAction::triggered, this, &InteractiveVideoWidget::onSelectAFile);
  connect(mSelectVideoFolder, &QAction::triggered, this, &InteractiveVideoWidget::onSelectAFolder);

  connect(mPlaybackModeIntAction.getActionGroup(),
          &QActionGroup::triggered,  //
          this, &InteractiveVideoWidget::onPlaybackModeTriggered);
  connect(mPlaybackTriggerIntAction.getActionGroup(),
          &QActionGroup::triggered,  //
          this, &InteractiveVideoWidget::onPlaybackTriggerModeTriggered);

  setFocusPolicy(Qt::FocusPolicy::ClickFocus);
}

MenuToolButton* InteractiveVideoWidget::GetPlaybackModelMenuToolButton(QWidget* notNullParent) const {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  MenuToolButton* playbackModeToolButton =
      new (std::nothrow) MenuToolButton{mPlaybackModeIntAction.getActionEnumAscendingList(), QToolButton::ToolButtonPopupMode::InstantPopup,
                                        Qt::ToolButtonStyle::ToolButtonTextBesideIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_16, notNullParent};
  playbackModeToolButton->SetCaption(QIcon{""}, tr("Playback Mode"), "Change Playback Mode");
  playbackModeToolButton->InitDefaultActionFromQSetting(MemoryKey::VIDEO_PLAYER_PLAYBACK_MODE, true);
  return playbackModeToolButton;
}

MenuToolButton* InteractiveVideoWidget::GetPlaybackTriggerModelMenuToolButton(QWidget* notNullParent) const {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  MenuToolButton* playbackTriggerModeToolButton =
      new (std::nothrow) MenuToolButton{mPlaybackTriggerIntAction.getActionEnumAscendingList(), QToolButton::ToolButtonPopupMode::InstantPopup,
                                        Qt::ToolButtonStyle::ToolButtonTextBesideIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_16, notNullParent};
  playbackTriggerModeToolButton->SetCaption(QIcon{":/VideoPlayer/PLAY_TRIGGER_MODE"}, tr("Play Trigger Mode"), "Change Playback Trigger Mode");
  playbackTriggerModeToolButton->InitDefaultActionFromQSetting(MemoryKey::VIDEO_PLAYER_PLAYBACK_TRIGGER_MODE, true);
  return playbackTriggerModeToolButton;
}

QWidget* InteractiveVideoWidget::GetExtendedFunctionCtrlBar(QWidget* notNullParent) const {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  MenuToolButton* playbackModeBtn = GetPlaybackModelMenuToolButton(notNullParent);
  CHECK_NULLPTR_RETURN_NULLPTR(playbackModeBtn);

  const bool bBasicModeHideIt{mBasicModeAct->isChecked()};

  ToolBarWidget* extendedFunctionCtrlBar = new (std::nothrow) ToolBarWidget{QBoxLayout::Direction::LeftToRight, notNullParent};
  CHECK_NULLPTR_RETURN_NULLPTR(extendedFunctionCtrlBar);
  if (bBasicModeHideIt) {
    extendedFunctionCtrlBar->hide();
  }
  extendedFunctionCtrlBar->addAction(mSelectVideoFolder);
  extendedFunctionCtrlBar->addSeparator();
  extendedFunctionCtrlBar->addAction(mSeekBackwardHotAct);
  extendedFunctionCtrlBar->addAction(mSeekForwardHotAct);
  extendedFunctionCtrlBar->addSeparator();
  extendedFunctionCtrlBar->addAction(mPlayPrevAct);
  extendedFunctionCtrlBar->addAction(mPlayNextAct);
  extendedFunctionCtrlBar->addSeparator();
  extendedFunctionCtrlBar->addWidget(playbackModeBtn);
  extendedFunctionCtrlBar->addAction(mShowFrames);
  extendedFunctionCtrlBar->addStretch();
  extendedFunctionCtrlBar->addAction(mShowVideoList);
  return extendedFunctionCtrlBar;
}

void InteractiveVideoWidget::onFullScreenActionToggled(bool bFullScreen) {
  emit fullScreenModeToggled(bFullScreen);
  if (bFullScreen) {
    onIntoFullScreenMode();
  } else {
    onQuitFullScreenMode();
  }
}

bool InteractiveVideoWidget::GetFocusCore(InteractiveVideoWidget* self) {
  CHECK_NULLPTR_RETURN_FALSE(self);
  self->setFocus();
  return true;
}

void InteractiveVideoWidget::onIntoFullScreenMode() {
  GetFocusCore(this);
  mLongTimeNoClickTimer.start();
}

void InteractiveVideoWidget::onQuitFullScreenMode() {
  mLongTimeNoClickTimer.stop();
  changeAllToolbarVisibility(true);
}

bool InteractiveVideoWidget::onSelectAFile() {
  QString defaultOpenPathLocatedIn =
      Configuration().value(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.v).toString();
  if (!QFile::exists(defaultOpenPathLocatedIn)) {
    defaultOpenPathLocatedIn = SystemPath::HOME_PATH();
  }
  static const QString filterStr = "Video Files (" + TYPE_FILTER::VIDEO_TYPE_SET.join(" ") + ")";
  QString fileSelected = QFileDialog::getOpenFileName(this,
                                                      "Select a video file",  //
                                                      defaultOpenPathLocatedIn, filterStr);
  if (fileSelected.isEmpty()) {
    return false;
  }
  Configuration().setValue(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, PathTool::absolutePath(fileSelected));
  emit newFileSelectedByUser(fileSelected, true);
  return true;
}

bool InteractiveVideoWidget::onSelectAFolder() {
  QString defaultOpenPathLocatedIn =
      Configuration().value(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.v).toString();
  if (!QFile::exists(defaultOpenPathLocatedIn)) {
    defaultOpenPathLocatedIn = SystemPath::HOME_PATH();
  }
  const QString dirSelected = QFileDialog::getExistingDirectory(this,
                                                                "Select a media folder",  //
                                                                defaultOpenPathLocatedIn);
  if (dirSelected.isEmpty()) {
    return false;
  }
  Configuration().setValue(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, dirSelected);
  emit newFolderSelectedChangedByUser(dirSelected, true);
  return true;
}

bool InteractiveVideoWidget::updatePauseActionState(bool bPauseChecked) {
  if (mPauseAct->isChecked() == bPauseChecked) {
    return false;
  }
  mPauseAct->setChecked(bPauseChecked);
  return true;
}

QMediaPlaylist::PlaybackMode InteractiveVideoWidget::GetPlaybackMode() const {
  return mPlaybackModeIntAction.curVal();
}

VideoPlayTool::PlaybackTriggerMode InteractiveVideoWidget::GetPlaybackTriggerMode() const {
  return mPlaybackTriggerIntAction.curVal();
}

void InteractiveVideoWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::MouseButton::LeftButton) {
    mPauseAct->toggle();
    event->setAccepted(true);
    return;
  } else if (event->button() == Qt::MouseButton::RightButton) {
    onMouseRightClickEventHappend();
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
      mVolumePlus->trigger();
      break;
    }
    case Qt::Key::Key_Down: {
      mVolumeMinus->trigger();
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
      if (isVideoFullScreen()) {
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
  CHECK_NULLPTR_RETURN_VOID(event);
  mContextMenu->exec(event->globalPos());
  event->accept();
}

void InteractiveVideoWidget::onPlaybackModeTriggered(const QAction* newPlaybackModeAct) {
  const QMediaPlaylist::PlaybackMode newPlaybackMode = mPlaybackModeIntAction.act2Enum(newPlaybackModeAct);
  emit playbackModeChanged(newPlaybackMode);
}

void InteractiveVideoWidget::onPlaybackTriggerModeTriggered(const QAction* newPlaybackTriggerModeAct) {
  const VideoPlayTool::PlaybackTriggerMode newPlaybackTriggerMode = mPlaybackTriggerIntAction.act2Enum(newPlaybackTriggerModeAct);
  emit playbackTriggerModeChanged(newPlaybackTriggerMode);
}

void InteractiveVideoWidget::onMouseRightClickEventHappend() {
  // show everything
  changeAllToolbarVisibility(true);
  if (isVideoFullScreen()) {
    // 全屏模式下, 重新开始计时
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
  if (isVideoFullScreen()) {
    changeAllToolbarVisibility(false);
    GetFocusCore(this);
  }
}
