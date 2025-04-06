#ifndef RENAMEWIDGET_INSERT_H
#define RENAMEWIDGET_INSERT_H
#include "AdvanceRenamer.h"

class RenameWidget_Insert : public AdvanceRenamer {
 public:
  explicit RenameWidget_Insert(QWidget* parent = nullptr);
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  void InitExtraMemberWidget() override;
  QStringList RenameCore(const QStringList& replaceeList) override;
  QComboBox* insertStrCB{nullptr};
  QComboBox* insertAtCB{nullptr};
};

#endif  // RENAMEWIDGET_INSERT_H
