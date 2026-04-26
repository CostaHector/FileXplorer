#include "StateLabel.h"
#include "ImageTool.h"
#include "Logger.h"

QPixmap GetLabelStatusPixmap(StateLabel::LABEL_STATUS_E status) {
  if (status < StateLabel::BEGIN || status >= StateLabel::BUTT) {
    LOG_W("status[%d] out of bound", status);
    return {};
  }
  return ImageTool::GetLabelStatusPixmap(status);
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
