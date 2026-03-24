#ifndef INTERACTIVEVIDEOWIDGET_H
#define INTERACTIVEVIDEOWIDGET_H

#include <QVideoWidget>
#include <QMediaPlaylist>
#include <QMenu>
#include <QTimer>
#include "EnumIntAction.h"
#include "MenuToolButton.h"
#include "VideoPlayTool.h"
#include "RateActions.h"

extern template struct EnumIntAction<QMediaPlaylist::PlaybackMode>;
extern template struct EnumIntAction<VideoPlayTool::PlaybackTriggerMode>;
typedef std::function<bool(bool)> TFuncFullScreenToggleCallback;

class InteractiveVideoWidget : public QVideoWidget {
  Q_OBJECT
public:
  friend class BasicVideoView;
  friend class VideoView;

  explicit InteractiveVideoWidget(bool bBasicMode = false, QWidget* parent = nullptr);
  MenuToolButton* GetPlaybackModelMenuToolButton(QWidget* notNullParent) const;
  MenuToolButton* GetPlaybackTriggerModelMenuToolButton(QWidget* notNullParent) const;
  QWidget* GetExtendedFunctionCtrlBar(QWidget* notNullParent) const;

  bool updatePauseActionState(bool bPauseChecked);

  bool isVideoFullScreen() const { return mFullScreenAct->isChecked(); }
  QMediaPlaylist::PlaybackMode GetPlaybackMode() const;
  VideoPlayTool::PlaybackTriggerMode GetPlaybackTriggerMode() const;
  RateActions* GetRateActions() const {
    return mRateActions;
  }
  bool isClickPressHappend() { return mClickPressHappend; }
  void clearClickPressHappend() { mClickPressHappend = false; }
  void onUserMouseClickOrKeyPressEvent() { mClickPressHappend = true; }

signals:
  void playbackModeChanged(QMediaPlaylist::PlaybackMode newPlaybackMode);
  void playbackTriggerModeChanged(VideoPlayTool::PlaybackTriggerMode newPlaybackTriggerMode);
  void layoutVisibilityChanged();
  void fullScreenModeToggled(bool bFullScreen);
  void newFileSelectedByUser(const QString& mediaFileSelected, bool bForcePlayInstant);
  void newFolderSelectedChangedByUser(const QString& mediaFolderSelected, bool bForcePlayInstant);

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;

private:
  void onPlaybackModeTriggered(const QAction* newPlaybackModeAct);
  EnumIntAction<QMediaPlaylist::PlaybackMode> mPlaybackModeIntAction;                                                 //
  static constexpr QMediaPlaylist::PlaybackMode DEFAULT_PLAYBACK_MODE{QMediaPlaylist::PlaybackMode::CurrentItemOnce}; // 缺省时列表播放模式

  void onPlaybackTriggerModeTriggered(const QAction* newPlaybackTriggerModeAct);
  EnumIntAction<VideoPlayTool::PlaybackTriggerMode> mPlaybackTriggerIntAction; //

  void onFullScreenActionToggled(bool bFullScreen);
  static bool GetFocusCore(InteractiveVideoWidget* self);
  void onIntoFullScreenMode();
  void onQuitFullScreenMode();
  bool onSelectAFile();
  bool onSelectAFolder();

  QAction                         // 播放触发模式
      *mPlaybackTrigger_MANUAL,   // 播放触发模式-手动播放
      *mPlaybackTrigger_AUTO,     // 播放触发模式-自动播放
      *mPlaybackTrigger_DISABLED; // 播放触发模式-禁用播放

  QAction* mPauseAct{nullptr};                                         // 通用暂停/继续动作
  QAction* mStopAct{nullptr};                                          // 停止播放
  QAction *mSeekBackwardAct{nullptr}, *mSeekForwardAct{nullptr};       // 快退10s, 快进10s
  QAction *mSeekBackwardHotAct{nullptr}, *mSeekForwardHotAct{nullptr}; // 上一个热点, 下一个热点
  QAction *mPlayPrevAct{nullptr}, *mPlayNextAct{nullptr};              // 上一首, 下一首
  QAction *mVolumePlus{nullptr}, *mVolumeMinus{nullptr};               // 增加音量, 减少音量
  RateActions* mRateActions{nullptr};                                  // 单文件/路径内文件递归评分
  QAction* mShowFrames{nullptr};                                       // 展示帧截图
  QAction* mShowVideoList{nullptr};                                    // 显示视频文件列表

  QAction                                        // 播放模式
      *mPlaybackMode_CurrentItemOnce{nullptr},   // 播放模式-单曲
      *mPlaybackMode_CurrentItemInLoop{nullptr}, // 播放模式-单曲循环
      *mPlaybackMode_Sequential{nullptr},        // 播放模式-列表顺序
      *mPlaybackMode_Loop{nullptr},              // 播放模式-循环
      *mPlaybackMode_Random{nullptr};            // 播放模式-随机

  QAction* mBasicModeAct{nullptr};            // 基础功能模式
  QAction* mHideToolBarAct{nullptr};          // 隐藏工具栏
  QAction* mFullScreenAct{nullptr};           // 全屏播放
  QAction* mOpenInSystemApplication{nullptr}; // 用系统应用打开

  QAction* mSelectVideoFileAct{nullptr}; // 选择视频文件
  QAction* mSelectVideoFolder{nullptr};  // 选择视频文件夹并播放

  QMenu* mRateMenu{nullptr};
  QMenu* mPlaybackModeMenu{nullptr};
  QMenu* mContextMenu{nullptr};

  void onMouseRightClickEventHappend();
  void changeAllToolbarVisibility(bool visible);
  void onLongTimeNoEventHappen();
  QTimer mLongTimeNoClickTimer;
  bool mClickPressHappend{false};
  static constexpr int TIMER_INTERVAL = 10 * 1000;
};

#endif // INTERACTIVEVIDEOWIDGET_H
