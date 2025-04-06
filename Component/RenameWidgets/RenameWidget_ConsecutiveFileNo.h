#ifndef RENAMEWIDGET_CONSECUTIVEFILENO_H
#define RENAMEWIDGET_CONSECUTIVEFILENO_H

#include "AdvanceRenamer.h"

class RenameWidget_ConsecutiveFileNo : public AdvanceRenamer {
 public:
  RenameWidget_ConsecutiveFileNo(QWidget* parent = nullptr);

  QLineEdit* m_fileNoStartIndex{nullptr};

  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;

  void InitExtraMemberWidget() override {}
  QStringList RenameCore(const QStringList& replaceeList) override;
};

#endif // RENAMEWIDGET_CONSECUTIVEFILENO_H
