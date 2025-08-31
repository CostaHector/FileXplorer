#include "RenameWidget_ConvertBoldUnicodeCharset2Ascii.h"
#include "ConvertUnicodeCharsetToAscii.h"
#include "PublicMacro.h"

RenameWidget_ConvertBoldUnicodeCharset2Ascii::RenameWidget_ConvertBoldUnicodeCharset2Ascii(QWidget* parent)
  : AdvanceRenamer(parent) {}

QToolBar* RenameWidget_ConvertBoldUnicodeCharset2Ascii::InitControlTB() {
  QToolBar* unicode2AsciiTb = new QToolBar{"Unicode 2 ASCII", this};
  CHECK_NULLPTR_RETURN_NULLPTR(unicode2AsciiTb);
  unicode2AsciiTb->addWidget(m_nameExtIndependent);
  unicode2AsciiTb->addWidget(m_recursiveCB);
  return unicode2AsciiTb;
}

QStringList RenameWidget_ConvertBoldUnicodeCharset2Ascii::RenameCore(const QStringList& replaceeList) {
  ConvertUnicodeCharsetToAscii cbuctt;
  return cbuctt.BatchCovert2Text(replaceeList);
}

void RenameWidget_ConvertBoldUnicodeCharset2Ascii::InitExtraCommonVariable() {
  windowTitleFormat = "Convert bold unicode charset to ascii | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/UNICODE_TO_ASCII_TEXT"));
}
