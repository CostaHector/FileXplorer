#include "PathComboBox.h"

void PathComboBox::focusInEvent(QFocusEvent* event) {
  QComboBox::focusInEvent(event);
  emit focusChanged(true);
}

void PathComboBox::focusOutEvent(QFocusEvent* event) {
  QComboBox::focusOutEvent(event);
  if (event->reason() == Qt::FocusReason::PopupFocusReason) {
    return;
  }
  emit focusChanged(false);
}
