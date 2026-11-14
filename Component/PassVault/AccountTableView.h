#ifndef ACCOUNTTABLEVIEW_H
#define ACCOUNTTABLEVIEW_H

#include "CustomTableView.h"
#include "AccountTableModel.h"
#include "AccountSortFilterProxyModel.h"

class AccountTableView : public CustomTableView {
  Q_OBJECT
public:
  explicit AccountTableView(AccountTableModel* pwdModel, AccountSortFilterProxyModel* sortProxyModel, QWidget* parent = nullptr);

signals:
  void insertLinesReq(int pos, int cnt);

private:
  bool EmitInsertLinesReq();

  QAction* mInsertAnRowAct{nullptr};
  AccountTableModel* _pwdModel{nullptr};
  AccountSortFilterProxyModel* _sortProxyModel{nullptr};
};

#endif // ACCOUNTTABLEVIEW_H
