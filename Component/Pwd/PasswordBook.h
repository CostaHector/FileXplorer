#ifndef PASSWORDBOOK_H
#define PASSWORDBOOK_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QStatusBar>
#include <QTextEdit>
#include <QToolBar>
#include "AccountDetailView.h"
#include "AccountTableView.h"
#include "CSVInputDialog.h"

class PasswordBook : public QMainWindow {
public:
  static PasswordBook* Creater(QWidget* parent);
  explicit PasswordBook(QWidget* parent = nullptr);

  void ReadSettings();
  void closeEvent(QCloseEvent* event) override;
  void Subscribe();
  void SetPWBookName();
  void SwitchToListView();
private:
  enum ViewType {
    LIST_VIEW = 1,
    DETAIL_VIEW = 1, // desktop list and detail share one
  };
  void onUpdateDetailView(const QModelIndex& proxyIndex);
  void onSave();
  void onGetRecordsFromInput();
  void onLoadRecordsFromCSVInput();
  void ShowPlainCSVContents();
  bool openEncFileLocatedIn() const;

  AccountTableView* mAccountListView{nullptr};
  AccountDetailView* mAccountDetailView{nullptr};
  QStackedWidget* mMainWidget{nullptr};
  QLineEdit* mSearchText{nullptr};
  QToolBar* mToolBar{nullptr};
  QStatusBar* mStatusBar{nullptr};
  CSVInputDialog* mCsvInputDialog{nullptr};
  QTextEdit* mPlainCSVContentWid{nullptr};
};
#endif // PASSWORDBOOK_H
