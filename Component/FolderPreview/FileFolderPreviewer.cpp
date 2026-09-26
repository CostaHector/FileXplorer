#include "FileFolderPreviewer.h"
#include "CastBrowserHelper.h"
#include "MovieDbHelper.h"
#include "PublicMacro.h"
#include "Configuration.h"
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
  if (Configuration().contains("Geometry/FLOATING_PREVIEW")) {
    restoreGeometry(Configuration().value("Geometry/FLOATING_PREVIEW").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 480, 1080));
  }
}

void FileFolderPreviewer::SaveSettings() {
  Configuration().setValue("Geometry/FLOATING_PREVIEW", saveGeometry());
}

void FileFolderPreviewer::StopVideoPlay() {
  if (mDetailsPane != nullptr) {
    mDetailsPane->onStopPlaying();
  }
  if (mImgVidOtherPane != nullptr) {
    mImgVidOtherPane->onStopPlaying();
  }
}

bool FileFolderPreviewer::DisplayFileInformation(const QString& pth) { // file system view
  if (!NeedUpdate(pth)) {
    return false;
  }
  StopVideoPlay();
  mLastName = pth;
  setWindowTitle(mLastName);
  if (QFileInfo{pth}.isFile()) { // a file
    CHECK_NULLPTR_RETURN_FALSE(mDetailsPane)
    BeforeDisplayAFileDetail();
    mDetailsPane->UpdateWhenSelectAFile(pth);
    return true;
  }
  CHECK_NULLPTR_RETURN_FALSE(mImgVidOtherPane)
  BeforeDisplayAFolder();
  mImgVidOtherPane->operator()(pth);
  return true;
}

bool FileFolderPreviewer::DisplayMovieInformation(const QSqlRecord& record) {
  CHECK_NULLPTR_RETURN_FALSE(mDetailsPane)
  mDetailsPane->setHtml("");
  QString newName = MovieDbHelper::GetWindowTitleName(record);
  if (!NeedUpdate(newName)) {
    return false;
  }
  mLastName = newName;
  setWindowTitle(mLastName);
  BeforeDisplayAFileDetail();
  const QString movieRecordHtmls = MovieDbHelper::GetDetailHtml(record);
  mDetailsPane->setHtml(movieRecordHtmls);
  return true;
}

bool FileFolderPreviewer::DisplayCastInformation(const QSqlRecord& record, const QString& imgHost) {
  CHECK_NULLPTR_RETURN_FALSE(mDetailsPane)
  mDetailsPane->setHtml("");
  QString newName = CastBrowserHelper::GetWindowTitleName(record);
  if (!NeedUpdate(newName)) {
    return false;
  }
  StopVideoPlay();
  mLastName = newName;
  setWindowTitle(mLastName);
  BeforeDisplayAFileDetail();
  QSize ICON_SIZE = mDetailsPane->iconSize();
  const CastHtmlParts castHtmls = CastBrowserHelper::GetCastHtmlParts(record, imgHost, ICON_SIZE);
  mDetailsPane->SetCastHtmlParts(castHtmls);
  mDetailsPane->UpdateHtmlContents();
  return true;
}

bool FileFolderPreviewer::DisplayJsonInformation(const QString& name, const QString& jsonAbsFilePath, const QStringList& imgPthLst, const QStringList& vidsLst) { // scene view
  CHECK_NULLPTR_RETURN_FALSE(mImgVidOtherPane)
  if (!NeedUpdate(name)) {
    return false;
  }
  StopVideoPlay();
  mLastName = name;
  setWindowTitle(mLastName);
  BeforeDisplayAFolder();
  mImgVidOtherPane->operator()(name, jsonAbsFilePath, imgPthLst, vidsLst);
  return true;
}
