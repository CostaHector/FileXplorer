#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <QWidget>
#include <QVideoProbe>
#include <QToolBar>
#include <QSplitter>
#include <QVBoxLayout>
#include <QToolButton>
#include <QMediaPlaylist>
#include "ClickableSlider.h"
#include "EnumIntAction.h"
#include "MenuToolButton.h"
#include "BasicVideoView.h"
#include "VideoTableView.h"

extern template struct EnumIntAction<QMediaPlaylist::PlaybackMode>;

class VideoView : public QSplitter {
  Q_OBJECT

public:
  explicit VideoView(bool bBasicMode = false, QWidget* parent = nullptr);
  virtual ~VideoView();
  void onReqModeChange(bool bBasicMode);
  int PlayAPath(const QString& folderPath);
  int PlayVideos(const QString& rootPath, const QStringList& mediafileNames);
  int onSelectAFolder();
  bool onSelectMediaFiles();

private:
  void subscribe();
  void onPlaybackModeChanged(const QAction* newPlaybackModeAct);

  BasicVideoView* mBasicVideoView{nullptr};
  QAction* mOpenVideoFolder{nullptr};                                  // 选择视频文件夹并播放
  QAction *mSeekBackwardHotAct{nullptr}, *mSeekForwardHotAct{nullptr}; // 上一个热点, 下一个热点
  QAction *mPlayPrevAct{nullptr}, *mPlayNextAct{nullptr};              // 上一首, 下一首
  QAction* mShowFrames{nullptr};                                       // 展示帧截图
  QAction                                                              //
      *mPlaybackMode_CurrentItemOnce{nullptr},                         // 播放模式-单曲
      *mPlaybackMode_CurrentItemInLoop{nullptr},                       // 播放模式-单曲循环
      *mPlaybackMode_Sequential{nullptr},                              // 播放模式-列表顺序
      *mPlaybackMode_Loop{nullptr},                                    // 播放模式-循环
      *mPlaybackMode_Random{nullptr};                                  // 播放模式-随机
  MenuToolButton* mPlaybackMode{nullptr};                              // 列表播放模式控制
  QAction* mShowVideoList{nullptr};                                    // 隐藏视频文件列表
  QToolBar* mExtendedFunctionCtrlBar{nullptr};                         // 扩展功能控制条
  QVBoxLayout* mExtendLeftLayout{nullptr};                             // 扩展左侧布局
  QWidget* mExtendLeftWidget{nullptr};                                 // 扩展左侧Widget

  VideoTableView* mVideoList{nullptr}; // 视频文件列表

  EnumIntAction<QMediaPlaylist::PlaybackMode> mPlaybackIntAction;                                                     //
  static constexpr QMediaPlaylist::PlaybackMode DEFAULT_PLAYBACK_MODE{QMediaPlaylist::PlaybackMode::CurrentItemOnce}; // 缺省时列表播放模式
  static constexpr int SOURCE_INDEX_COLUMN = 2;
};
#endif // VIDEOVIEW_H
