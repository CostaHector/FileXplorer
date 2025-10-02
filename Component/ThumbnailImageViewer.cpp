#include "ThumbnailImageViewer.h"
#include "StyleSheet.h"
#include "MemoryKey.h"
#include <QWheelEvent>

ThumbnailImageViewer::ThumbnailImageViewer(const QString& memoryKeyName, QWidget* parent) : QLabel{parent}, m_memoryKeyName{memoryKeyName} {
  int iconSizeIndexHint = Configuration().value(m_memoryKeyName + "_ICON_SIZE_INDEX", mCurIconScaledSizeIndex).toInt();
  setIconSizeScaledIndex(iconSizeIndexHint);
}

ThumbnailImageViewer::~ThumbnailImageViewer() {
  LOG_E("descontruct");
  Configuration().setValue(m_memoryKeyName + "_ICON_SIZE_INDEX", mCurIconScaledSizeIndex);
}

void ThumbnailImageViewer::refreshPixmapSize() {
  setPixmapByByteArrayData(m_curDataByteArray);
}

void ThumbnailImageViewer::setPixmapByByteArrayData(const QByteArray& dataByteArray) {
  m_curDataByteArray = dataByteArray;
  QPixmap pm;
  pm.loadFromData(dataByteArray);
  if (pm.width() * mHeight >= pm.height() * mWidth) {
    pm = pm.scaledToWidth(mWidth, Qt::FastTransformation);
  } else {
    pm = pm.scaledToHeight(mHeight, Qt::FastTransformation);
  }
  setPixmap(pm);
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
  clear();
  m_curDataByteArray.clear();
}

void ThumbnailImageViewer::wheelEvent(QWheelEvent* event) {
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
  ThumbnailImageViewer::wheelEvent(event);
}
