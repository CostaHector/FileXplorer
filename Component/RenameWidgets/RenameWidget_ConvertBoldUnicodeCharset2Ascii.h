#ifndef RENAMEWIDGET_CONVERTBOLDUNICODECHARSET2ASCII_H
#define RENAMEWIDGET_CONVERTBOLDUNICODECHARSET2ASCII_H

#include "AdvanceRenamer.h"

class RenameWidget_ConvertBoldUnicodeCharset2Ascii : public AdvanceRenamer {
 public:
  RenameWidget_ConvertBoldUnicodeCharset2Ascii(QWidget* parent = nullptr) : AdvanceRenamer(parent) {}

  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = "Convert bold unicode charset to ascii | %1 item(s) under [%2]";
    setWindowTitle(windowTitleFormat);
    setWindowIcon(QIcon(":img/UNICODE_TO_ASCII_TEXT"));
  }

  auto InitControlTB() -> QToolBar* override;
  auto extraSubscribe() -> void override {}

  auto InitExtraMemberWidget() -> void override {}

  QStringList RenameCore(const QStringList& replaceeList) override;
};

#endif  // RENAMEWIDGET_CONVERTBOLDUNICODECHARSET2ASCII_H
