#ifndef RENAMEWIDGET_CONSECUTIVEFILENO_H
#define RENAMEWIDGET_CONSECUTIVEFILENO_H

#include "AdvanceRenamer.h"

class RenameWidget_ConsecutiveFileNo : public AdvanceRenamer {
 public:
  explicit RenameWidget_ConsecutiveFileNo(QWidget* parent = nullptr);
  void initExclusiveSetting() override;
  void InitExtraCommonVariable() override;
  void InitExtraMemberWidget() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;

  QStringList RenameCore(const QStringList& replaceeList) override;
private:
  QLineEdit* m_fileNoStartIndex{nullptr};
};

#endif // RENAMEWIDGET_CONSECUTIVEFILENO_H
