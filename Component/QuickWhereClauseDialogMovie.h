#ifndef QUICKWHERECLAUSEDIALOGMOVIE_H
#define QUICKWHERECLAUSEDIALOGMOVIE_H

#include "QuickWhereClauseDialog.h"

class QuickWhereClauseDialogMovie : public QuickWhereClauseDialog {
public:
  using QuickWhereClauseDialog::QuickWhereClauseDialog;

private:
  void CreatePrivateWidget() override;
  void InitPrivateLayout() override;
  void PrivateSubscribe() override;
  void onConditionsChanged() override;

  QLineEdit* m_Name{nullptr}; // shared
  QLineEdit* m_Size{nullptr};
  QLineEdit* m_Duration{nullptr};
  QLineEdit* m_Studio{nullptr};
  QLineEdit* m_Cast{nullptr};
  QLineEdit* m_Tags{nullptr}; // shared
};

#endif // QUICKWHERECLAUSEDIALOGMOVIE_H
