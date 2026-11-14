#include "AccountListView.h"

AccountListView::AccountListView(AccountTableModel* pwdModel, AccountSortFilterProxyModel* sortProxyModel, QWidget* parent)
  : CustomListView{"PASSWORD_LISTVIEW", parent}, _pwdModel{pwdModel}, _sortProxyModel{sortProxyModel} {
  setModel(_sortProxyModel);
  InitListView();

  m_cardDelegate = new AccountCardDelegate(this);
  setItemDelegate(m_cardDelegate);
  setUniformItemSizes(true);
  // 双击或按 F2 进入编辑态
}
