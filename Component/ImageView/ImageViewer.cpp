#include "ImageViewer.h"
#include "ImageTool.h"
#include "Configuration.h"
#include "PublicMacro.h"
#include "SizeTool.h"
#include "StyleSheet.h"
#include "DataFormatter.h"
#include "Logger.h"

#include <QScrollBar>
#include <QFileInfo>
#include <QShortcut>
#include <QWheelEvent>
#include <QShortcut>
#include <QImageReader>

bool ImageViewer::IsFileImage(const QFileInfo& fi) {
  return ImageTool::IsFileAbsPathImage(fi.absoluteFilePath());
}

ImageViewer::ImageViewer(const QString& memoryKeyName, QWidget* parent) //
  : QScrollArea{parent}
  , m_memoryKeyName{memoryKeyName} {
  setIconSizeScaledIndex(IMAGE_SIZE::GetInitialScaledSize(GetName()));

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
  setFocusPolicy(Qt::StrongFocus);

  setWindowIcon(QIcon{":img/IMAGE"});
  ReadSetting();
  subscribe();
}

void ImageViewer::subscribe() {
  if (QShortcut* escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this)) {
    connect(escShortcut, &QShortcut::activated, this, &ImageViewer::close);
  }
}

ImageViewer::~ImageViewer() {
  if (mPMovie) {
    mPMovie->stop();
  }
  Configuration().setValue(m_memoryKeyName + "/GEOMETRY", saveGeometry());
  IMAGE_SIZE::SaveInitialScaledSize(m_memoryKeyName, mCurIconScaledSizeIndex);
}

void ImageViewer::showEvent(QShowEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  QScrollArea::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void ImageViewer::wheelEvent(QWheelEvent* event) {
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

void ImageViewer::keyPressEvent(QKeyEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  QScrollBar* scrollBar{nullptr};
  if (event->modifiers() == Qt::NoModifier) {
    switch (event->key()) {
      case Qt::Key_Left:
      case Qt::Key_Right: {
        scrollBar = horizontalScrollBar();
        break;
      }
      case Qt::Key_Up:
      case Qt::Key_Down: {
        scrollBar = verticalScrollBar();
        break;
      }
      default:
        break;
    }
  }
  if (scrollBar != nullptr && !scrollBar->isHidden()) {
    static constexpr int pixel = 30; // each up/down/left/right key pixels count
    const int delta{event->key() == Qt::Key_Left || event->key() == Qt::Key_Up ? -pixel : pixel};
    // clamp
    const int modifiedToValue{qMax(scrollBar->minimum(), qMin(scrollBar->value() + delta, scrollBar->maximum()))};
    scrollBar->setValue(modifiedToValue);
    event->accept();
    return;
  }
  QScrollArea::keyPressEvent(event);
}

bool ImageViewer::UpdatePixmapAndTitle() {
  clearPixmap();

  QString winTitle;
  winTitle.reserve(260 + 80);
  winTitle += GetPathInfoInWinTitle();

  if (isCurImageGif()) {
    mPMovie = GetMovie(winTitle);
    if (!mPMovie) {
      return false;
    }
    mLabel->setMovie(mPMovie.get());
  } else {
    QPixmap pm = GetPixmap(winTitle);
    if (pm.isNull()) {
      return false;
    }
    mLabel->setPixmap(pm);
  }
  setWindowTitle(winTitle);
  return true;
}

QSize ImageViewer::getDestSize(QSize sz) const {
  if (sz.width() * mHeight >= sz.height() * mWidth) {
    return {mWidth, (sz.height() * mWidth) / sz.width()};
  } else {
    return {(sz.width() * mHeight) / sz.height(), mHeight};
  }
}

QString ImageViewer::GetResolutionAndSizeInWinTitle(const QSize& oldSz, const QSize& dstSz) const {
  QString winTitle;
  winTitle.reserve(80);
  winTitle += " [Resolution:";
  winTitle += QString::number(oldSz.width());
  winTitle += "x";
  winTitle += QString::number(oldSz.height());
  winTitle += " -> ";
  winTitle += QString::number(dstSz.width());
  winTitle += "x";
  winTitle += QString::number(dstSz.height());
  winTitle += " Size: ";
  winTitle += DataFormatter::formatFileSizeWithBytes(GetImageFileSize());
  winTitle += "]";
  return winTitle;
}

QPixmap ImageViewer::GetPixmap(QString& winTitle) const {
  QPixmap pm{GetPixmapCore()};
  if (pm.isNull()) {
    return {};
  }
  const QSize oldSz{pm.size()};
  if (!oldSz.isValid() || oldSz.width() <= 0 && oldSz.height() <= 0) {
    return {};
  }

  const qint64 bytesOcuppied{GetImageFileSize()};
  const Qt::TransformationMode transformMode{bytesOcuppied < 10 * 1024 * 1024 ? Qt::SmoothTransformation : Qt::FastTransformation};
  const QSize dstSz{getDestSize(oldSz)};
  pm = pm.scaled(dstSz, Qt::IgnoreAspectRatio, transformMode);

  winTitle += GetResolutionAndSizeInWinTitle(oldSz, dstSz);
  return pm;
}

std::unique_ptr<QMovie> ImageViewer::GetMovie(QString& winTitle) const {
  QSize oldSz{-1, -1};
  std::unique_ptr<QMovie> preloaderAnimation{GetMovieCore(oldSz)};
  if (!preloaderAnimation || !preloaderAnimation->isValid()) {
    return nullptr;
  }
  if (!oldSz.isValid() || oldSz.width() <= 0 && oldSz.height() <= 0) {
    return nullptr;
  }
  const QSize dstSz{getDestSize(oldSz)};

  winTitle += GetResolutionAndSizeInWinTitle(oldSz, dstSz);

  winTitle += " [GIF, Frames: ";
  winTitle += QString::number(preloaderAnimation->frameCount());
  winTitle += ", Speed: ";
  winTitle += QString::number(preloaderAnimation->speed());
  winTitle += "ms]";

  preloaderAnimation->setScaledSize(dstSz);
  preloaderAnimation->start();
  return preloaderAnimation;
}

void ImageViewer::ReadSetting() {
  if (Configuration().contains(m_memoryKeyName + "/GEOMETRY")) {
    restoreGeometry(Configuration().value(m_memoryKeyName + "/GEOMETRY").toByteArray());
  } else {
    setGeometry(SizeTool::DEFAULT_GEOMETRY);
  }
}

bool ImageViewer::setIconSizeScaledIndex(int newScaledIndex) {
  if (newScaledIndex < 0 || newScaledIndex >= IMAGE_SIZE::ICON_SIZE_CANDIDATES_N) {
    return false;
  }
  mCurIconScaledSizeIndex = newScaledIndex;
  mHeight = IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconScaledSizeIndex].height();
  mWidth = IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconScaledSizeIndex].width();
  return true;
}
