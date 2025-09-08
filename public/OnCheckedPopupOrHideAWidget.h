#ifndef ONCHECKEDPOPUPORHIDEAWIDGET_H
#define ONCHECKEDPOPUPORHIDEAWIDGET_H

#include <QWidget>
#include "Logger.h"

template <typename WIDGET_TYPE>
WIDGET_TYPE* PopupHideWidget(WIDGET_TYPE* p, const bool checked, QWidget* parent = nullptr) {
  if (!checked) {
    if (p != nullptr) {
      p->hide();
    }
    return p;
  }
  if (p == nullptr) {
    p = new (std::nothrow) WIDGET_TYPE{parent};
  }
  if (p == nullptr) {
    LOG_C("WIDGET_TYPE* p is nullptr");
    return nullptr;
  }

  p->show();
  p->activateWindow();
  p->raise();
  return p;
}

#endif  // ONCHECKEDPOPUPORHIDEAWIDGET_H
