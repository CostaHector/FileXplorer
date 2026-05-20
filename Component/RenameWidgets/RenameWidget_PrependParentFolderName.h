#ifndef RENAMEWIDGET_PREPENDPARENTFOLDERNAME_H
#define RENAMEWIDGET_PREPENDPARENTFOLDERNAME_H

#include "AdvanceRenamer.h"
class RenameWidget_PrependParentFolderName : public AdvanceRenamer {
  Q_OBJECT
 public:
  using AdvanceRenamer::AdvanceRenamer;
  void initExclusiveSetting() override;
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  QStringList RenameCore(const QStringList& replaceeList) override;
};

#endif  // RENAMEWIDGET_PREPENDPARENTFOLDERNAME_H
