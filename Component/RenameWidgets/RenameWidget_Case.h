#ifndef RENAMEWIDGET_CASE_H
#define RENAMEWIDGET_CASE_H
#include "AdvanceRenamer.h"

class RenameWidget_Case : public AdvanceRenamer {
  Q_OBJECT
 public:
  using AdvanceRenamer::AdvanceRenamer;
  void InitExtraCommonVariable() override;
  void InitExtraMemberWidget() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  bool isCaseChangedOnly() const override { return true; }
  QStringList RenameCore(const QStringList& replaceeList) override;

private:
  void onNameCaseActionTriggered(QAction* pCaseAct);
  void onLowerCaseFileExtension(bool bLowerCaseFileExtension);

  QAction* m_upperCase {nullptr};
  QAction* m_lowerCase {nullptr};
  QAction* m_capitalizeKeepOther {nullptr};
  QAction* m_capitalizeLowerOther {nullptr};
  QAction* m_toggleCase {nullptr};
  QActionGroup* m_nameCaseAg {nullptr};
  QList<QAction*> m_nameCaseActs;

  QAction* m_lowerCaseFileExtension {nullptr};
};

#endif  // RENAMEWIDGET_CASE_H
