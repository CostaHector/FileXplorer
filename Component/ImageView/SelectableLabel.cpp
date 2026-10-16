#include "SelectableLabel.h"
#include <QPixmap>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QFileDialog>
#include "NotificatorMacro.h"
#include "PathTool.h"

void SelectableLabel::setSelectionEnabled(bool on) {
  mSelectionEnabled = on;
  setCursor(on ? Qt::CrossCursor : Qt::ArrowCursor);
  if (!on) {
    mSelecting = false;
    update();
  }
}

void SelectableLabel::mousePressEvent(QMouseEvent *e) {
  if (!mSelectionEnabled || e->button() != Qt::LeftButton) {
    QLabel::mousePressEvent(e);
    return;
  }
  mSelecting = true;
  mOrigin = mCurrent = e->pos();
  update();
}

void SelectableLabel::mouseMoveEvent(QMouseEvent *e) {
  if (!mSelecting) {
    QLabel::mouseMoveEvent(e);
    return;
  }
  mCurrent = e->pos();
  update();
}

void SelectableLabel::mouseReleaseEvent(QMouseEvent *e) {
  if (!mSelecting || e->button() != Qt::LeftButton) {
    QLabel::mouseReleaseEvent(e);
    return;
  }
  mSelecting = false;
  mCurrent = e->pos();
  update();

  const QRect sel = selectionRect();
  if (sel.width() < 2 || sel.height() < 2) return;

  const QPixmap full = pixmap(Qt::ReturnByValueConstant::ReturnByValue);
  if (full.isNull()) return;

  // 关键：pixmap 在 label 坐标系中的矩形
  const QRect pixRect = pixmapRectInLabel();
  if (pixRect.isEmpty()) return;

  // label 坐标 → pixmap 坐标：直接平移即可
  QRect src = sel.translated(-pixRect.topLeft());

  // 裁剪到 pixmap 有效范围内
  src = src.intersected(QRect(QPoint(0, 0), full.size()));
  if (src.isEmpty()) return;

  emit selectionFinished(full.copy(src), src.size());
}

void SelectableLabel::paintEvent(QPaintEvent *e) {
  QLabel::paintEvent(e);
  if (!mSelecting) return;

  QPainter p(this);
  p.setPen(QPen(QColor(255, 0, 0), 1, Qt::DashLine));
  p.setBrush(QColor(255, 0, 0, 40));
  p.drawRect(selectionRect());
}

QRect SelectableLabel::pixmapRectInLabel() const {
  const QPixmap pm = pixmap(Qt::ReturnByValueConstant::ReturnByValue);
  if (pm.isNull()) return {};

  const QRect cr = contentsRect();
  const QSize ps = pm.size();

  // 如果用了 setScaledContents(true)，pixmap 会拉伸填满 contentsRect
  if (hasScaledContents()) return cr;

  const Qt::Alignment a = alignment();

  // 水平
  int x = cr.x();
  if (a & Qt::AlignRight)       x = cr.right()  - ps.width()  + 1;
  else if (!(a & Qt::AlignLeft)) x = cr.x() + (cr.width()  - ps.width())  / 2;

  // 垂直
  int y = cr.y();
  if (a & Qt::AlignBottom)       y = cr.bottom() - ps.height() + 1;
  else if (!(a & Qt::AlignTop))  y = cr.y() + (cr.height() - ps.height()) / 2;

  return QRect(QPoint(x, y), ps);
}

bool SelectableLabel::SaveSelectionRectImage(const QPixmap& selected, const QSize& srcSize, const QString& sourceImageFullPath) {
  // 文件名：宽x高.jpg
  QString sourceImagePrepath;
  const QString sourceImageFileName = PathTool::GetPrepathAndFileName(sourceImageFullPath, sourceImagePrepath);
  QString sourceImageFileBaseName, sourceImageDotExt;
  std::tie(sourceImageFileBaseName, sourceImageDotExt) = PathTool::GetBaseNameExt(sourceImageFileName);
  const QString dstImageFileName{QString{"%1 %2x%3%4"}
                                     .arg(sourceImageFileBaseName)
                                     .arg(srcSize.width()).arg(srcSize.height())
                                     .arg(sourceImageDotExt)};
  const QString recommendSaveToFullPath{PathTool::Path2Join(sourceImagePrepath, dstImageFileName)};
  const QString actualSavedToPath = QFileDialog::getSaveFileName(nullptr, "Save to", recommendSaveToFullPath, tr("JPEG (*.jpg)"));
  if (actualSavedToPath.isEmpty()) {
    LOG_INFO_NP("Skip", "User cancel save");
    return false;
  }
  const bool bSavedResult{selected.save(actualSavedToPath)};
  LOG_OE_NP(bSavedResult, "Save image", actualSavedToPath);
  return bSavedResult;
}