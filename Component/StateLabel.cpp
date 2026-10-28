#include "StateLabel.h"
#include "Logger.h"

QPixmap GetLabelStatusPixmap(StateLabel::LABEL_STATUS_E status) {
  if (status < StateLabel::BEGIN || status >= StateLabel::BUTT) {
    LOG_W("status[%d] out of bound", status);
    return {};
  }
  static const QPixmap labelSavedStatusPxp[]                      //
      {QPixmap(":img/NOT_SAVED").scaled(24, 24, Qt::KeepAspectRatio), //
       QPixmap(":img/SAVED").scaled(24, 24, Qt::KeepAspectRatio)};
  const int clampResult = std::max(0, std::min(1, (int)status));
  return labelSavedStatusPxp[clampResult];
}

StateLabel::StateLabel(const QString& text, QWidget* parent)  //
  : QLabel{text, parent} {
#ifndef RUNNING_UNIT_TESTS
  setPixmap(GetLabelStatusPixmap(m_currentState));
#endif
  setAlignment(Qt::AlignRight);
}

void StateLabel::ToSaved() {
  if (m_currentState == SAVED) {
    return;
  }
  m_currentState = SAVED;
#ifndef RUNNING_UNIT_TESTS
  setPixmap(GetLabelStatusPixmap(m_currentState));
#endif
}
void StateLabel::ToNotSaved() {
  if (m_currentState == NOT_SAVED) {
    return;
  }
  m_currentState = NOT_SAVED;
#ifndef RUNNING_UNIT_TESTS
  setPixmap(GetLabelStatusPixmap(m_currentState));
#endif
}
