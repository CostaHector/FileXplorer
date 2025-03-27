#ifndef RENAMEWIDGET_REVERSENAMES_H
#define RENAMEWIDGET_REVERSENAMES_H

#include "AdvanceRenamer.h"

class RenameWidget_ReverseNames : public AdvanceRenamer {
 public:
  explicit RenameWidget_ReverseNames(QWidget* parent = nullptr);
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe()  override;
  void InitExtraMemberWidget() override;
  QStringList RenameCore(const QStringList& replaceeList) override;
};

#endif // RENAMEWIDGET_REVERSENAMES_H
