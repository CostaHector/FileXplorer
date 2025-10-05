#include "DraggableToolButton.h"
#include "PublicMacro.h"
#include "ViewHelper.h"
#include <QAction>
#include <QMimeData>
#include <QDrag>
#include <QPainter>

DraggableToolButton::DraggableToolButton(QWidget* parent)  //
    : QToolButton{parent}  //
{
  setAcceptDrops(false);
}

void DraggableToolButton::mousePressEvent(QMouseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  if (event->button() & Qt::LeftButton) {
    mDragStartPosition = event->pos();
  }
  QToolButton::mousePressEvent(event);
}

void DraggableToolButton::mouseMoveEvent(QMouseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  if (!(event->buttons().testFlag(Qt::LeftButton)))
    return;
  if ((event->pos() - mDragStartPosition).manhattanLength() < View::START_DRAG_DIST_MIN) {
    return;
  }
  setEnabled(false);
  QMimeData* pMimeData = new (std::nothrow) QMimeData;
  CHECK_NULLPTR_RETURN_VOID(pMimeData)
  pMimeData->setText(text());
  QDrag* pDrag = new (std::nothrow) QDrag{this};
  CHECK_NULLPTR_RETURN_VOID(pDrag)
  pDrag->setMimeData(pMimeData);

  QPixmap pixmap = grab();
  QPixmap transparent_pixmap(pixmap.size());
  transparent_pixmap.fill(Qt::transparent);
  QPainter painter(&transparent_pixmap);
  painter.setOpacity(0.75);
  painter.drawPixmap(0, 0, pixmap);
  painter.end();

  pDrag->setPixmap(transparent_pixmap);
  pDrag->setHotSpot(event->pos());
#ifdef RUNNING_UNIT_TESTS
  setEnabled(true);
#else
  if (pDrag->exec(Qt::MoveAction) == Qt::IgnoreAction) {
    setEnabled(true);
  }
#endif
  event->accept();
}
