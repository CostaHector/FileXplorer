#ifndef QUICKWHERECLAUSEDIALOGCAST_H
#define QUICKWHERECLAUSEDIALOGCAST_H
#include "QuickWhereClauseDialog.h"

class QuickWhereClauseDialogCast : public QuickWhereClauseDialog {
public:
  using QuickWhereClauseDialog::QuickWhereClauseDialog;

private:
  void CreatePrivateWidget() override;

  ColumnFilterLineEdit* m_Name{nullptr};
  ColumnFilterLineEdit* m_Rate{nullptr};
  ColumnFilterLineEdit* m_Ori{nullptr};
  ColumnFilterLineEdit* m_Tags{nullptr};
  ColumnFilterLineEdit* m_Height{nullptr};
  ColumnFilterLineEdit* m_Size{nullptr};
};

#endif // QUICKWHERECLAUSEDIALOGCAST_H
