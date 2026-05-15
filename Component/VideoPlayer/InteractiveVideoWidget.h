#ifndef INTERACTIVEVIDEOWIDGET_H
#define INTERACTIVEVIDEOWIDGET_H

#include <QVideoWidget>
#include <QMediaPlaylist>
#include <QMenu>
#include <QTimer>
#include "RateActions.h"

typedef std::function<bool(bool)> TFuncFullScreenToggleCallback;

class InteractiveVideoWidget : public QVideoWidget {
  Q_OBJECT
public:
  friend class BasicVideoView;
  friend class VideoView;

  explicit InteractiveVideoWidget(bool bBasicMode = false, QWidget* parent = nullptr);
  ~InteractiveVideoWidget();

  QWidget* GetExtendedFunctionCtrlBar(QWidget* notNullParent) const;

  bool updatePauseActionState(bool bPauseChecked);

  bool isVideoFullScreen() const { return mFullScreenAct->isChecked(); }
  RateActions* GetRateActions() const { return mRateActions; }
  bool isClickPressHappend() { return mClickPressHappend; }
  void clearClickPressHappend() { mClickPressHappend = false; }
  void onUserMouseClickOrKeyPressEvent() { mClickPressHappend = true; }
  bool isAutoHideToolBarDisabled() { return mDisableAutoHideToolBar->isChecked(); }
  bool isShowVideoListView() const { return mShowVideoList != nullptr && mShowVideoList->isChecked(); }

signals:
  void layoutVisibilityChanged();
  void fullScreenModeToggled(bool bFullScreen);
  void newFileSelectedByUser(const QString& mediaFileSelected, bool bForcePlayInstant);
  void newFolderSelectedChangedByUser(const QString& mediaFolderSelected, bool bForcePlayInstant);

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;

private:
  void onFullScreenActionToggled(bool bFullScreen);
  static bool GetFocusCore(InteractiveVideoWidget* self);
  void onIntoFullScreenMode();
  void onQuitFullScreenMode();
  bool onSelectAFile();
  bool onSelectAFolder();
  void tryStartAutoHideTimer();

  QAction* mPauseAct{nullptr};                                         // 通用暂停/继续动作
  QAction* mStopAct{nullptr};                                          // 停止播放
  QAction *mSeekBackwardAct{nullptr}, *mSeekForwardAct{nullptr};       // 快退10s, 快进10s
  QAction *mSeekBackwardHotAct{nullptr}, *mSeekForwardHotAct{nullptr}; // 上一个热点, 下一个热点
  QAction *mPlayPrevAct{nullptr}, *mPlayNextAct{nullptr};              // 上一首, 下一首
  RateActions* mRateActions{nullptr};                                  // 单文件/路径内文件递归评分
  QAction* mShowFrames{nullptr};                                       // 展示帧截图
  QAction* mShowVideoList{nullptr};                                    // 显示视频文件列表

  QAction* mBasicModeAct{nullptr};            // 基础功能模式
  QAction* mHideToolBarAct{nullptr};          // 隐藏工具栏
  QAction* mFullScreenAct{nullptr};           // 全屏播放
  QAction* mOpenInSystemApplication{nullptr}; // 用系统应用打开

  QAction* mSelectVideoFileAct{nullptr};     // 选择视频文件
  QAction* mSelectVideoFolder{nullptr};      // 选择视频文件夹并播放
  QAction* mDisableAutoHideToolBar{nullptr}; // 禁用自动隐藏工具栏

  QMenu* mRateMenu{nullptr};
  QMenu* mContextMenu{nullptr};

  void onMouseRightClickEventHappend();
  void changeAllToolbarVisibility(bool visible);
  void onLongTimeNoEventHappen();
  QTimer mLongTimeNoClickTimer;
  bool mClickPressHappend{false};
};

#endif // INTERACTIVEVIDEOWIDGET_H
