#include "FloatingPreview.h"
#include "CastBrowserHelper.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include <QIcon>
#include <QFileInfo>

FloatingPreview::FloatingPreview(const QString& memoryName, QWidget* parent)
  : QStackedWidget{parent}
{
  mDetailsPane = new (std::nothrow) ClickableTextBrowser{this};
  CHECK_NULLPTR_RETURN_VOID(mDetailsPane)

  mImgVidOtherPane = new (std::nothrow) ImgVidOthWid{memoryName, this};
  CHECK_NULLPTR_RETURN_VOID(mImgVidOtherPane)

  addWidget(mDetailsPane);
  addWidget(mImgVidOtherPane);
  if (currentIndex() != (int)m_curIndex) {
    setCurrentIndex((int)m_curIndex);
  }

  ReadSettings();
  setWindowIcon(QIcon(":img/FLOATING_PREVIEW"));
}

void FloatingPreview::ReadSettings() {
  if (PreferenceSettings().contains("FLOATING_PREVIEW_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("FLOATING_PREVIEW_GEOMETRY").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 480, 1080));
  }
}

void FloatingPreview::SaveSettings() {
  PreferenceSettings().setValue("FLOATING_PREVIEW_GEOMETRY", saveGeometry());
}

void FloatingPreview::operator()(const QSqlRecord& record, const QString& imgHost, const int imgHeight) {
  CHECK_NULLPTR_RETURN_VOID(mDetailsPane)
  if (record.isEmpty()) {
    mDetailsPane->setHtml("");
    return;
  }

  setWindowTitle(mLastName);
  BeforeDisplayAFileDetail();
  using namespace CastBrowserHelper;
  const stCastHtml castHtmls = GetCastHtml(record, imgHost, imgHeight);
  mDetailsPane->SetCastHtmlParts(castHtmls);
  mDetailsPane->UpdateHtmlContents();
}

void FloatingPreview::operator()(const QString& pth) {  // file system view
  if (!NeedUpdate(pth)) {
    return;
  }

  mLastName = pth;
  setWindowTitle(mLastName);
  if (QFileInfo{pth}.isFile()) {  // a file
    BeforeDisplayAFileDetail();
    mDetailsPane->setHtml(CastBrowserHelper::GetDetailDescription(pth));
    return;
  }
  BeforeDisplayAFolder();
  mImgVidOtherPane->operator()(pth);
}

void FloatingPreview::operator()(const QString& name, const QString& pth) {  // scene view
  mLastName = name;
  setWindowTitle(mLastName);
  mImgVidOtherPane->operator()(name, pth);
}

