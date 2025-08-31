#include "StateLabel.h"
#include <QPixmap>

QPixmap GetLabelStatusPixmap(StateLabel::LABEL_STATUS_E status) {
  if (status < StateLabel::BEGIN || status >= StateLabel::BUTT) {
    qWarning("status[%d] out of bound", status);
    return {};
  }
  static const QPixmap labelSavedStatusPxp[StateLabel::BUTT]       //
      {QPixmap(":img/SAVED").scaled(24, 24, Qt::KeepAspectRatio),  //
       QPixmap(":img/NOT_SAVED").scaled(24, 24, Qt::KeepAspectRatio)};
  return labelSavedStatusPxp[status];
}

StateLabel::StateLabel(const QString& text, QWidget* parent)  //
  : QLabel{text, parent} {
#ifndef RUNNING_UNIT_TESTS
  setPixmap(GetLabelStatusPixmap(m_currentState));
#endif
  setAlignment(Qt::AlignRight);
}

void StateLabel::ToSaved() {
  m_currentState = SAVED;
#ifndef RUNNING_UNIT_TESTS
  setPixmap(GetLabelStatusPixmap(m_currentState));
#endif
}
void StateLabel::ToNotSaved() {
  m_currentState = NOT_SAVED;
#ifndef RUNNING_UNIT_TESTS
  setPixmap(GetLabelStatusPixmap(m_currentState));
#endif
}
