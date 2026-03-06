#ifndef BASICVIDEOVIEW_H
#define BASICVIDEOVIEW_H

#include <QWidget>
#include <QLabel>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QToolBar>
#include <QSlider>
#include <QVBoxLayout>
#include <QToolButton>
#include "ClickableSlider.h"
#include "MenuToolButton.h"
#include "VolumeWidget.h"
#include "ToolBarWidget.h"
#include <QTimer>

class PausableVideoWidget : public QVideoWidget {
  Q_OBJECT
 public:
  explicit PausableVideoWidget(QWidget* parent = nullptr);
  void onIntoFullScreenMode();
  void onQuitFullScreenMode();

 signals:
  void reqPausePlayStatusToggle();
  void reqToolBarVisibilityChange(bool bVisible);

 protected:
  void mousePressEvent(QMouseEvent* event) override;

 private:
  void onTimerTimeout();
  QTimer mLongTimeNoClickTimer;
  static constexpr int TIMER_INTERVAL = 10 * 1000;
};

class BasicVideoView : public QWidget {
  Q_OBJECT
 public:
  friend class VideoView;

  explicit BasicVideoView(bool bBasicMode, QWidget* parent = nullptr);
  virtual ~BasicVideoView();

  bool PlayAVideo(const QString& filePath, bool forcePlayInstantly = false);
  bool isAutoPlay() const { return mPlayInstantlyBtn != nullptr && mPlayInstantlyBtn->isChecked(); }
  bool isFullScreen() const {return mFullScreenAct!= nullptr && mFullScreenAct->isChecked(); }
  void StopPlay();
  QString GetCurrentPlayingMediaPath() const { return mCurrentPlayingMediaPath; }
  const PausableVideoWidget* GetVideoWidget() const { return mVideoWidget; }
  void onChangeToolBarVisibility(bool visibility);

 signals:
  void reqFunctionModeChange(bool bBasicMode);
  void reqFullscreenModeChange(bool bFullScreen);

 protected:
  void resizeEvent(QResizeEvent* e) override;
  void keyPressEvent(QKeyEvent* event) override;
  QMediaPlayer* GetPlayer() { return mPlayer; }
  const QMediaPlayer* GetPlayer() const { return mPlayer; }

 private:  
  void subscribe();
  void emitFullScreenModeReq(bool bFullScreen);
  void durationChanged(qint64 duration);
  void onPlaying(qint64 position);
  void onStopPlaying();
  void onPauseActionToggled(bool pauseChecked);
  void onMediaPlayStateChanged(QMediaPlayer::State state);
  bool onSelectAFile();
  void movePauseBtnToCenter();

  QMediaPlayer* mPlayer{nullptr};
  PausableVideoWidget* mVideoWidget{nullptr};  // 播放显示框

  ToolBarWidget* mFunctionCtrlBar{nullptr};                           // 功能控制条
  QToolButton* mPlayInstantlyBtn{nullptr};                            // 是否自动播放
  QToolButton* mSelectVideoFileBtn{nullptr};                          // 选择视频文件
  QAction* mPauseAct{nullptr};                                        // 通用暂停/继续动作
  QToolButton* mPauseBtn{nullptr};                                    // 暂停/继续
  QToolButton* mPauseShieldButton{nullptr};                           // 暂停遮罩画面
  QToolButton* mStopBtn{nullptr};                                     // 停止播放
  QToolButton *mSeekBackwardBtn{nullptr}, *mSeekForwardBtn{nullptr};  // 快退10s, 快进10s
  ClickableSlider* mProgressSlider{nullptr};                          // 进度控制块
  QLabel* mCurrentTimeLabel{nullptr};                                 // 当前播放的时间点
  QLabel* mDurationLabel{nullptr};                                    // 视频文件时长
  VolumeWidget* mVolumeWid{nullptr};                                  // 音量控制组件
  QToolButton* mBasicModeBtn{nullptr};                                // 基础功能模式
  QAction* mFullScreenAct{nullptr};
  QToolButton* mFullScreenBtn{nullptr};  // 全屏播放

  QVBoxLayout* mLeftLayout{nullptr};  // 左侧布局

  QString mCurrentPlayingMediaPath;
  bool bPauseButtonCenterInit = false;
};
#endif  // BASICVIDEOVIEW_H
