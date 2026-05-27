#include "InteractiveVideoWidget.h"
#include "RateActions.h"
#include "DualIconCheckableAction.h"
#include "VideoPlayerActions.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include "SystemPath.h"
#include "Logger.h"
#include "VideoPlayerKey.h"
#include "PathKey.h"
#include "Configuration.h"
#include "ToolBarWidget.h"
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QFileDialog>

InteractiveVideoWidget::InteractiveVideoWidget(bool bBasicMode, QWidget* parent) : QVideoWidget{parent} {
  mGrabFrame = new QAction{QIcon{":/VideoPlayer/GRAB_FRAME"}, tr("Grab a Frame"), this};
  mGrabFrame->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key::Key_E));
  mGrabFrame->setToolTip(QString{"<b>%1 (%2)</b><br/>Grab the frame at current position"}//
                               .arg(mGrabFrame->text(), mGrabFrame->shortcut().toString()));

  mPauseAct = DualIconCheckableAction::CreatePauseAction(this, true);

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

  mOpenInSystemApplication = new (std::nothrow) QAction{QIcon{":img/SYSTEM_APPLICATION_VIDEO"}, tr("Open in System Application"), this};

  const bool bShowVideoList{Configuration().value("VideoView/ShowVideoList", true).toBool()};
  mShowVideoList = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VIDEO_LIST"}, tr("show list"), this};
  mShowVideoList->setCheckable(true);
  mShowVideoList->setChecked(bShowVideoList);

  mBasicModeAct = new QAction(QIcon{":/VideoPlayer/VIDEO_PLAYER_BASIC"}, tr("basic mode"), this);
  mBasicModeAct->setCheckable(true);
  mBasicModeAct->setChecked(bBasicMode);
  mBasicModeAct->setToolTip("Compact interface with only essential functions when enabled, otherwise standard interface with all features");

  mHideToolBarAct = DualIconCheckableAction::CreateHideToolBarAction(this, false);
  mFullScreenAct = DualIconCheckableAction::CreateFullScreenAction(this, false);

  mSelectVideoFileAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/OPEN_A_VIDEO"}, tr("select video"), this};
  mSelectVideoFolder = new (std::nothrow) QAction{QIcon{":/VideoPlayer/OPEN_A_FOLDER"}, tr("select a folder"), this};
  mDisableAutoHideToolBar = new (std::nothrow) QAction{QIcon{":/VideoPlayer/DISABLE_AUTO_HIDE"}, tr("disable auto hide"), this};
  mDisableAutoHideToolBar->setCheckable(true);
  const bool isAutoHideDisabled = getConfig(VideoPlayerKey::AUTO_HIDE_TOOLBAR).toBool();
  mDisableAutoHideToolBar->setChecked(isAutoHideDisabled);

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
  mContextMenu->addAction(mGrabFrame);
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
  mContextMenu->addAction(mSelectVideoFileAct);
  mContextMenu->addAction(mSelectVideoFolder);
  mContextMenu->addSeparator();
  mContextMenu->addAction(mBasicModeAct);
  mContextMenu->addAction(mShowFrames);
  mContextMenu->addAction(mDisableAutoHideToolBar);

  mLongTimeNoClickTimer.setSingleShot(true);
  mLongTimeNoClickTimer.setInterval(10 * 1000); // 10s

  connect(&mLongTimeNoClickTimer, &QTimer::timeout, this, &InteractiveVideoWidget::onLongTimeNoEventHappen);
  connect(mFullScreenAct, &QAction::toggled, this, &InteractiveVideoWidget::onFullScreenActionToggled);
  connect(mSelectVideoFileAct, &QAction::triggered, this, &InteractiveVideoWidget::onSelectAFile);
  connect(mSelectVideoFolder, &QAction::triggered, this, &InteractiveVideoWidget::onSelectAFolder);
  connect(mDisableAutoHideToolBar, &QAction::toggled, &mLongTimeNoClickTimer, &QTimer::stop);

  setFocusPolicy(Qt::FocusPolicy::ClickFocus);
}

InteractiveVideoWidget::~InteractiveVideoWidget() {
  setConfig(VideoPlayerKey::AUTO_HIDE_TOOLBAR, isAutoHideToolBarDisabled());
}

QWidget* InteractiveVideoWidget::GetExtendedFunctionCtrlBar(QWidget* notNullParent) const {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);

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
  extendedFunctionCtrlBar->addAction(mShowFrames);
  extendedFunctionCtrlBar->addStretch();
  extendedFunctionCtrlBar->addAction(mDisableAutoHideToolBar);
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
  tryStartAutoHideTimer();
}

void InteractiveVideoWidget::onQuitFullScreenMode() {
  mLongTimeNoClickTimer.stop();
  changeAllToolbarVisibility(true);
}

bool InteractiveVideoWidget::onSelectAFile() {
  QString defaultOpenPathLocatedIn = getConfig(PathKey::VIDEO_PLAYER_OPEN_PATH).toString();
  if (!QFile::exists(defaultOpenPathLocatedIn)) {
    defaultOpenPathLocatedIn = SystemPath::HomePath();
  }
  static const QString filterStr = "Video Files (" + TYPE_FILTER::VIDEO_TYPE_SET.join(" ") + ")";
  QString fileSelected = QFileDialog::getOpenFileName(this,
                                                      "Select a video file",  //
                                                      defaultOpenPathLocatedIn, filterStr);
  if (fileSelected.isEmpty()) {
    return false;
  }
  setConfig(PathKey::VIDEO_PLAYER_OPEN_PATH, PathTool::absolutePath(fileSelected));
  emit newFileSelectedByUser(fileSelected, true);
  return true;
}

bool InteractiveVideoWidget::onSelectAFolder() {
  QString defaultOpenPathLocatedIn = getConfig(PathKey::VIDEO_PLAYER_OPEN_PATH).toString();
  if (!QFile::exists(defaultOpenPathLocatedIn)) {
    defaultOpenPathLocatedIn = SystemPath::HomePath();
  }
  const QString dirSelected = QFileDialog::getExistingDirectory(this,
                                                                "Select a media folder",  //
                                                                defaultOpenPathLocatedIn);
  if (dirSelected.isEmpty()) {
    return false;
  }
  setConfig(PathKey::VIDEO_PLAYER_OPEN_PATH, dirSelected);
  emit newFolderSelectedChangedByUser(dirSelected, true);
  return true;
}

void InteractiveVideoWidget::tryStartAutoHideTimer() {
  if (!isAutoHideToolBarDisabled()) {
    mLongTimeNoClickTimer.start();
  }
}

bool InteractiveVideoWidget::updatePauseActionState(bool bPauseChecked) {
  if (mPauseAct->isChecked() == bPauseChecked) {
    return false;
  }
  mPauseAct->setChecked(bPauseChecked);
  return true;
}

void InteractiveVideoWidget::mousePressEvent(QMouseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
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
  CHECK_NULLPTR_RETURN_VOID(event);
  const Qt::KeyboardModifiers modifier = event->modifiers();
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
      VideoPlayerActions::GetInst().mVolumePlus->trigger();
      break;
    }
    case Qt::Key::Key_Down: {
      VideoPlayerActions::GetInst().mVolumeMinus->trigger();
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
    case Qt::Key::Key_Return: {
      if (modifier == Qt::KeyboardModifier::AltModifier) {
        mHideToolBarAct->toggle();
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

void InteractiveVideoWidget::onMouseRightClickEventHappend() {
  // 只修改进度条所在的toolbar的可见性, 不修改列表的可见性
  if (mHideToolBarAct->isChecked()) {
    mHideToolBarAct->toggle();
    emit layoutVisibilityChanged();
  }
  if (isVideoFullScreen()) {
    // 全屏模式下, 重新开始计时
    tryStartAutoHideTimer();
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
  if (isClickPressHappend()) {
    // 超时后, 发现之前有鼠标/键盘事件, 重新启动, 实现延迟超时
    clearClickPressHappend();
    tryStartAutoHideTimer();
    return;
  }
  // 全屏模式下 hide everything except video itself
  if (isVideoFullScreen()) {
    changeAllToolbarVisibility(false);
    GetFocusCore(this);
  }
}
