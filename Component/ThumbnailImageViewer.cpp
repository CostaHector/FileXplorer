#include "ThumbnailImageViewer.h"
#include "StyleSheet.h"
#include "MemoryKey.h"
#include "PathTool.h"
#include "FileTool.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "DataFormatter.h"
#include "CastBrowserHelper.h"
#include "RateActions.h"
#include <QMovie>
#include <QPixmap>
#include <QWheelEvent>
#include <QPainter>
#include <QShortcut>

bool ThumbnailImageViewer::IsFileAbsPathImage(const QString& fileAbsPath) {
  const QString dotExt = PathTool::GetDotFileExtension(fileAbsPath);
  return TYPE_FILTER::isDotExtImage(dotExt);
}

bool ThumbnailImageViewer::IsFileImage(const QFileInfo& fi) {
  return IsFileAbsPathImage(fi.absoluteFilePath());
}

bool ThumbnailImageViewer::IsGifFile(const QString& fileAbsPath) {
  return fileAbsPath.endsWith(".gif", Qt::CaseInsensitive);
}

ThumbnailImageViewer::ThumbnailImageViewer(const QString& memoryKeyName, QWidget* parent) //
  : QScrollArea{parent}
  , //
  m_memoryKeyName{memoryKeyName} {
  int iconSizeIndexHint = Configuration().value(m_memoryKeyName + "_ICON_SIZE_INDEX", mCurIconScaledSizeIndex).toInt();
  setIconSizeScaledIndex(iconSizeIndexHint);

  mNavigateIntoSub = new (std::nothrow) QCheckBox{"Navigate Into Subdirectory", this};
  mNavigateIntoSub->setChecked(mImgIt.IsIncludingSubDirectory());

  auto& rateInst = RateActions::GetInst();
  mControlToolBar = new (std::nothrow) QToolBar{"Recusive Navigate and Rate", this};
  mControlToolBar->addWidget(mNavigateIntoSub);
  mControlToolBar->addSeparator();
  mControlToolBar->addActions(rateInst.RATE_AGS->actions());

  m_prevButton = new (std::nothrow) QPushButton{QIcon{":img/PAGINATION_LAST"}, "", this};
  CHECK_NULLPTR_RETURN_VOID(m_prevButton);
  m_prevButton->setShortcut(QKeySequence(Qt::Key_Left));
  m_prevButton->setToolTip(
      QString("<b>%1 (%2)</b><br/>Go to previous image").arg(m_prevButton->text(), m_prevButton->shortcut().toString()));

  m_nextButton = new (std::nothrow) QPushButton{QIcon{":img/PAGINATION_NEXT"}, "", this};
  CHECK_NULLPTR_RETURN_VOID(m_nextButton);
  m_nextButton->setShortcut(QKeySequence(Qt::Key_Right));
  m_nextButton->setToolTip(QString("<b>%1 (%2)</b><br/>Go to next image").arg(m_nextButton->text(), m_nextButton->shortcut().toString()));

  mLabel = new (std::nothrow) QLabel{this};
  CHECK_NULLPTR_RETURN_VOID(mLabel);
  mLabel->setAlignment(Qt::AlignCenter);
  mLabel->setContextMenuPolicy(Qt::CustomContextMenu);

  setWidget(mLabel);
  setWidgetResizable(true);

  setAlignment(Qt::AlignCenter);
  setBackgroundRole(QPalette::Dark);
  if (this->parent() == nullptr) {
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
  }

  setWindowIcon(QIcon{":img/IMAGE"});
  ReadSetting();
  {
    connect(m_prevButton, &QPushButton::clicked, this, &ThumbnailImageViewer::NavigateImagePrevious);
    connect(m_nextButton, &QPushButton::clicked, this, &ThumbnailImageViewer::NavigateImageNext);
    connect(mNavigateIntoSub, &QCheckBox::toggled, this, &ThumbnailImageViewer::NavigateIntoSubdirectoryChanged);
    connect(mLabel, &QLabel::customContextMenuRequested, this, &ThumbnailImageViewer::onCustomContextMenuRequested);

    QShortcut *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, &ThumbnailImageViewer::close);
  }
}

ThumbnailImageViewer::~ThumbnailImageViewer() {
  if (mPMovie) {
    mPMovie->stop();
  }
  Configuration().setValue(m_memoryKeyName + "_GEOMETRY", saveGeometry());
  Configuration().setValue(m_memoryKeyName + "_ICON_SIZE_INDEX", mCurIconScaledSizeIndex);
}

void ThumbnailImageViewer::ReadSetting() {
  if (Configuration().contains(m_memoryKeyName + "_GEOMETRY")) {
    restoreGeometry(Configuration().value(m_memoryKeyName + "_GEOMETRY").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}

void ThumbnailImageViewer::resizeEvent(QResizeEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  QScrollArea::resizeEvent(event);
  adjustButtonPosition();
}

void ThumbnailImageViewer::showEvent(QShowEvent* event) {
  QScrollArea::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void ThumbnailImageViewer::adjustButtonPosition() {
  static constexpr int margin = 15;
  const int buttonHeight = height() / 3;
  const int yPos = (height() - buttonHeight) / 2;

  if (m_prevButton != nullptr) {
    m_prevButton->setFixedHeight(buttonHeight);
    int prevX = margin;
    m_prevButton->move(prevX, yPos);
  }

  if (m_nextButton != nullptr) {
    m_nextButton->setFixedHeight(buttonHeight);
    int nextX = width() - m_nextButton->width() - margin;
    m_nextButton->move(nextX, yPos);
  }
}

bool ThumbnailImageViewer::refreshPixmapSize() {
  return UpdatePixmapAndTitle();
}

bool ThumbnailImageViewer::setPixmapByByteArrayData(const QByteArray& dataByteArray) {
  mImageFrom = ImageFrom::ARCHIVE;
  mDataFromArchive = dataByteArray;

  return UpdatePixmapAndTitle();
}

QString ThumbnailImageViewer::FromPath::GetImageAbsPath() const { //
  return PathTool::join(parentPath, rel2image);
}

bool ThumbnailImageViewer::setPixmapByAbsFilePath(const QString& parentPath, const QString& rel2Img) {
  mImageFrom = ImageFrom::PATH;
  mDataFromPath = FromPath{parentPath, rel2Img, QFile(PathTool::join(parentPath, rel2Img)).size()};
  return UpdatePixmapAndTitle();
}

bool ThumbnailImageViewer::GetPixmap(QPixmap& pm, QString& winTitle) const {
  winTitle.reserve(260 + 80);
  qint64 bytesOcuppied{0};
  switch (mImageFrom) {
    case ImageFrom::ARCHIVE: {
      if (!pm.loadFromData(mDataFromArchive)) {
        LOG_W("Image load from bytearray failed");
        return false;
      }
      bytesOcuppied = mDataFromArchive.size();
      winTitle += "Archive";
      break;
    }
    case ImageFrom::PATH: {
      if (!pm.load(mDataFromPath.GetImageAbsPath())) {
        LOG_W("Image load from path[%s/%s] failed", qPrintable(mDataFromPath.parentPath), qPrintable(mDataFromPath.rel2image));
        return false;
      }
      bytesOcuppied = mDataFromPath.imageBytes;
      winTitle += mDataFromPath.rel2image;
      winTitle += " under: ";
      winTitle += mDataFromPath.parentPath;
      break;
    }
    default: {
      LOG_W("ImageFrom[%d] not support", (int) mImageFrom);
      return false;
    }
  }
  if (pm.isNull()) {
    LOG_W("Pixmap[%d] is null");
    return false;
  }

  winTitle += " [Resolution:";
  winTitle += QString::number(pm.width());
  winTitle += "x";
  winTitle += QString::number(pm.height());

  const Qt::TransformationMode transformMode{bytesOcuppied < 10 * 1024 * 1024 ? Qt::SmoothTransformation : Qt::FastTransformation};
  if (pm.width() * mHeight >= pm.height() * mWidth) {
    pm = pm.scaledToWidth(mWidth, transformMode);
  } else {
    pm = pm.scaledToHeight(mHeight, transformMode);
  }

  winTitle += " -> ";
  winTitle += QString::number(pm.width());
  winTitle += "x";
  winTitle += QString::number(pm.height());
  winTitle += " Size: ";
  winTitle += DataFormatter::formatFileSizeWithBytes(bytesOcuppied);
  winTitle += "]";

  return true;
}

QMovie* ThumbnailImageViewer::GetMovie(QString& winTitle) const {
  const QString imageAbsPath = mDataFromPath.GetImageAbsPath();
  QMovie* preloaderAnimation = new (std::nothrow) QMovie{imageAbsPath, ""};
  CHECK_NULLPTR_RETURN_NULLPTR(preloaderAnimation);
  if (!preloaderAnimation->isValid()) {
    LOG_W("file[%s] is no a movie", qPrintable(imageAbsPath));
    return nullptr;
  }

  const QSize sz{CastBrowserHelper::GetImageSize(imageAbsPath)};
  QSize destSz;
  if (sz.width() * mHeight >= sz.height() * mWidth) {
    destSz = QSize{mWidth, (sz.height() * mWidth) / sz.width()};
  } else {
    destSz = QSize{(sz.width() * mHeight) / sz.height(), mHeight};
  }
  preloaderAnimation->setScaledSize(destSz);

  preloaderAnimation->start();

  winTitle += mDataFromPath.rel2image;
  winTitle += " under: ";
  winTitle += mDataFromPath.parentPath;
  winTitle += " [GIF, Frames: ";
  winTitle += QString::number(preloaderAnimation->frameCount());
  winTitle += ", Speed: ";
  winTitle += QString::number(preloaderAnimation->speed());
  winTitle += "ms, Size: ";
  winTitle += DataFormatter::formatFileSizeWithBytes(mDataFromPath.imageBytes);
  winTitle += "]";
  return preloaderAnimation;
}

bool ThumbnailImageViewer::UpdatePixmapAndTitle() {
  clearPixmap();
  QString winTitle;
  if (mImageFrom == ImageFrom::PATH && IsGifFile(mDataFromPath.rel2image)) {
    mPMovie.reset(GetMovie(winTitle));
    if (mPMovie == nullptr) {
      return false;
    }
    mLabel->setMovie(mPMovie.get());
  } else {
    QPixmap pm;
    if (!GetPixmap(pm, winTitle)) {
      return false;
    }
    mLabel->setPixmap(pm);
  }
  setWindowTitle(winTitle);
  return true;
}

bool ThumbnailImageViewer::setIconSizeScaledIndex(int newScaledIndex) {
  if (newScaledIndex < 0 || newScaledIndex >= IMAGE_SIZE::ICON_SIZE_CANDIDATES_N) {
    return false;
  }
  mCurIconScaledSizeIndex = newScaledIndex;
  mHeight = IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconScaledSizeIndex].height();
  mWidth = IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconScaledSizeIndex].width();
  return true;
}

void ThumbnailImageViewer::clearPixmap() {
  mLabel->clear();
}

bool ThumbnailImageViewer::NavigateImageCore(FolderNxtAndLastIterator::NaviDirection direction) {
  if (mImageFrom != ImageFrom::PATH) {
    return false;
  }
  mImgIt(mDataFromPath.parentPath);

  QString newImageName;
  if (direction == FolderNxtAndLastIterator::NaviDirection::NEXT) {
    newImageName = mImgIt.next(mDataFromPath.parentPath, mDataFromPath.rel2image);
  } else {
    newImageName = mImgIt.last(mDataFromPath.parentPath, mDataFromPath.rel2image);
  }
  if (newImageName == mDataFromPath.rel2image) {
    LOG_D("No need navigate, remains[%s]", qPrintable(mDataFromPath.rel2image));
    return true;
  }
  return setPixmapByAbsFilePath(mDataFromPath.parentPath, newImageName);
}

bool ThumbnailImageViewer::NavigateIntoSubdirectoryChanged(bool bInclude) {
  if (mImageFrom != ImageFrom::PATH) {
    LOG_D("Not from path, skip");
    return false;
  }
  mImgIt.setIncludingSubDirectory(bInclude);
  mImgIt(mDataFromPath.parentPath, true); // force refresh images in folder structure
  return true;
}

void ThumbnailImageViewer::onCustomContextMenuRequested(const QPoint& pos) {
  if (mMenu == nullptr) {
    _OPEN_IN_SYSTEM_APPLICATION = new (std::nothrow) QAction{QIcon{":img/LARGE"}, tr("Open in system application"), this};
    CHECK_NULLPTR_RETURN_VOID(_OPEN_IN_SYSTEM_APPLICATION);
    _OPEN_IN_SYSTEM_APPLICATION->setShortcutVisibleInContextMenu(true);

    _REVEAL_IN_FILE_EXPLORER = new (std::nothrow) QAction{QIcon{":img/REVEAL_IN_EXPLORER"}, tr("Reveal in explorer"), this};
    CHECK_NULLPTR_RETURN_VOID(_REVEAL_IN_FILE_EXPLORER);
    _REVEAL_IN_FILE_EXPLORER->setShortcutVisibleInContextMenu(true);

    _COPY_FILE_NAME = new (std::nothrow) QAction{QIcon{":img/COPY_TEXT"}, tr("Copy file name"), this};
    CHECK_NULLPTR_RETURN_VOID(_COPY_FILE_NAME);
    _COPY_FILE_NAME->setShortcutVisibleInContextMenu(true);

    mMenu = new (std::nothrow) QMenu{"Image viewer system menu", nullptr};
    CHECK_NULLPTR_RETURN_VOID(mMenu);
    mMenu->addAction(_OPEN_IN_SYSTEM_APPLICATION);
    mMenu->addAction(_REVEAL_IN_FILE_EXPLORER);
    mMenu->addAction(_COPY_FILE_NAME);

    connect(_OPEN_IN_SYSTEM_APPLICATION, &QAction::triggered, this, [this]() {
      if (mImageFrom == ImageFrom::PATH) {
        FileTool::OpenLocalFileUsingDesktopService(mDataFromPath.GetImageAbsPath());
      }
    });
    connect(_REVEAL_IN_FILE_EXPLORER, &QAction::triggered, this, [this]() {
      if (mImageFrom == ImageFrom::PATH) {
        FileTool::RevealInSystemExplorer(mDataFromPath.GetImageAbsPath());
      }
    });
    connect(_COPY_FILE_NAME, &QAction::triggered, this, [this]() { //
      if (mImageFrom == ImageFrom::PATH) {
        FileTool::CopyTextToSystemClipboard(mDataFromPath.GetImageAbsPath());
      }
    });
  }
  CHECK_NULLPTR_RETURN_VOID(mMenu);
  mMenu->popup(mapToGlobal(pos));
}

void ThumbnailImageViewer::wheelEvent(QWheelEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  if (event->modifiers() == Qt::ControlModifier) {
    QPoint numDegrees = event->angleDelta() / 8;
    if (!numDegrees.isNull()) {
      int numSteps = numDegrees.y() / 15;
      int newSizeIndex = mCurIconScaledSizeIndex + (numSteps > 0 ? 1 : -1);
      if (!setIconSizeScaledIndex(newSizeIndex)) {
        event->ignore();
        return;
      }
      refreshPixmapSize();
      emit onImageScaledIndexChanged(mCurIconScaledSizeIndex);
      event->accept();
      return;
    }
  }
  QScrollArea::wheelEvent(event);
}
