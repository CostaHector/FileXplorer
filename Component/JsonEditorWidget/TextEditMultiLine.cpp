#include "TextEditMultiLine.h"

TextEditMultiLine::TextEditMultiLine(const QString& formName, const QString& text, QWidget* parent) : QTextEdit{text, parent}, mFormName{formName} {}
QString TextEditMultiLine::GetFormName() const {
  return mFormName;
}
