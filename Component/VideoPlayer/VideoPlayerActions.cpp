#include "VideoPlayerActions.h"
#include "Configuration.h"
#include "VideoPlayerKey.h"
#include "PublicMacro.h"

constexpr QMediaPlaylist::PlaybackMode VideoPlayerActions::DEFAULT_PLAYBACK_MODE;

VideoPlayerActions& VideoPlayerActions::GetInst() {
  static VideoPlayerActions inst;
  return inst;
}

VideoPlayerActions::VideoPlayerActions(QObject* parent)
  : QObject{parent} {
  mPlaybackTrigger_MANUAL = new QAction{QIcon{":/VideoPlayer/PLAY_TRIGGER_MANUAL"}, tr("Manual play"), this};
  mPlaybackTrigger_MANUAL->setCheckable(true);
  mPlaybackTrigger_AUTO = new QAction{QIcon{":/VideoPlayer/PLAY_TRIGGER_AUTO"}, tr("Auto play"), this};
  mPlaybackTrigger_AUTO->setCheckable(true);
  mPlaybackTrigger_DISABLED = new QAction{QIcon{":/VideoPlayer/PLAY_TRIGGER_DISABLED"}, tr("Disabled play"), this};
  mPlaybackTrigger_DISABLED->setCheckable(true);
  {
    using namespace VideoPlayTool;
    mPlaybackTriggerIntAction.init({{mPlaybackTrigger_MANUAL, PlaybackTriggerMode::MANUAL}, //
                                    {mPlaybackTrigger_AUTO, PlaybackTriggerMode::AUTO},     //
                                    {mPlaybackTrigger_DISABLED, PlaybackTriggerMode::DISABLED}},
                                   DEFAULT_PLAYBACK_TRIGGER_MODE,
                                   QActionGroup::ExclusionPolicy::Exclusive);
    const int playbackTriggerModeInt = getConfig(VideoPlayerKey::PLAYBACK_TRIGGER_MODE).toInt();
    const PlaybackTriggerMode initPlaybackTriggerMode = mPlaybackTriggerIntAction.intVal2Enum(playbackTriggerModeInt);
    mPlaybackTriggerIntAction.setCheckedIfActionExist(initPlaybackTriggerMode);
  }

  mPlaybackMode_CurrentItemOnce = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_CURRENT_ITEM_ONCE"}, tr("current item once"), this};
  mPlaybackMode_CurrentItemOnce->setCheckable(true);
  mPlaybackMode_CurrentItemInLoop = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_CURRENT_ITEM_IN_LOOP"}, tr("current item in loop"), this};
  mPlaybackMode_CurrentItemInLoop->setCheckable(true);
  mPlaybackMode_Sequential = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_SEQUENTIAL"}, tr("sequential"), this};
  mPlaybackMode_Sequential->setCheckable(true);
  mPlaybackMode_Loop = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_LOOP"}, tr("loop"), this};
  mPlaybackMode_Loop->setCheckable(true);
  mPlaybackMode_Random = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_RANDOM"}, tr("random"), this};
  mPlaybackMode_Random->setCheckable(true);

  {
    mPlaybackModeIntAction.init({{mPlaybackMode_CurrentItemOnce, QMediaPlaylist::PlaybackMode::CurrentItemOnce},     //
                                 {mPlaybackMode_CurrentItemInLoop, QMediaPlaylist::PlaybackMode::CurrentItemInLoop}, //
                                 {mPlaybackMode_Sequential, QMediaPlaylist::PlaybackMode::Sequential},
                                 {mPlaybackMode_Loop, QMediaPlaylist::PlaybackMode::Loop},
                                 {mPlaybackMode_Random, QMediaPlaylist::PlaybackMode::Random}},
                                DEFAULT_PLAYBACK_MODE,
                                QActionGroup::ExclusionPolicy::Exclusive);
    const int playbackModeInt = getConfig(VideoPlayerKey::PLAYBACK_MODE).toInt();
    const QMediaPlaylist::PlaybackMode initPlaybackMode = mPlaybackModeIntAction.intVal2Enum(playbackModeInt);
    mPlaybackModeIntAction.setCheckedIfActionExist(initPlaybackMode);
  }

  mVolumePlus = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VOLUME_PLUS"}, tr("+Volume"), this};
  mVolumeMinus = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VOLUME_MINUS"}, tr("-Volume"), this};
  mVolumeWid = new VolumeWidget{QBoxLayout::Direction::LeftToRight, nullptr}; // 加入到布局后, 所有权移交给布局所在widget

  connect(mPlaybackModeIntAction.getActionGroup(),
          &QActionGroup::triggered, //
          this,
          &VideoPlayerActions::onPlaybackModeTriggered);
  connect(mPlaybackTriggerIntAction.getActionGroup(),
          &QActionGroup::triggered, //
          this,
          &VideoPlayerActions::onPlaybackTriggerModeTriggered);
}

MenuToolButton* VideoPlayerActions::GetPlaybackModeMenuToolButton(QWidget* notNullParent) const {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  MenuToolButton* playbackModeToolButton = new (std::nothrow) MenuToolButton{mPlaybackModeIntAction.getActionEnumAscendingList(),
                                                                             QToolButton::ToolButtonPopupMode::InstantPopup,
                                                                             Qt::ToolButtonStyle::ToolButtonTextBesideIcon,
                                                                             IMAGE_SIZE::TABS_ICON_IN_MENU_16,
                                                                             notNullParent};
  playbackModeToolButton->SetCaption(QIcon{""}, tr("Playback Mode"), "Change Playback Mode");
  playbackModeToolButton->InitDefaultActionFromQSetting(VideoPlayerKey::PLAYBACK_MODE, true);
  return playbackModeToolButton;
}

MenuToolButton* VideoPlayerActions::GetPlaybackTriggerModelMenuToolButton(QWidget* notNullParent) const {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  MenuToolButton* playbackTriggerModeToolButton = new (std::nothrow) MenuToolButton{mPlaybackTriggerIntAction.getActionEnumAscendingList(),
                                                                                    QToolButton::ToolButtonPopupMode::InstantPopup,
                                                                                    Qt::ToolButtonStyle::ToolButtonTextBesideIcon,
                                                                                    IMAGE_SIZE::TABS_ICON_IN_MENU_16,
                                                                                    notNullParent};
  playbackTriggerModeToolButton->SetCaption(QIcon{":/VideoPlayer/PLAY_TRIGGER_MODE"}, tr("Play Trigger Mode"), "Change Playback Trigger Mode");
  playbackTriggerModeToolButton->InitDefaultActionFromQSetting(VideoPlayerKey::PLAYBACK_TRIGGER_MODE, true);
  return playbackTriggerModeToolButton;
}

VolumeWidget* VideoPlayerActions::GetInitedVolumeWid(QWidget* notNullParent) {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  if (mVolumeWid->parent() == nullptr) {
    mVolumeWid->setParent(notNullParent);
  }
  return mVolumeWid;
}

QMediaPlaylist::PlaybackMode VideoPlayerActions::GetPlaybackMode() const {
  return mPlaybackModeIntAction.curVal();
}

VideoPlayTool::PlaybackTriggerMode VideoPlayerActions::GetPlaybackTriggerMode() const {
  return mPlaybackTriggerIntAction.curVal();
}

void VideoPlayerActions::onPlaybackModeTriggered(const QAction* newPlaybackModeAct) {
  const QMediaPlaylist::PlaybackMode newPlaybackMode = mPlaybackModeIntAction.act2Enum(newPlaybackModeAct);
  emit playbackModeChanged(newPlaybackMode);
}

void VideoPlayerActions::onPlaybackTriggerModeTriggered(const QAction* newPlaybackTriggerModeAct) {
  const VideoPlayTool::PlaybackTriggerMode newPlaybackTriggerMode = mPlaybackTriggerIntAction.act2Enum(newPlaybackTriggerModeAct);
  emit playbackTriggerModeChanged(newPlaybackTriggerMode);
}
