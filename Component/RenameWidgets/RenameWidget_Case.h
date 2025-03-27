#ifndef RENAMEWIDGET_CASE_H
#define RENAMEWIDGET_CASE_H
#include "AdvanceRenamer.h"

class RenameWidget_Case : public AdvanceRenamer {
 public:
  explicit RenameWidget_Case(QWidget* parent = nullptr);
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  void InitExtraMemberWidget() override {}

  QStringList RenameCore(const QStringList& replaceeList) override;
};

#endif  // RENAMEWIDGET_CASE_H
