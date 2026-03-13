#ifndef BASICVIDEOVIEW_H
#define BASICVIDEOVIEW_H

#include <QWidget>
#include <QLabel>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QSlider>
#include <QVBoxLayout>
#include <QToolButton>
#include <QTimer>
#include "ClickableSlider.h"
#include "MenuToolButton.h"
#include "VolumeWidget.h"
#include "ToolBarWidget.h"
#include "InteractiveVideoWidget.h"

class BasicVideoView : public QWidget {
  Q_OBJECT
public:
  friend class VideoView;

  explicit BasicVideoView(bool bBasicMode, QWidget* parent = nullptr);
  virtual ~BasicVideoView();

  bool PlayAVideo(const QString& filePath, bool forcePlayInstantly = false);
  bool StopPlay();
  QString GetCurrentPlayingMediaPath() const { return mCurrentPlayingMediaPath; }
  const InteractiveVideoWidget* GetVideoWidget() const { return mVideoWidget; }
  void onChangeToolBarVisibility(bool bHide);
  bool isVideoFullScreen() const;
  bool registerFullScreenToggleCallback(TFuncFullScreenToggleCallback funcCallback);

signals:
  void reqFunctionModeChange(bool bBasicMode);
  void reqFullscreenModeChange(bool bFullScreen);

protected:
  void resizeEvent(QResizeEvent* e) override;
  QMediaPlayer* GetPlayer() { return mPlayer; }
  const QMediaPlayer* GetPlayer() const { return mPlayer; }

private:
  void subscribe();
  void emitFullScreenModeReq(bool bFullScreen);
  void onDurationChanged(qint64 duration);
  bool onUpdateProgressSliderPosition();
  void onStopPlaying();
  void onPauseActionToggled(bool pauseChecked);
  void onMediaPlayStateChanged(QMediaPlayer::State state);
  void movePauseBtnToCenter();
  static bool SetMediaCore(QMediaPlayer* mediaPlayer, const QUrl& mediaUrl);
  static bool PlayCore(QMediaPlayer* mediaPlayer);
  static bool DeviatePositionCore(QMediaPlayer* mPlayer, int deviationInSeconds);
  bool deviatePositionPrevious();
  bool deviatePositionNext();
  static bool SetPositionCore(QMediaPlayer* mPlayer, int newPosition);
  static qint64 GetPositionCore(QMediaPlayer* mPlayer);
  bool reqPlayInSystemApplication() const;
  QMediaPlayer::Error onError(QMediaPlayer::Error error) const;
  int onAudioAvailableChanged(bool available) const;

  QMediaPlayer* mPlayer{nullptr};
  InteractiveVideoWidget* mVideoWidget{nullptr}; // 播放显示框

  ToolBarWidget* mFunctionCtrlBar{nullptr};  // 功能控制条
  QToolButton* mPauseShieldButton{nullptr};  // 暂停遮罩画面
  ClickableSlider* mProgressSlider{nullptr}; // 进度控制块
  QLabel* mCurrentTimeLabel{nullptr};        // 当前播放的时间点
  QLabel* mDurationLabel{nullptr};           // 视频文件时长
  VolumeWidget* mVolumeWid{nullptr};         // 音量控制组件

  QVBoxLayout* mLeftLayout{nullptr}; // 左侧布局

  void setPlaybackTriggerMode(VideoPlayTool::PlaybackTriggerMode newTriggerMode) { mPlaybackTriggerMode = newTriggerMode; }
  VideoPlayTool::PlaybackTriggerMode GetPlayTriggerMode() const { return mPlaybackTriggerMode; }
  VideoPlayTool::PlaybackTriggerMode mPlaybackTriggerMode{VideoPlayTool::DEFAULT_PLAYBACK_TRIGGER_MODE};

  TFuncFullScreenToggleCallback mFullScreenCallback;

  QTimer mProgressSliderUpdateTimer;
  QString mCurrentPlayingMediaPath;
  bool bPauseButtonCenterInit = false;

  mutable QMediaPlayer::Error mError{QMediaPlayer::Error::NoError};
};
#endif // BASICVIDEOVIEW_H
