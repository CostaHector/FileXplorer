#ifndef RENAMEWIDGET_PREPENDPARENTFOLDERNAME_H
#define RENAMEWIDGET_PREPENDPARENTFOLDERNAME_H

#include "AdvanceRenamer.h"
class RenameWidget_PrependParentFolderName : public AdvanceRenamer
{
public:
  RenameWidget_PrependParentFolderName(QWidget* parent = nullptr);
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe()  override;
  void InitExtraMemberWidget() override;
  QStringList RenameCore(const QStringList& replaceeList) override;
};

#endif // RENAMEWIDGET_PREPENDPARENTFOLDERNAME_H
