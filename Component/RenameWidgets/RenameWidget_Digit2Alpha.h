#ifndef RENAMEWIDGET_DIGIT2ALPHA_H
#define RENAMEWIDGET_DIGIT2ALPHA_H

#include "AdvanceRenamer.h"
class RenameWidget_Digit2Alpha : public AdvanceRenamer {
  Q_OBJECT
public:
  using AdvanceRenamer::AdvanceRenamer;
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  QStringList RenameCore(const QStringList& replaceeList) override;
};

#endif // RENAMEWIDGET_DIGIT2ALPHA_H
