#ifndef MULTIPARDIALOG_H
#define MULTIPARDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include "MultiParView.h"
#include "DialogWithSearchLine.h"
#include <QPlainTextEdit>

class MultiParDialog : public DialogWithSearchLine {
public:
  explicit MultiParDialog(ParVerifyInfomationList&& resultList, QWidget* parent = nullptr);

private:
  void subscribe();
  void onStartFilter(const QString& searchText) override;

  MultiParView* m_multiParTableView{nullptr};
  QPlainTextEdit* m_verifyCliOutput{nullptr};
  QDialogButtonBox* m_dlgBtnBox{nullptr};
};

#endif // MULTIPARDIALOG_H
