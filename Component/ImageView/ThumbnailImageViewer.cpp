#include "ThumbnailImageViewer.h"
#include "ImageTool.h"
#include "PathTool.h"
#include "FileTool.h"
#include "PublicMacro.h"

#include <QPixmap>
#include <QMenu>

ThumbnailImageViewer::ThumbnailImageViewer(const QString& memoryKeyName, QWidget* parent) //
  : ImageViewer{memoryKeyName, parent} {
  mNaviToolBar = new QToolBar{"Navi toolbar", this};
  CHECK_NULLPTR_RETURN_VOID(mNaviToolBar);

  mNavigateIntoSub = mNaviToolBar->addAction("Navigate Into Subdirectory");
  CHECK_NULLPTR_RETURN_VOID(mNavigateIntoSub);
  mNavigateIntoSub->setCheckable(true);
  mNavigateIntoSub->setChecked(mImgIt.IsIncludingSubDirectory());

  m_prevButton = mNaviToolBar->addAction(QIcon{":img/PAGINATION_LAST"}, "");
  CHECK_NULLPTR_RETURN_VOID(m_prevButton);
  m_prevButton->setShortcut(QKeySequence(Qt::Key_Left));
  m_prevButton->setToolTip(QString("<b>%1 (%2)</b><br/>Go to previous image").arg(m_prevButton->text(), m_prevButton->shortcut().toString()));

  m_nextButton = mNaviToolBar->addAction(QIcon{":img/PAGINATION_NEXT"}, "");
  CHECK_NULLPTR_RETURN_VOID(m_nextButton);
  m_nextButton->setShortcut(QKeySequence(Qt::Key_Right));
  m_nextButton->setToolTip(QString("<b>%1 (%2)</b><br/>Go to next image").arg(m_nextButton->text(), m_nextButton->shortcut().toString()));

  subscribe();
}

void ThumbnailImageViewer::subscribe() {
  connect(GetLabel(), &QLabel::customContextMenuRequested, this, &ThumbnailImageViewer::onCustomContextMenuRequested);

  connect(m_prevButton, &QAction::triggered, this, &ThumbnailImageViewer::NavigateImagePrevious);
  connect(m_nextButton, &QAction::triggered, this, &ThumbnailImageViewer::NavigateImageNext);
  connect(mNavigateIntoSub, &QAction::toggled, this, &ThumbnailImageViewer::NavigateIntoSubdirectoryChanged);
}

QString ThumbnailImageViewer::GetImageAbsPath() const { //
  return PathTool::join(mParentPath, mRel2Image);
}

bool ThumbnailImageViewer::setPixmapByAbsFilePath(const QString& parentPath, const QString& rel2Img) {
  mParentPath = parentPath;
  mRel2Image = rel2Img;
  setFormatAndImgSizeBytes(PathTool::GetFormatInHar(mRel2Image), QFile{GetImageAbsPath()}.size());
  return UpdatePixmapAndTitle();
}

QPixmap ThumbnailImageViewer::GetPixmapCore() const {
  const QString imageAbsPath = GetImageAbsPath();
  QPixmap pm;
  if (!pm.load(imageAbsPath, mNoDotFormat.toStdString().c_str())) {
    LOG_W("Image load from path[%s] failed", qPrintable(imageAbsPath));
    return {};
  }
  return pm;
}

std::unique_ptr<QMovie> ThumbnailImageViewer::GetMovieCore(QSize& movieSize) const {
  const QString imageAbsPath = GetImageAbsPath();
  movieSize = ImageTool::GetImageDimensionPixel(imageAbsPath);
  return std::unique_ptr<QMovie>{new (std::nothrow) QMovie{imageAbsPath, mNoDotFormat.toUtf8()}};
}

bool ThumbnailImageViewer::NavigateImageCore(FolderNxtAndLastIterator::NaviDirection direction) {
  mImgIt(mParentPath);

  QString newImageName;
  switch (direction) {
    case FolderNxtAndLastIterator::NaviDirection::PREV:
      newImageName = mImgIt.last(mParentPath, mRel2Image);
      break;
    case FolderNxtAndLastIterator::NaviDirection::NEXT:
    default:
      newImageName = mImgIt.next(mParentPath, mRel2Image);
      break;
  }

  if (newImageName == mRel2Image) {
    LOG_D("No need navigate, remains[%s]", qPrintable(mRel2Image));
    return true;
  }
  return setPixmapByAbsFilePath(mParentPath, newImageName);
}

QString ThumbnailImageViewer::GetPathInfoInWinTitle() const {
  QString pathInfo;
  pathInfo += mRel2Image;
  pathInfo += " under: ";
  pathInfo += mParentPath;
  return pathInfo;
}

void ThumbnailImageViewer::NavigateIntoSubdirectoryChanged(bool bInclude) {
  mImgIt.setIncludingSubDirectory(bInclude);
  mImgIt(mParentPath, true); // force refresh images in folder structure
}

void ThumbnailImageViewer::onCustomContextMenuRequested(const QPoint& pos) {
  if (mMenu == nullptr) {
    mMenu = new (std::nothrow) QMenu{"Image viewer system menu", nullptr};
    CHECK_NULLPTR_RETURN_VOID(mMenu);

    _OPEN_IN_SYSTEM_APPLICATION = mMenu->addAction(QIcon{":img/LARGE"}, tr("Open in system application"));
    _REVEAL_IN_FILE_EXPLORER = mMenu->addAction(QIcon{":img/REVEAL_IN_EXPLORER"}, tr("Reveal in explorer"));
    _COPY_FILE_NAME = mMenu->addAction(QIcon{":img/COPY_TEXT"}, tr("Copy file name"));

    _OPEN_IN_SYSTEM_APPLICATION->setShortcutVisibleInContextMenu(true);
    _REVEAL_IN_FILE_EXPLORER->setShortcutVisibleInContextMenu(true);
    _COPY_FILE_NAME->setShortcutVisibleInContextMenu(true);

    connect(_OPEN_IN_SYSTEM_APPLICATION, &QAction::triggered, this, [this]() { FileTool::OpenLocalFileUsingDesktopService(GetImageAbsPath()); });
    connect(_REVEAL_IN_FILE_EXPLORER, &QAction::triggered, this, [this]() { FileTool::RevealInSystemExplorer(GetImageAbsPath()); });
    connect(_COPY_FILE_NAME, &QAction::triggered, this, [this]() { FileTool::CopyTextToSystemClipboard(GetImageAbsPath()); });
  }
  CHECK_NULLPTR_RETURN_VOID(mMenu);
  mMenu->popup(mapToGlobal(pos));
}
