#include "ReturnEventConsumingLineEdit.h"
#include <QKeyEvent>

void ReturnEventConsumingLineEdit::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key::Key_Enter || event->key() == Qt::Key::Key_Return) {
    event->accept();
    emit returnPressed();
    return;
  }
  QLineEdit::keyPressEvent(event);
}
