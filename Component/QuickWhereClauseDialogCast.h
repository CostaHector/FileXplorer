#ifndef QUICKWHERECLAUSEDIALOGCAST_H
#define QUICKWHERECLAUSEDIALOGCAST_H
#include "QuickWhereClauseDialog.h"

class QuickWhereClauseDialogCast : public QuickWhereClauseDialog {
public:
  using QuickWhereClauseDialog::QuickWhereClauseDialog;

private:
  void CreatePrivateWidget() override;
  void InitPrivateLayout() override;
  void PrivateSubscribe() override;
  void onConditionsChanged() override;

  QLineEdit* m_Name{nullptr};
  QLineEdit* m_Rate{nullptr};
  QLineEdit* m_Ori{nullptr};
  QLineEdit* m_Tags{nullptr};
  QLineEdit* m_Height{nullptr};
  QLineEdit* m_Size{nullptr};
};

#endif // QUICKWHERECLAUSEDIALOGCAST_H
