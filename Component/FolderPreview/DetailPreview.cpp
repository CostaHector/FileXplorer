#include "DetailPreview.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include "MemoryKey.h"
#include "Logger.h"

DetailPreview::DetailPreview(QWidget* parent)
  : QSplitter{parent} {
  setOrientation(Qt::Orientation::Vertical);

  mDetailTextBrowser = new (std::nothrow) ClickableTextBrowser{this};
  mBasicVideoView = new (std::nothrow) BasicVideoView{true, this};
  mBasicVideoView->setVisible(false);
  addWidget(mDetailTextBrowser);
  addWidget(mBasicVideoView);

  connect(mBasicVideoView, &BasicVideoView::reqFullscreenModeChange, this, &DetailPreview::onReqFullscreenModeChange);

  restoreState(Configuration().value("DETAIL_PREVIEW_SPLITTER_STATE").toByteArray());
}

DetailPreview::~DetailPreview() {
  if (mBasicVideoView->parent() != nullptr) { // only save when mBasicVideoView is this instead of out
    Configuration().setValue("DETAIL_PREVIEW_SPLITTER_STATE", saveState());
  }
}

QSize DetailPreview::iconSize() const {
  if (mDetailTextBrowser == nullptr) {
    return {};
  }
  return mDetailTextBrowser->iconSize();
}

void DetailPreview::setHtml(const QString& text) {
  if (mDetailTextBrowser == nullptr) {
    return;
  }
  mDetailTextBrowser->setHtml(text);
}

void DetailPreview::SetCastHtmlParts(const CastHtmlParts& castHtmls) {
  if (mDetailTextBrowser == nullptr) {
    return;
  }
  mDetailTextBrowser->SetCastHtmlParts(castHtmls);
}
void DetailPreview::UpdateHtmlContents() {
  if (mDetailTextBrowser == nullptr) {
    return;
  }
  mDetailTextBrowser->UpdateHtmlContents();
}

void DetailPreview::StopPlay() {
  mBasicVideoView->StopPlay();
}

void DetailPreview::UpdateWhenSelectAFile(const QString& pth) {
  const bool bIsAVideo{TYPE_FILTER::isDotExtVideo(PathTool::GetDotFileExtension(pth))};
  // 更新可见性, 开始/停止播放, 更新基础描述html文本
  if (mBasicVideoView->isVisible() != bIsAVideo) {
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

void DetailPreview::onReqFullscreenModeChange(bool bFullScreen) {
  if (mBasicVideoView == nullptr) {
    return;
  }
  LOG_E("req, %d", bFullScreen);
  // 最大化
  if (bFullScreen) {
    if (mBasicVideoView->parent() == nullptr) {
      return;
    }
    mBasicVideoView->setParent(nullptr);
    mBasicVideoView->setAttribute(Qt::WA_DeleteOnClose);
    mBasicVideoView->setWindowFlags(Qt::Window);
    mBasicVideoView->showFullScreen();
    return;
  }
  // 正常大小
  if (mBasicVideoView->parent() != nullptr) {
    return;
  }
  mBasicVideoView->setParent(this);
  mBasicVideoView->setAttribute(Qt::WA_DeleteOnClose, false);
  mBasicVideoView->setWindowFlags(Qt::Widget);
  mBasicVideoView->showNormal();
}
