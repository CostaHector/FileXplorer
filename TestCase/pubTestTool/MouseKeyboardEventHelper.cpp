#include "MouseKeyboardEventHelper.h"
#include "ThumbnailImageViewer.h"
#include "ClickableTextBrowser.h"
#include "CustomListView.h"
#include "CustomTableView.h"
#include <QWheelEvent>

namespace MouseKeyboardEventHelper {
template <typename WidgetType>
bool SendWheelEvent(WidgetType& wid, const QPoint& angleDelta, Qt::KeyboardModifiers modifiers, bool bNeedAccepted) {
  QPoint widCenterPnt = wid.geometry().center();
  QPoint widCenterPntGlb = wid.mapToGlobal(widCenterPnt);

  // 注意：修正了参数顺序，QWheelEvent构造函数的正确顺序
  QWheelEvent event(widCenterPnt, widCenterPntGlb, QPoint(), angleDelta, Qt::NoButton, modifiers, Qt::NoScrollPhase, false);
  wid.wheelEvent(&event);
  return event.isAccepted() == bNeedAccepted;
}

// 显式实例化定义
template bool SendWheelEvent<ThumbnailImageViewer>(ThumbnailImageViewer& wid,
                                                   const QPoint& angleDelta,
                                                   Qt::KeyboardModifiers modifiers,
                                                   bool bNeedAccepted);

template bool SendWheelEvent<ClickableTextBrowser>(ClickableTextBrowser& wid,
                                                   const QPoint& angleDelta,
                                                   Qt::KeyboardModifiers modifiers,
                                                   bool bNeedAccepted);

template bool SendWheelEvent<CustomListView>(CustomListView& wid,              //
                                             const QPoint& angleDelta,         //
                                             Qt::KeyboardModifiers modifiers,  //
                                             bool bNeedAccepted);              //


template <typename ViewType>
bool SendMousePressEvent(ViewType& view,
                         Qt::MouseButton button,
                         Qt::KeyboardModifiers modifiers)
{
  QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0),
                    button, button, modifiers);
  view.mousePressEvent(&event);
  return event.isAccepted();
}

// 显式实例化定义
template bool SendMousePressEvent<CustomListView>(
    CustomListView& view, Qt::MouseButton button,
    Qt::KeyboardModifiers modifiers);

template bool SendMousePressEvent<CustomTableView>(
    CustomTableView& view, Qt::MouseButton button,
    Qt::KeyboardModifiers modifiers);


}  // namespace MouseKeyboardEventHelper
