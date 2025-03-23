#include "LineEditStr.h"

LineEditStr::LineEditStr(const QString& formName, const QString& text, QWidget* parent) : QLineEdit{text, parent}, mFormName{formName} {}

QString LineEditStr::GetFormName() const {
  return mFormName;
}
