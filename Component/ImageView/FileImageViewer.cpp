#include "FileImageViewer.h"
#include "ImageTool.h"
#include "PathTool.h"
#include "FileTool.h"
#include "RelatedHelper.h"
#include "RecycleCfmDlg.h"
#include "ImageViewerActions.h"
#include "FileBatchOps.h"
#include "FileOperatorPub.h"
#include "UndoRedo.h"
#include "FileOpActs.h"
#include "PublicCheckMacro.h"
#include "NotificatorMacro.h"

#include <QPixmap>
#include <QMenu>

void SetNavigateButtonNeedTransparent(QToolBar* toolBar) {
  CHECK_NULLPTR_RETURN_VOID(toolBar);
  for (QAction* pAct: toolBar->actions()) {
    if (pAct->isSeparator()) {
      continue;
    }
    if (QWidget *widget = toolBar->widgetForAction(pAct)) {
      widget->setProperty("needTransparentFlag", true);
    }
  }
}

FileImageViewer::FileImageViewer(const QString& memoryKeyName, QWidget* parent) //
  : ImageViewer{memoryKeyName, parent},
  mImgIt{FolderNxtAndLastIterator::GetInstsNaviImages(ImageViewerActions::GetInst().isNavigateIntoSubFoldersWhenIterate())} {

  auto& inst = ImageViewerActions::GetInst();
  mNaviToolBar->addAction(inst.NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE_ACT);

  m_prevButton = mNaviToolBar->addAction(QIcon{":img/PAGINATION_LAST"}, "Previous Image");
  CHECK_NULLPTR_RETURN_VOID(m_prevButton);
  m_prevButton->setShortcut(QKeySequence(Qt::Key_PageUp));
  m_prevButton->setToolTip(QString("<b>%1 (%2)</b><br/>Go to previous image").arg(m_prevButton->text(), m_prevButton->shortcut().toString()));

  m_nextButton = mNaviToolBar->addAction(QIcon{":img/PAGINATION_NEXT"}, "Next Image");
  CHECK_NULLPTR_RETURN_VOID(m_nextButton);
  m_nextButton->setShortcut(QKeySequence(Qt::Key_PageDown));
  m_nextButton->setToolTip(QString("<b>%1 (%2)</b><br/>Go to next image").arg(m_nextButton->text(), m_nextButton->shortcut().toString()));

  mNaviToolBar->addSeparator();
  mRenameImageRelatedFilesAct = mNaviToolBar->addAction(QIcon{":img/NAME_STR_REPLACER_PATH"}, "Rename(Replace)");
  CHECK_NULLPTR_RETURN_VOID(mRenameImageRelatedFilesAct);
  mRenameImageRelatedFilesAct->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key::Key_M));
  mRenameImageRelatedFilesAct->setToolTip(QString("<b>%1 (%2)</b><br/>Rename current image and its related files.").arg(mRenameImageRelatedFilesAct->text(), mRenameImageRelatedFilesAct->shortcut().toString()));

  mNaviToolBar->addSeparator();
  m_recycleThisImageButton = mNaviToolBar->addAction(QIcon{":img/MOVE_TO_TRASH_BIN"}, "Recycle");
  CHECK_NULLPTR_RETURN_VOID(m_recycleThisImageButton);
  m_recycleThisImageButton->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_D));
  m_recycleThisImageButton->setShortcutVisibleInContextMenu(true);
  m_recycleThisImageButton->setToolTip(
      QString("<b>%1 (%2)</b><br/>Move the current image to the trash bin.")
          .arg(m_recycleThisImageButton->text(),
               m_recycleThisImageButton->shortcut().toString())
      );

  m_recyleThisImageAndRelatedButton = mNaviToolBar->addAction(QIcon{":img/MOVE_TO_TRASH_BIN_INCLUDE_RELATED"}, "Recyle Related");
  CHECK_NULLPTR_RETURN_VOID(m_recyleThisImageAndRelatedButton);
  m_recyleThisImageAndRelatedButton->setShortcut(QKeySequence(Qt::Key_Delete));
  m_recyleThisImageAndRelatedButton->setShortcutVisibleInContextMenu(true);
  m_recyleThisImageAndRelatedButton->setToolTip(
      QString("<b>%1 (%2)</b><br/>Delete the current image along with all associated sidecar files (e.g., .xmp, .pp3, or paired RAW+JPEG).")
          .arg(m_recyleThisImageAndRelatedButton->text(),
               m_recyleThisImageAndRelatedButton->shortcut().toString())
      );

  mNaviToolBar->addSeparator();
  mNaviToolBar->addActions(FileOpActs::GetInst().UNDO_REDO_RIBBONS->actions());

  // SetNavigateButtonNeedTransparent(mNaviToolBar);

  subscribe();
}

void FileImageViewer::subscribe() {
  connect(GetLabel(), &QLabel::customContextMenuRequested, this, &FileImageViewer::onCustomContextMenuRequested);

  connect(m_prevButton, &QAction::triggered, this, &FileImageViewer::NavigateImagePrevious);
  connect(m_nextButton, &QAction::triggered, this, &FileImageViewer::NavigateImageNext);
  connect(this, &ImageViewer::reqPreviousImage, this, &FileImageViewer::NavigateImagePrevious);
  connect(this, &ImageViewer::reqNextImage, this, &FileImageViewer::NavigateImageNext);

  connect(ImageViewerActions::GetInst().NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE_ACT, &QAction::toggled, this, &FileImageViewer::NavigateIntoSubdirectoryChanged);

  connect(mRenameImageRelatedFilesAct, &QAction::triggered, this, &FileImageViewer::on_RenameReplace);
  connect(m_recycleThisImageButton, &QAction::triggered, this, &FileImageViewer::RecycleCurrentImage);
  connect(m_recyleThisImageAndRelatedButton, &QAction::triggered, this, &FileImageViewer::RecycleCurrentImageAndItsRelated);
}

QString FileImageViewer::GetImageAbsPath() const { //
  return PathTool::Path2Join(mParentPath, mRel2Image);
}

bool FileImageViewer::setPixmapByAbsFilePath(const QString& parentPath, const QString& rel2Img) {
  mParentPath = parentPath;
  mRel2Image = rel2Img;  
  setFormatAndImgSizeBytes(PathTool::GetFormatInHar(mRel2Image), QFile{GetImageAbsPath()}.size());

  GetLabel()->setBaseImageFullPath(GetImageAbsPath());
  return UpdatePixmapAndTitle();
}

QPixmap FileImageViewer::GetPixmapCore() const {
  const QString imageAbsPath = GetImageAbsPath();
  QPixmap pm;
  if (!pm.load(imageAbsPath, mNoDotFormat.toStdString().c_str())) {
    LOG_W("Image load from path[%s] failed", qPrintable(imageAbsPath));
    return {};
  }
  return pm;
}

std::unique_ptr<QMovie> FileImageViewer::GetMovieCore(QSize& movieSize) const {
  const QString imageAbsPath = GetImageAbsPath();
  movieSize = ImageTool::GetImageDimensionPixel(imageAbsPath);
  return std::unique_ptr<QMovie>{new (std::nothrow) QMovie{imageAbsPath, mNoDotFormat.toUtf8()}};
}

bool FileImageViewer::NavigateImageCore(FolderNxtAndLastIterator::NaviDirection direction) {
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

QString FileImageViewer::GetPathInfoInWinTitle() const {
  QString pathInfo;
  pathInfo += mRel2Image;
  pathInfo += " under: ";
  pathInfo += mParentPath;
  return pathInfo;
}

void FileImageViewer::NavigateIntoSubdirectoryChanged(bool bInclude) {
  mImgIt.setIncludingSubDirectory(bInclude);
  mImgIt(mParentPath, true); // force refresh images in folder structure
}

bool AfterRecycle(FileBatchOps::OpStatus recycleStatus, std::function<void()> pNaviFunc) {
  switch (recycleStatus) {
    case FileBatchOps::OpStatus::PARTIAL_FAILED: return false;
    case FileBatchOps::OpStatus::SKIP: return true;
    case FileBatchOps::OpStatus::ALL_SUCCEED: {
      if (pNaviFunc) {
        pNaviFunc();
      }
      return true;
    }
    default:
      return false;
  }
}

bool FileImageViewer::RecycleCurrentImage() {
  const QString currentImageAbsPath = GetImageAbsPath();
  FileBatchOps::OpStatus recycleStatus = FileBatchOps::MoveToTrashSelected("", {currentImageAbsPath});
  return AfterRecycle(recycleStatus, [this](){ NavigateImageNext(); });
}

bool FileImageViewer::RecycleCurrentImageAndItsRelated() {
  FileBatchOps::OpStatus recycleStatus{FileBatchOps::OpStatus::SKIP};
  const QString currentImageAbsPath = GetImageAbsPath();
  QString jsonPath;
  if (!RelatedHelper::getJsonPathFromFile(currentImageAbsPath, jsonPath)) {
    // no json find. no other need recycle, only recycle itself
    recycleStatus = FileBatchOps::MoveToTrashSelected("", {currentImageAbsPath});
  } else {
    QString jsonLocatedInPath;
    QString jsonFileName = PathTool::GetPrepathAndFileName(jsonPath, jsonLocatedInPath);
    const FileBatchOps::OpOutcome recycleOutcome = FileBatchOps::MoveToTrashSelectedWithRelated(jsonLocatedInPath, {jsonFileName});
    recycleStatus = recycleOutcome.result;
  }
  return AfterRecycle(recycleStatus, [this](){ NavigateImageNext(); });
}

bool FileImageViewer::on_RenameReplace() {
  const QString currentImageAbsPath = GetImageAbsPath();
  QString jsonPath;
  if (!RelatedHelper::getJsonPathFromFile(currentImageAbsPath, jsonPath)) {
    // no json find. no other need recycle, only rename itself
    return FileBatchOps::RenameSelectedReplace(mParentPath, {mRel2Image}) != FileBatchOps::OpStatus::PARTIAL_FAILED;
  }
  QString jsonLocatedInPath;
  QString jsonFileName = PathTool::GetPrepathAndFileName(jsonPath, jsonLocatedInPath);
  int renameCnt = FileBatchOps::RenameWithPatternReplace(jsonLocatedInPath, {jsonFileName});
  return renameCnt >= 0;
}

void FileImageViewer::onCustomContextMenuRequested(const QPoint& pos) {
  if (mMenu == nullptr) {
    mMenu = new (std::nothrow) QMenu{"Image viewer system menu", nullptr};
    CHECK_NULLPTR_RETURN_VOID(mMenu);

    _OPEN_IN_SYSTEM_APPLICATION = mMenu->addAction(QIcon{":img/SYSTEM_APPLICATION"}, tr("Open in system application"));
    _REVEAL_IN_FILE_EXPLORER = mMenu->addAction(QIcon{":img/REVEAL_IN_EXPLORER"}, tr("Reveal in explorer"));
    _COPY_FILE_NAME = mMenu->addAction(QIcon{":img/COPY_TEXT"}, tr("Copy file name"));
    mMenu->addSeparator();
    mMenu->addAction(m_recycleThisImageButton);
    mMenu->addAction(m_recyleThisImageAndRelatedButton);

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
