#include "MouseKeyboardEventHelper.h"
#include <QWheelEvent>
namespace MouseKeyboardEventHelper {
bool SendWheelEvent(ThumbnailImageViewer& wid, const QPoint& angelDelta, Qt::KeyboardModifiers modifiers, bool bNeedAccepted) {
  QPoint widCenterPnt = wid.geometry().center();
  QPoint widCenterPntGlb = wid.mapToGlobal(widCenterPnt);
  QWheelEvent event(widCenterPnt, widCenterPntGlb, angelDelta, angelDelta, Qt::NoButton, modifiers, Qt::NoScrollPhase, false);
  wid.wheelEvent(&event);
  return event.isAccepted() == bNeedAccepted;
}
bool SendWheelEvent(ClickableTextBrowser& wid, const QPoint& angelDelta, Qt::KeyboardModifiers modifiers, bool bNeedAccepted) {
  QPoint widCenterPnt = wid.geometry().center();
  QPoint widCenterPntGlb = wid.mapToGlobal(widCenterPnt);
  QWheelEvent event(widCenterPnt, widCenterPntGlb, angelDelta, angelDelta, Qt::NoButton, modifiers, Qt::NoScrollPhase, false);
  wid.wheelEvent(&event);
  return event.isAccepted() == bNeedAccepted;
}
}
