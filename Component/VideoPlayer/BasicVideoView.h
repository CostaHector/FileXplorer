#ifndef BASICVIDEOVIEW_H
#define BASICVIDEOVIEW_H

#include <QWidget>
#include <QMediaPlayer>
#include <QVBoxLayout>
#include <QTimer>
#include "AutoHideLabel.h"
#include "ClickableSlider.h"
#include "ToolBarWidget.h"
#include "InteractiveVideoWidget.h"

class FolderNxtAndLastIterator;
class BasicVideoView : public QWidget {
  Q_OBJECT
public:
  explicit BasicVideoView(bool bBasicMode, QWidget* parent = nullptr);
  virtual ~BasicVideoView();

  bool PlayAVideo(const QString& filePath, bool forcePlayInstantly = false);
  QString GetCurrentPlayingMediaPath() const { return mCurrentPlayingMediaPath; }
  const InteractiveVideoWidget* GetVideoWidget() const { return mVideoWidget; }
  void onChangeToolBarVisibility(bool bHide);
  void onStopPlaying();
  bool isVideoFullScreen() const;
  bool registerFullScreenToggleCallback(TFuncFullScreenToggleCallback funcCallback);
  bool rateCurrentVideo(int score) const;
  int rateAllVideoSameLevelAsCurrentVideo(bool bForce) const;
  bool adjustRateCurrentVideo(int delta) const;
  int adjustRateAllVideoSameLevelAsCurrentVideo(int delta) const;
  void setAlwaysShowStatusLabel();

  /* 重命名 */
  bool on_RenameReplace();

  /* 访问 前一个/后一个 视频 */
  void EnableVideoIterate();
  bool isEnableVideoIterate() { return mVideoIterator != nullptr; }
  void DisableVideoIterate();
  void clearVideoIterate();

signals:
  void reqFunctionModeChange(bool bBasicMode);
  void reqFullscreenModeChange(bool bFullScreen);
  void reqPlayNextOneMedia();
  void userMousePressOrKeyPressHappened();
  void currentMediaChanged(const QString& mediaPath);

protected:
  void showEvent(QShowEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  bool eventFilter(QObject* watched, QEvent* event) override;
  QMediaPlayer* GetPlayer() { return mPlayer; }
  const QMediaPlayer* GetPlayer() const { return mPlayer; }

private:
  void subscribe();
  void emitFullScreenModeReq(bool bFullScreen);
  void onDurationChanged(qint64 duration);
  bool onUpdateProgressSliderPosition();
  bool onGrabCurrentFrame();
  void onPauseActionToggled(bool pauseChecked);
  void onStateChanged(QMediaPlayer::State state);
  void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
  static bool SetMediaCore(BasicVideoView* self, const QString& mediaAbsPath);
  static bool PlayCore(QMediaPlayer* mediaPlayer);
  static bool DeviatePositionCore(QMediaPlayer* mPlayer, int deviationInSeconds);
  bool deviatePositionPrevious();
  bool deviatePositionNext();
  static bool SetPositionCore(QMediaPlayer* mPlayer, int newPosition);
  static qint64 GetPositionCore(QMediaPlayer* mPlayer);
  bool reqPlayInSystemApplication() const;
  QMediaPlayer::Error onError(QMediaPlayer::Error error) const;
  int onAudioAvailableChanged(bool available) const;
  void setMediaWithStatus(const QString& filePath, QIODevice* stream = nullptr);

  QMediaPlayer* mPlayer{nullptr};
  InteractiveVideoWidget* mVideoWidget{nullptr}; // 播放显示框

  ToolBarWidget* mFunctionCtrlBar{nullptr};    // 功能控制条
  AutoHideLabel* mOperationStatusLbl{nullptr}; // 操作状态标签
  ClickableSlider* mProgressSlider{nullptr};   // 进度控制块
  QLabel* mCurrentTimeLabel{nullptr};          // 当前播放的时间点
  QLabel* mDurationLabel{nullptr};             // 视频文件时长

  QVBoxLayout* mLeftLayout{nullptr}; // 左侧布局

  TFuncFullScreenToggleCallback mFullScreenCallback;

  QTimer mProgressSliderUpdateTimer;
  QString mCurrentPlayingMediaPath;
  FolderNxtAndLastIterator* mVideoIterator{nullptr};
  bool onNavigatePreviousVideo() { return navigateVideo(false); }
  bool onNavigateNextVideo()     { return navigateVideo(true);  }
  bool navigateVideo(bool bForward);

  mutable QMediaPlayer::Error mError{QMediaPlayer::Error::NoError};
};
#endif // BASICVIDEOVIEW_H
