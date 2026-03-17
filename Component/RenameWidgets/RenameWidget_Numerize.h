#ifndef RENAMEWIDGET_NUMERIZE_H
#define RENAMEWIDGET_NUMERIZE_H

#include "AdvanceRenamer.h"
#include <QLineEdit>
#include <QComboBox>

class RenameWidget_Numerize : public AdvanceRenamer {
  Q_OBJECT
 public:
  using AdvanceRenamer::AdvanceRenamer;

  void initExclusiveSetting() override;
  void InitExtraMemberWidget() override;
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  QStringList RenameCore(const QStringList& replaceeList) override;
  bool reorderNamesInListView();

 private:
  QLineEdit* m_startNo{nullptr};
  QCheckBox* m_isUniqueCounterPerExtension{nullptr};
  QComboBox* m_numberPattern{nullptr};
  QLineEdit* m_completeBaseName{nullptr};
  QAction* m_dragToReorderNames{nullptr};
  bool m_baseNameInited = false;
};
#endif // RENAMEWIDGET_NUMERIZE_H
