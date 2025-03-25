#ifndef RENAMEWIDGET_CONVERTBOLDUNICODECHARSET2ASCII_H
#define RENAMEWIDGET_CONVERTBOLDUNICODECHARSET2ASCII_H

#include "AdvanceRenamer.h"

class RenameWidget_ConvertBoldUnicodeCharset2Ascii : public AdvanceRenamer {
 public:
  RenameWidget_ConvertBoldUnicodeCharset2Ascii(QWidget* parent = nullptr) : AdvanceRenamer(parent) {}

  void InitExtraCommonVariable() override;

  QToolBar* InitControlTB() override;
  void extraSubscribe() override {}

  void InitExtraMemberWidget() override {}

  QStringList RenameCore(const QStringList& replaceeList) override;
};

#endif  // RENAMEWIDGET_CONVERTBOLDUNICODECHARSET2ASCII_H
