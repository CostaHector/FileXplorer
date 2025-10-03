#ifndef MOUSEKEYBOARDEVENTHELPER_H
#define MOUSEKEYBOARDEVENTHELPER_H
#include <QWidget>
class ThumbnailImageViewer;
class ClickableTextBrowser;
class CustomListView;
class CustomTableView;

namespace MouseKeyboardEventHelper {
template<typename WidgetType>
bool SendWheelEvent(WidgetType& wid, const QPoint& angleDelta,
                    Qt::KeyboardModifiers modifiers = Qt::NoModifier,
                    bool bNeedAccepted = true);


extern template bool SendWheelEvent<ThumbnailImageViewer>(
    ThumbnailImageViewer& wid, const QPoint& angleDelta,
    Qt::KeyboardModifiers modifiers, bool bNeedAccepted);

extern template bool SendWheelEvent<ClickableTextBrowser>(
    ClickableTextBrowser& wid, const QPoint& angleDelta,
    Qt::KeyboardModifiers modifiers, bool bNeedAccepted);

extern template bool SendWheelEvent<CustomListView>(
    CustomListView& wid, const QPoint& angleDelta,
    Qt::KeyboardModifiers modifiers, bool bNeedAccepted);


template <typename ViewType>
bool SendMousePressEvent(ViewType& view,
                         Qt::MouseButton button = Qt::BackButton,
                         Qt::KeyboardModifiers modifiers = Qt::NoModifier);

// 显式实例化声明
extern template bool SendMousePressEvent<CustomListView>(
    CustomListView& view, Qt::MouseButton button,
    Qt::KeyboardModifiers modifiers);

extern template bool SendMousePressEvent<CustomTableView>(
    CustomTableView& view, Qt::MouseButton button,
    Qt::KeyboardModifiers modifiers);


}

#endif  // MOUSEKEYBOARDEVENTHELPER_H
