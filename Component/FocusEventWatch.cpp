#include "FocusEventWatch.h"
#include "public/PublicMacro.h"

FocusEventWatch::FocusEventWatch(QObject* parent)
  : QObject{parent} {
  CHECK_NULLPTR_RETURN_VOID(parent);
  parent->installEventFilter(this);
}

bool FocusEventWatch::eventFilter(QObject* watched, QEvent* event) {
  switch (event->type()) {
    case QEvent::Type::MouseButtonPress: {
      mouseButtonPressedBefore = true;
      break;
    }
    case QEvent::Type::MouseButtonRelease: {
      break;
    }
    case QEvent::Type::FocusIn: {
      if (!mouseButtonPressedBefore) { // block until next time focus out
        emit focusChanged(true);
      }
      mouseButtonPressedBefore = false;
      break;
    }
    case QEvent::Type::FocusOut: {
      if (!mouseButtonPressedBefore) { // block until next time focus out
        emit focusChanged(false);
      }
      mouseButtonPressedBefore = false;
      break;
    }
    default:
      break;
  }
  return QObject::eventFilter(watched, event);
}
