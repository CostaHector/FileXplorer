#ifndef RENAMEWIDGET_PREPENDPARENTFOLDERNAME_H
#define RENAMEWIDGET_PREPENDPARENTFOLDERNAME_H

#include "AdvanceRenamer.h"
class RenameWidget_PrependParentFolderName : public AdvanceRenamer {
public:
  explicit RenameWidget_PrependParentFolderName(QWidget* parent = nullptr);
  void initExclusiveSetting() override;
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  QStringList RenameCore(const QStringList& replaceeList) override;
};

#endif // RENAMEWIDGET_PREPENDPARENTFOLDERNAME_H
