#ifndef RENAMEWIDGET_ARRANGESECTION_H
#define RENAMEWIDGET_ARRANGESECTION_H

#include "AdvanceRenamer.h"

class RenameWidget_ArrangeSection : public AdvanceRenamer {
 public:
  QComboBox* m_indexesCB{new QComboBox(this)};
  QCheckBox* m_strictMode{new QCheckBox("Strict mode", this)};
  explicit RenameWidget_ArrangeSection(QWidget* parent = nullptr);
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  void InitExtraMemberWidget() override;
  QStringList RenameCore(const QStringList& replaceeList) override;
};


#endif // RENAMEWIDGET_ARRANGESECTION_H
