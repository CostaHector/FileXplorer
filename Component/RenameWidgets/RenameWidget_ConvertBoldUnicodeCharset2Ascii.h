#ifndef RENAMEWIDGET_CONVERTBOLDUNICODECHARSET2ASCII_H
#define RENAMEWIDGET_CONVERTBOLDUNICODECHARSET2ASCII_H

#include "AdvanceRenamer.h"

class RenameWidget_ConvertBoldUnicodeCharset2Ascii : public AdvanceRenamer {
 public:
  explicit RenameWidget_ConvertBoldUnicodeCharset2Ascii(QWidget* parent = nullptr);
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;

  QStringList RenameCore(const QStringList& replaceeList) override;
};

#endif  // RENAMEWIDGET_CONVERTBOLDUNICODECHARSET2ASCII_H
