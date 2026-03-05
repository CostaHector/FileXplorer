#include "FileFolderPreviewer.h"
#include "CastBrowserHelper.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include <QIcon>
#include <QFileInfo>

FileFolderPreviewer::FileFolderPreviewer(const QString& memoryName, QWidget* parent)
  : QStackedWidget{parent} {
  mDetailsPane = new (std::nothrow) DetailPreview{this};
  CHECK_NULLPTR_RETURN_VOID(mDetailsPane)

  mImgVidOtherPane = new (std::nothrow) ImgVidOthInFolderPreviewer{memoryName, this};
  CHECK_NULLPTR_RETURN_VOID(mImgVidOtherPane)

  addWidget(mDetailsPane);
  addWidget(mImgVidOtherPane);
  if (currentIndex() != (int) m_curIndex) {
    setCurrentIndex((int) m_curIndex);
  }

  ReadSettings();
  setWindowIcon(QIcon(":img/FLOATING_PREVIEW"));
}

FileFolderPreviewer::~FileFolderPreviewer() {
  SaveSettings();
}

void FileFolderPreviewer::ReadSettings() {
  if (Configuration().contains("FLOATING_PREVIEW_GEOMETRY")) {
    restoreGeometry(Configuration().value("FLOATING_PREVIEW_GEOMETRY").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 480, 1080));
  }
}

void FileFolderPreviewer::SaveSettings() {
  Configuration().setValue("FLOATING_PREVIEW_GEOMETRY", saveGeometry());
}

void FileFolderPreviewer::operator()(const QSqlRecord& record, const QString& imgHost) {
  CHECK_NULLPTR_RETURN_VOID(mDetailsPane)
  mDetailsPane->setHtml("");
  mDetailsPane->StopPlay();
  if (record.isEmpty()) {
    return;
  }

  setWindowTitle(mLastName);
  BeforeDisplayAFileDetail();
  QSize ICON_SIZE = mDetailsPane->iconSize();
  const CastHtmlParts castHtmls = CastBrowserHelper::GetCastHtmlParts(record, imgHost, ICON_SIZE);
  mDetailsPane->SetCastHtmlParts(castHtmls);
  mDetailsPane->UpdateHtmlContents();
}

void FileFolderPreviewer::operator()(const QString& pth) { // file system view
  if (!NeedUpdate(pth)) {
    return;
  }
  mDetailsPane->StopPlay();
  mLastName = pth;
  setWindowTitle(mLastName);
  if (QFileInfo{pth}.isFile()) { // a file
    BeforeDisplayAFileDetail();
    mDetailsPane->UpdateWhenSelectAFile(pth);
    return;
  }
  BeforeDisplayAFolder();
  mImgVidOtherPane->operator()(pth);
}

void FileFolderPreviewer::operator()(const QString& name,
                                     const QString& jsonAbsFilePath,
                                     const QStringList& imgPthLst,
                                     const QStringList& vidsLst) { // scene view
  if (!NeedUpdate(name)) {
    return;
  }
  mDetailsPane->StopPlay();
  mLastName = name;
  setWindowTitle(mLastName);
  BeforeDisplayAFolder();
  mImgVidOtherPane->operator()(name, jsonAbsFilePath, imgPthLst, vidsLst);
}
