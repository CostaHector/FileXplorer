#ifndef RENAMEWIDGET_REPLACE_H
#define RENAMEWIDGET_REPLACE_H
#include "AdvanceRenamer.h"

class RenameWidget_Replace : public AdvanceRenamer {
 public:
  QComboBox* oldStrCB;
  QComboBox* newStrCB;
  QCheckBox* regex;
  RenameWidget_Replace(QWidget* parent = nullptr);

  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  void InitExtraMemberWidget() override;

  QStringList RenameCore(const QStringList& replaceeList) override;
};

class RenameWidget_Delete : public RenameWidget_Replace {
 public:
  RenameWidget_Delete(QWidget* parent = nullptr);
  void InitExtraCommonVariable() override;
};

#endif // RENAMEWIDGET_REPLACE_H
