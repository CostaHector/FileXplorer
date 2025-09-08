#include "LineEditInt.h"
#include "Logger.h"

LineEditInt::LineEditInt(const QString& formName, const QString& text, QWidget* parent) : QLineEdit{text, parent}, mFormName{formName} {}

QString LineEditInt::GetFormName() const {
  return mFormName;
}

bool LineEditInt::CheckValueValid() const {
  const QString& numberStr = text();
  bool isNumber = false;
  numberStr.toInt(&isNumber);
  if (!isNumber) {
    LOG_W("NumberStr[%s] is not a number", qPrintable(numberStr));
    return false;
  }
  return true;
}

void LineEditInt::ReadFromInt(int iVal)
{
  setText(QString::number(iVal));
}

int LineEditInt::GetIntValue() const {
  const QString& numberStr = text();
  bool isNumber = false;
  int ansInt = numberStr.toInt(&isNumber);
  if (!isNumber) {
    LOG_W("NumberStr[%s] is not a number", qPrintable(numberStr));
    return -1;
  }
  return ansInt;
}
