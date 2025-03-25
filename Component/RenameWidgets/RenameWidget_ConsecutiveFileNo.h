#ifndef RENAMEWIDGET_CONSECUTIVEFILENO_H
#define RENAMEWIDGET_CONSECUTIVEFILENO_H

#include "AdvanceRenamer.h"

class RenameWidget_ConsecutiveFileNo : public AdvanceRenamer {
 public:
  QLineEdit* m_fileNoStartIndex{new QLineEdit{"0", this}};

  RenameWidget_ConsecutiveFileNo(QWidget* parent = nullptr);

  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;

  void InitExtraMemberWidget() override {}
  QStringList RenameCore(const QStringList& replaceeList) override;
};

#endif // RENAMEWIDGET_CONSECUTIVEFILENO_H
