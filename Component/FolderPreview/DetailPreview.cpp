#include "DetailPreview.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include "MemoryKey.h"
#include "Logger.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"

DetailPreview::DetailPreview(QWidget* parent) : FullScreenableSplitter{"DETAIL_PREVIEW_SPLITTER", parent} {
  setOrientation(Qt::Orientation::Vertical);

  mDetailTextBrowser = new (std::nothrow) ClickableTextBrowser{this};
  mBasicVideoView = new (std::nothrow) BasicVideoView{true, this};

  mBasicVideoView->setVisible(false);
  addWidget(mDetailTextBrowser);
  addWidget(mBasicVideoView);

  mBasicVideoView->registerFullScreenToggleCallback(std::bind(&DetailPreview::onReqFullscreenModeChange, this, std::placeholders::_1));
  restoreState(Configuration().value(GetMemoryName() + "/STATE").toByteArray());
}

DetailPreview::~DetailPreview() {  //
  saveStateInDerivedDestructor();
}

QSize DetailPreview::iconSize() const {
  CHECK_NULLPTR_RETURN_INT(mDetailTextBrowser, {});
  return mDetailTextBrowser->iconSize();
}

void DetailPreview::setHtml(const QString& text) {
  CHECK_NULLPTR_RETURN_VOID(mDetailTextBrowser);
  mDetailTextBrowser->setHtml(text);
}

void DetailPreview::SetCastHtmlParts(const CastHtmlParts& castHtmls) {
  CHECK_NULLPTR_RETURN_VOID(mDetailTextBrowser);
  mDetailTextBrowser->SetCastHtmlParts(castHtmls);
}
void DetailPreview::UpdateHtmlContents() {
  CHECK_NULLPTR_RETURN_VOID(mDetailTextBrowser);
  mDetailTextBrowser->UpdateHtmlContents();
}

void DetailPreview::onStopPlaying() {
  mBasicVideoView->onStopPlaying();
}

void DetailPreview::UpdateWhenSelectAFile(const QString& pth) {
  const bool bIsAVideo{TYPE_FILTER::isDotExtVideo(PathTool::GetDotFileExtension(pth))};
  // 更新可见性, 开始/停止播放, 更新基础描述html文本
  if (mBasicVideoView->isHidden() == bIsAVideo) {
    mBasicVideoView->setVisible(bIsAVideo);
  }
  if (bIsAVideo) {
    mBasicVideoView->PlayAVideo(pth, false);
  }
  const QSize ICON_SIZE = iconSize();
  const QString detailHtmls = CastBrowserHelper::GetDetailDescription(pth, ICON_SIZE);
  setHtml("");
  setHtml(detailHtmls);
}
