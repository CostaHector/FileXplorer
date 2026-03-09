#ifndef DETAILPREVIEW_H
#define DETAILPREVIEW_H

#include "BasicVideoView.h"
#include "ClickableTextBrowser.h"
#include "FullScreenableSplitter.h"

class DetailPreview : public FullScreenableSplitter {
 public:
  explicit DetailPreview(QWidget* parent = nullptr);
  virtual ~DetailPreview();
  QSize iconSize() const;
  void setHtml(const QString& text);
  void SetCastHtmlParts(const CastHtmlParts& castHtmls);
  void UpdateHtmlContents();

  void StopPlay();
  void UpdateWhenSelectAFile(const QString& pth);

 protected:
  QWidget* GetFullScreenableWidget() const override {
    return mBasicVideoView;
  }

 private:
  ClickableTextBrowser* mDetailTextBrowser{nullptr};
  BasicVideoView* mBasicVideoView{nullptr};
};

#endif  // DETAILPREVIEW_H
