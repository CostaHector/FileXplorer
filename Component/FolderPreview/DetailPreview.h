#ifndef DETAILPREVIEW_H
#define DETAILPREVIEW_H

#include "BasicVideoView.h"
#include "ClickableTextBrowser.h"
#include <QSplitter>

class DetailPreview : public QSplitter {
 public:
  explicit DetailPreview(QWidget* parent = nullptr);
  virtual ~DetailPreview();
  QSize iconSize() const;
  void setHtml(const QString& text);
  void SetCastHtmlParts(const CastHtmlParts& castHtmls);
  void UpdateHtmlContents();

  void StopPlay();
  void UpdateWhenSelectAFile(const QString& pth);

  void onReqFullscreenModeChange(bool bFullScreen);

 private:
  ClickableTextBrowser* mDetailTextBrowser{nullptr};
  BasicVideoView* mBasicVideoView{nullptr};
  QWidget* rmReplacer{nullptr};
  Qt::WindowFlags beforeFlags;

  void CleanTempFullScreenWindow();
  QWidget* mFullScreenWindow{nullptr};
  QByteArray mBeforeFullScreenState;
  int mVideoViewOriginalIndex{0};
};

#endif  // DETAILPREVIEW_H
