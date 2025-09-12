#include "PathComboBox.h"

void PathComboBox::focusInEvent(QFocusEvent* event) {
  QComboBox::focusInEvent(event);
  emit focusChanged(true);
}

void PathComboBox::focusOutEvent(QFocusEvent* event) {
  QComboBox::focusOutEvent(event);
  emit focusChanged(false);
}
