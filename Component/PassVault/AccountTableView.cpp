#include "AccountTableView.h"
#include "StyleSheetEditDelegate.h"
#include "AccountInfo.h"
#include <QHeaderView>

AccountTableView::AccountTableView(AccountTableModel* pwdModel, AccountSortFilterProxyModel* sortProxyModel, QWidget* parent)
  : CustomTableView{"PASSWORD_TABLEVIEW", parent}, _pwdModel{pwdModel}, _sortProxyModel{sortProxyModel}  //
{
  setModel(_sortProxyModel);

  StyleSheetEditDelegate* detailColumnDelegator = new (std::nothrow) StyleSheetEditDelegate{AccountInfo::DATA_TYPE_ROLE, {AccountInfo::Role::OTHERS_STR_ROLE - AccountInfo::Role::DEF_BEGIN_ROLE}, this};
  setItemDelegate(detailColumnDelegator);

  InitTableView();

  mInsertAnRowAct = new QAction{QIcon{":img/INSERT_INTO_TABLE"}, tr("Insert a row"), this};
  mInsertAnRowAct->setToolTip("Insert a row before current line");
  InsertActionAtVerHeaderMenu(mInsertAnRowAct, 0);

  connect(mInsertAnRowAct, &QAction::triggered, this, &AccountTableView::EmitInsertLinesReq);
}

bool AccountTableView::EmitInsertLinesReq() {
  const QModelIndex verHeaderIndex = verticalHeader()->currentIndex();
  if (!verHeaderIndex.isValid()) {
    return false;
  }
  const int insertIndexBefore = verHeaderIndex.row();
  emit insertLinesReq(insertIndexBefore, 1);
  return true;
}