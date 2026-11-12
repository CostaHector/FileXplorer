#ifndef ACCOUNTLISTVIEW_H
#define ACCOUNTLISTVIEW_H

#include "CustomListView.h"
#include "AccountTableModel.h"
#include "AccountSortFilterProxyModel.h"
#include "AccountCardDelegate.h"

class AccountListView  : public CustomListView {
public:
  AccountListView(AccountTableModel* pwdModel, AccountSortFilterProxyModel* sortProxyModel, QWidget* parent = nullptr);

private:
  AccountTableModel* _pwdModel{nullptr};
  AccountSortFilterProxyModel* _sortProxyModel{nullptr};
  AccountCardDelegate* m_cardDelegate{nullptr};
};

#endif // ACCOUNTLISTVIEW_H
