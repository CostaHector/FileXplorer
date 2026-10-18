#include "SelectableLabel.h"
#include <QPixmap>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QFileDialog>
#include "NotificatorMacro.h"
#include "FaceRecognizeGuest.h"
#include "PathTool.h"

SelectableLabel::SelectableLabel(QWidget* parent) : QLabel{parent} {
  mFrame = new SelectionFrame(this);
  mFrame->hide();

  connect(mFrame, &SelectionFrame::faceRecognizeRequested, this, &SelectableLabel::onFaceRecognizeRequested);
  connect(mFrame, &SelectionFrame::closeRequested, this, &SelectableLabel::cancelSelection);
  connect(mFrame, &SelectionFrame::saveRequested, this, &SelectableLabel::onSaveRequested);
}

void SelectableLabel::onSaveRequested() {
  LazyUpdateRectSelectionImage();
  if (mRectSelectedPixmap.isNull()) return;
  // call save dialog then hide rectangle select frame.
  SaveRectSelectionImage(mRectSelectedPixmap, mSourceImageFileFullPath);
  cancelSelection();
}

void SelectableLabel::onFaceRecognizeRequested() {
  LazyUpdateRectSelectionImage();
  if (mRectSelectedPixmap.isNull()) return;
  auto& inst = FaceRecognizeGuest::GetInst();
  inst.PostRecognize(mRectSelectedPixmap, mFrame->GetFaceRecognizeResultLabel(), 5);
}

void SelectableLabel::setSelectionEnabled(bool on) {
  mSelectionEnabled = on;
  setCursor(on ? Qt::CrossCursor : Qt::ArrowCursor);
  if (!on) {
    mIsDragging = false;
    cancelSelection();
  }
}

void SelectableLabel::cancelSelection() {
  mFrame->hide();
  mRectSelectedPixmap = QPixmap();
  update();
}

void SelectableLabel::mousePressEvent(QMouseEvent* e) {
  if (!mSelectionEnabled || e->button() != Qt::LeftButton) {
    QLabel::mousePressEvent(e);
    return;
  }

  mIsDragging = true;
  mOrigin = mCurrent = e->pos();
  update();
}

void SelectableLabel::mouseMoveEvent(QMouseEvent* e) {
  if (!mIsDragging) { QLabel::mouseMoveEvent(e); return; }
  mCurrent = e->pos();
  update();
}

void SelectableLabel::LazyUpdateRectSelectionImage() {
  if (!mIsNeedUpdateRectSelectedPixmap) {
    return;
  }
  mRectSelectedPixmap = pixmap(Qt::ReturnByValueConstant::ReturnByValue).copy(mRelativeRect);
  mIsNeedUpdateRectSelectedPixmap = false;
}

void SelectableLabel::mouseReleaseEvent(QMouseEvent* e) {
  if (!mIsDragging || e->button() != Qt::LeftButton) {
    QLabel::mouseReleaseEvent(e);
    return;
  }
  mIsDragging = false;          // 拖动结束
  mCurrent = e->pos();
  update();

  const QRect visualSelRect = visSelectionRect().intersected(contentsRect());

  const QRect pixRect = pixmapRectInLabel();
  const QRect selInPix = visualSelRect.intersected(pixRect);
  if (selInPix.isEmpty()) return;

  const QPixmap full = pixmap(Qt::ReturnByValueConstant::ReturnByValue);
  if (full.isNull()) return;

  mRelativeRect = selInPix.translated(-pixRect.topLeft()).intersected(QRect(QPoint(0, 0), full.size()));
  if (mRelativeRect.isEmpty()) return;
  // 需要更新裁剪结果
  mIsNeedUpdateRectSelectedPixmap = true;

  // 显示选区框
  mFrame->setGeometry(visualSelRect);
  mFrame->show();
  mFrame->raise();

  if (mFrame->IsAutoFaceRecognize()) {
    onFaceRecognizeRequested();
  }
}

void SelectableLabel::paintEvent(QPaintEvent* e) {
  QLabel::paintEvent(e);
  if (!mIsDragging) return;                    // 固定框由 SelectionFrame 自绘
  QPainter p(this);
  p.setPen(QPen(QColor(255, 0, 0), 1, Qt::DashLine));
  p.setBrush(QColor(255, 0, 0, 40));
  p.drawRect(visSelectionRect());
}

QRect SelectableLabel::pixmapRectInLabel() const {
  const QPixmap pm = pixmap(Qt::ReturnByValueConstant::ReturnByValue);
  if (pm.isNull()) return {};

  const QRect cr = contentsRect();
  const QSize ps = pm.size();
  if (hasScaledContents()) return cr;

  const Qt::Alignment a = alignment();
  int x = cr.x();
  if (a & Qt::AlignRight)        x = cr.right()  - ps.width()  + 1;
  else if (!(a & Qt::AlignLeft)) x = cr.x() + (cr.width()  - ps.width())  / 2;

  int y = cr.y();
  if (a & Qt::AlignBottom)       y = cr.bottom() - ps.height() + 1;
  else if (!(a & Qt::AlignTop))  y = cr.y() + (cr.height() - ps.height()) / 2;

  return QRect(QPoint(x, y), ps);
}

bool SelectableLabel::SaveRectSelectionImage(const QPixmap& rectSelectedImg, const QString& sourceImageFullPath) {
  // 文件名：宽x高.jpg
  QString sourceImagePrepath;
  const QString sourceImageFileName = PathTool::GetPrepathAndFileName(sourceImageFullPath, sourceImagePrepath);
  QString sourceImageFileBaseName, sourceImageDotExt;
  std::tie(sourceImageFileBaseName, sourceImageDotExt) = PathTool::GetBaseNameExt(sourceImageFileName);
  const QSize srcSize{rectSelectedImg.size()};
  const QString dstImageFileName{QString{"%1 %2x%3%4"}
                                     .arg(sourceImageFileBaseName)
                                     .arg(srcSize.width()).arg(srcSize.height())
                                     .arg(sourceImageDotExt)};
  const QString recommendSaveToFullPath{PathTool::Path2Join(sourceImagePrepath, dstImageFileName)};
  const QString actualSavedToPath = QFileDialog::getSaveFileName(nullptr, "Save to", recommendSaveToFullPath, "*" + sourceImageDotExt);
  if (actualSavedToPath.isEmpty()) {
    LOG_INFO_NP("Skip", "User cancel save");
    return false;
  }
  const bool bSavedResult{rectSelectedImg.save(actualSavedToPath)};
  LOG_OE_NP(bSavedResult, "Save image", actualSavedToPath);
  return bSavedResult;
}