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

class VideoView : public QSplitter {
  Q_OBJECT

public:
  explicit VideoView(bool bBasicMode = false, QWidget* parent = nullptr);
  virtual ~VideoView();
  void onReqModeChange(bool bBasicMode);
  int PlayAPath(const QString& folderPath, bool bPlayInstantly);
  int PlayVideos(const QString& rootPath, const QStringList& mediafileNames, bool bPlayInstantly);
  bool onSelectMediaFiles();
  QString GetCurrentPlayingMediaPath() const { return mBasicVideoView == nullptr ? "" : mBasicVideoView->GetCurrentPlayingMediaPath(); }
  const BasicVideoView* GetBasicVideoView() const { return mBasicVideoView; }
  void StopPlay();
  void onChangeToolBarVisibility(bool bHide);
  void onChangeListVisibility(bool visibility);
  bool registerFullScreenToggleCallback(TFuncFullScreenToggleCallback funcCallback);

protected:
  QSize sizeHint() const override { return {480, 360}; }

private:
  void subscribe();

  BasicVideoView* mBasicVideoView{nullptr};
  QWidget* mExtendedFunctionCtrlBar{nullptr};                          // 扩展功能控制条
  QVBoxLayout* mExtendLeftLayout{nullptr};                             // 扩展左侧布局
  QWidget* mExtendLeftWidget{nullptr};                                 // 扩展左侧Widget

  VideoTableView* mVideoList{nullptr}; // 视频文件列表

  TFuncFullScreenToggleCallback mFullScreenCallback;
};
#endif // VIDEOVIEW_H
