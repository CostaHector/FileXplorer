#ifndef RENAMEWIDGET_NUMERIZE_H
#define RENAMEWIDGET_NUMERIZE_H

#include "AdvanceRenamer.h"

class RenameWidget_Numerize : public AdvanceRenamer {
 public:
  explicit RenameWidget_Numerize(QWidget* parent = nullptr);

  void initExclusiveSetting() override;
  void InitExtraMemberWidget() override;
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  QStringList RenameCore(const QStringList& replaceeList) override;

 private:
  QLineEdit* m_startNo{nullptr};
  QCheckBox* m_isUniqueCounterPerExtension{nullptr};
  QComboBox* m_numberPattern{nullptr};
  QLineEdit* m_completeBaseName{nullptr};
  bool m_baseNameInited = false;
};
#endif // RENAMEWIDGET_NUMERIZE_H
