#ifndef DETAILPREVIEW_H
#define DETAILPREVIEW_H

#include "BasicVideoView.h"
#include "ClickableTextBrowser.h"
#include "FullScreenableSplitter.h"

class DetailPreview : public FullScreenableSplitter {
  Q_OBJECT
 public:
  explicit DetailPreview(QWidget* parent = nullptr);
  virtual ~DetailPreview();
  QSize iconSize() const;
  void setHtml(const QString& text);
  void SetCastHtmlParts(const CastHtmlParts& castHtmls);
  void UpdateHtmlContents();

  void onStopPlaying();
  void UpdateWhenSelectAFile(const QString& pth);

 signals:
  void forwardPlaySiblingMediaRequested(int step);

 protected:
  QWidget* GetFullScreenableWidget() const override {
    return mBasicVideoView;
  }

 private:
  ClickableTextBrowser* mDetailTextBrowser{nullptr};
  BasicVideoView* mBasicVideoView{nullptr};
};

#endif  // DETAILPREVIEW_H
