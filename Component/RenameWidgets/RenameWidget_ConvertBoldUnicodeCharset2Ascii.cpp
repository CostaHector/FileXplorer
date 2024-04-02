#include "RenameWidget_ConvertBoldUnicodeCharset2Ascii.h"
#include "Tools/ConvertBoldUnicodeCharsetToAscii.h"

QToolBar* RenameWidget_ConvertBoldUnicodeCharset2Ascii::InitControlTB() {
  QToolBar* replaceControl = new QToolBar;
  replaceControl->addWidget(EXT_INSIDE_FILENAME);
  replaceControl->addWidget(ITEMS_INSIDE_SUBDIR);
  return replaceControl;
}

QStringList RenameWidget_ConvertBoldUnicodeCharset2Ascii::RenameCore(const QStringList& replaceeList) {
  ConvertBoldUnicodeCharsetToAscii cbuctt;
  return cbuctt.BatchCovert2Text(replaceeList);
}
