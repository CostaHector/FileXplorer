#ifndef PASSWORDBOOK_H
#define PASSWORDBOOK_H

#include <QMainWindow>
#include <QLineEdit>
#include <QStatusBar>
#include <QTextEdit>
#include <QToolBar>
#include "AccountDetailView.h"
#include "AccountTableView.h"
#include "CSVInputDialog.h"

class PasswordBook : public QMainWindow {
 public:
  explicit PasswordBook();
  void closeEvent(QCloseEvent* event) override;
  void ReadSettings();
  void Subscribe();
  void SetPWBookName();

 private:
  void onUpdateDetailView(const QModelIndex& proxyIndex);
  void onSave();
  void onGetRecordsFromInput();
  void onLoadRecordsFromCSVInput();
  void ShowPlainCSVContents();
  bool openEncFileLocatedIn() const;

  AccountTableView* mAccountListView {nullptr};
  AccountDetailView* mAccountDetailView{nullptr};
  QLineEdit* mSearchText{nullptr};
  QToolBar* mToolBar {nullptr};
  QStatusBar* mStatusBar {nullptr};
  CSVInputDialog* mCsvInputDialog{nullptr};
  QTextEdit *mPlainCSVContentWid{nullptr};
};
#endif // PASSWORDBOOK_H
