#ifndef RENAMEWIDGET_SWAPFILENAMES_H
#define RENAMEWIDGET_SWAPFILENAMES_H

#include "AdvanceRenamer.h"

class RenameWidget_SwapFileNames : public AdvanceRenamer {
  Q_OBJECT
 public:
  explicit RenameWidget_SwapFileNames(QWidget* parent = nullptr);
  void initExclusiveSetting() override;
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  QStringList RenameCore(const QStringList& replaceeList) override;
};

#endif  // RENAMEWIDGET_SWAPFILENAMES_H
