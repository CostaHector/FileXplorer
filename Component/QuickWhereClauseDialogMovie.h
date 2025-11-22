#ifndef QUICKWHERECLAUSEDIALOGMOVIE_H
#define QUICKWHERECLAUSEDIALOGMOVIE_H

#include "QuickWhereClauseDialog.h"

class QuickWhereClauseDialogMovie : public QuickWhereClauseDialog {
public:
  using QuickWhereClauseDialog::QuickWhereClauseDialog;

private:
  void CreatePrivateWidget() override;

  ColumnFilterLineEdit* m_Name{nullptr}; // shared
  ColumnFilterLineEdit* m_Size{nullptr};
  ColumnFilterLineEdit* m_Duration{nullptr};
  ColumnFilterLineEdit* m_Studio{nullptr};
  ColumnFilterLineEdit* m_Cast{nullptr};
  ColumnFilterLineEdit* m_Tags{nullptr}; // shared
};

#endif // QUICKWHERECLAUSEDIALOGMOVIE_H
