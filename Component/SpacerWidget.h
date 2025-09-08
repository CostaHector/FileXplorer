#ifndef SPACERWIDGET_H
#define SPACERWIDGET_H
#include <QWidget>
#include "Logger.h"

inline QWidget* GetSpacerWidget(QWidget* parent = nullptr, Qt::Orientation ori = Qt::Orientation::Horizontal) {
  QWidget* pSpacerWid = new (std::nothrow) QWidget{parent};
  if (pSpacerWid == nullptr) {
    LOG_C("pSpacerWid is nullptr");
    return nullptr;
  }
  if (ori == Qt::Orientation::Horizontal) {
    pSpacerWid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  } else if (ori == Qt::Orientation::Vertical) {
    pSpacerWid->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  }
  return pSpacerWid;
}

#endif  // SPACERWIDGET_H
