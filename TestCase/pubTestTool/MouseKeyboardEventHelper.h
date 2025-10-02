#ifndef MOUSEKEYBOARDEVENTHELPER_H
#define MOUSEKEYBOARDEVENTHELPER_H
#include <QWidget>
#include "ThumbnailImageViewer.h"
#include "ClickableTextBrowser.h"
namespace MouseKeyboardEventHelper {
bool SendWheelEvent(ThumbnailImageViewer& wid, const QPoint& angelDelta, Qt::KeyboardModifiers modifiers, bool bNeedAccepted);
bool SendWheelEvent(ClickableTextBrowser& wid, const QPoint& angelDelta, Qt::KeyboardModifiers modifiers, bool bNeedAccepted);
}

#endif  // MOUSEKEYBOARDEVENTHELPER_H
