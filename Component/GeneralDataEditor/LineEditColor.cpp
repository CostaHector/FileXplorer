#include "LineEditColor.h"
#include "NotificatorMacro.h"
#include <QColorDialog>

void LineEditColor::onActionTriggered() {
  QColor newColor = QColorDialog::getColor(Qt::GlobalColor::white, this, "Select color");
  if (!newColor.isValid()) {
    LOG_INFO_NP("Skip", "User cancel select a color");
    return;
  }
  setText(newColor.name(QColor::HexArgb));
}
