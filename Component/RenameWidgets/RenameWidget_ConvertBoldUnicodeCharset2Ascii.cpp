#include "RenameWidget_ConvertBoldUnicodeCharset2Ascii.h"
#include "Tools/ConvertUnicodeCharsetToAscii.h"

QToolBar* RenameWidget_ConvertBoldUnicodeCharset2Ascii::InitControlTB() {
  QToolBar* replaceControl = new QToolBar;
  replaceControl->addWidget(m_extensionInNameCB);
  replaceControl->addWidget(m_recursiveCB);
  return replaceControl;
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
