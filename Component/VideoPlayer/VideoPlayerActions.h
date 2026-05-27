#ifndef VIDEOPLAYERACTIONS_H
#define VIDEOPLAYERACTIONS_H

#include <QAction>
#include <QObject>
#include <QMediaPlaylist>
#include "EnumIntAction.h"
#include "VideoPlayTool.h"
#include "MenuToolButton.h"
#include "VolumeWidget.h"

extern template struct EnumIntAction<QMediaPlaylist::PlaybackMode>;
extern template struct EnumIntAction<VideoPlayTool::PlaybackTriggerMode>;

class VideoPlayerActions : public QObject {
  Q_OBJECT
public:
  static VideoPlayerActions& GetInst();

  MenuToolButton* GetPlaybackModeMenuToolButton(QWidget* notNullParent) const;
  MenuToolButton* GetPlaybackTriggerModelMenuToolButton(QWidget* notNullParent) const;

  QMediaPlaylist::PlaybackMode GetPlaybackMode() const;
  VideoPlayTool::PlaybackTriggerMode GetPlaybackTriggerMode() const;

  QAction *mVolumePlus{nullptr}, *mVolumeMinus{nullptr}; // 增加音量, 减少音量

signals:
  void playbackModeChanged(QMediaPlaylist::PlaybackMode newPlaybackMode);
  void playbackTriggerModeChanged(VideoPlayTool::PlaybackTriggerMode newPlaybackTriggerMode);

private:
  explicit VideoPlayerActions(QObject* parent = nullptr);

  void onPlaybackModeTriggered(const QAction* newPlaybackModeAct);
  EnumIntAction<QMediaPlaylist::PlaybackMode> mPlaybackModeIntAction;                                                 //
  static constexpr QMediaPlaylist::PlaybackMode DEFAULT_PLAYBACK_MODE{QMediaPlaylist::PlaybackMode::CurrentItemOnce}; // 缺省时列表播放模式

  void onPlaybackTriggerModeTriggered(const QAction* newPlaybackTriggerModeAct);
  EnumIntAction<VideoPlayTool::PlaybackTriggerMode> mPlaybackTriggerIntAction; //

  QAction                         // 播放触发模式
      *mPlaybackTrigger_MANUAL,   // 播放触发模式-手动播放
      *mPlaybackTrigger_AUTO;     // 播放触发模式-自动播放

  QAction                                        // 播放模式
      *mPlaybackMode_CurrentItemOnce{nullptr},   // 播放模式-单曲
      *mPlaybackMode_CurrentItemInLoop{nullptr}, // 播放模式-单曲循环
      *mPlaybackMode_Sequential{nullptr},        // 播放模式-列表顺序
      *mPlaybackMode_Loop{nullptr},              // 播放模式-循环
      *mPlaybackMode_Random{nullptr};            // 播放模式-随机
};
#endif // VIDEOPLAYERACTIONS_H
