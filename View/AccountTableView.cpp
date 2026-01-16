#include "AccountTableView.h"
#include <QHeaderView>
#include <QDateTime>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <set>
#include "PwdPublicVariable.h"
#include "PwdTableEditActions.h"
#include "NotificatorMacro.h"

AccountTableView::AccountTableView(QWidget* parent)
    : CustomTableView{"PASSWORD_TABLEVIEW", parent}  //
{
  mPwdModel = new (std::nothrow) PwdTableModel{this};
  CHECK_NULLPTR_RETURN_VOID(mPwdModel);

  mSortProxyModel = new (std::nothrow) AccountSortFilterProxyModel{this};
  CHECK_NULLPTR_RETURN_VOID(mSortProxyModel);

  mSortProxyModel->setSourceModel(mPwdModel);
  mSortProxyModel->BindAccountsList(mPwdModel->mAccountsList);

  setModel(mSortProxyModel);
  setEditTriggers(QAbstractItemView::EditKeyPressed);
  InitTableView();
  setFont(PwdPublicVariable::TEXT_EDIT_FONT);
  Subscribe();
}

int AccountTableView::GetRowsCountFromUserInput(const QString& method) {
  bool isOk{false};
  int userInputCount{0};
#ifdef RUNNING_UNIT_TESTS
  std::tie(isOk, userInputCount) = AccountTableViewMock::getIntFromUserInputMock();
#else
  userInputCount = QInputDialog::getInt(this, method + " Rows Count", "number", 1, 0, 255, 1, &isOk);
#endif
  if (!isOk) {
    LOG_W("User input invalid row count[%d]", userInputCount);
    return 0;
  }
  return userInputCount;
}

void AccountTableView::Subscribe() {
  auto& ins = GetTableEditActionsInst();
  connect(ins.INSERT_A_ROW, &QAction::triggered, this, [this]() {
    int cnt = 1;
    InsertNRows(cnt);
  });
  connect(ins.INSERT_ROWS, &QAction::triggered, this, [this]() {
    int cnt = GetRowsCountFromUserInput("INSERT");
    if (cnt > 0) {
      InsertNRows(cnt);
    }
  });
  connect(ins.APPEND_ROWS, &QAction::triggered, this, [this]() {
    int cnt = GetRowsCountFromUserInput("APPEND");
    if (cnt > 0) {
      AppendNRows(cnt);
    }
  });
  connect(ins.DELETE_ROWS, &QAction::triggered, this, &AccountTableView::RemoveSelectedRows);
}

int AccountTableView::RemoveSelectedRows() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("Nothing were selected", "skip remove");
    return 0;
  }
  const int nSelectedRowsCnt = selectionModel()->selectedRows().size();
  QString msg{QString{"Are you sure to remove the %1 row(s) selected?<br/><b>NOT RECOVERABLE</b>"}.arg(nSelectedRowsCnt)};
  QMessageBox deleteConfirm;
  deleteConfirm.setWindowTitle("Delete Confirm?");
  deleteConfirm.setWindowIcon(QIcon(":/edit/DELETE_ROWS"));
  deleteConfirm.setIcon(QMessageBox::Icon::Warning);
  deleteConfirm.setText(msg);
  deleteConfirm.addButton(QMessageBox::StandardButton::Ok);
  deleteConfirm.addButton(QMessageBox::StandardButton::Cancel);

  int deleteCfmBtn = QMessageBox::StandardButton::Cancel;
#ifdef RUNNING_UNIT_TESTS
  deleteCfmBtn = AccountTableViewMock::cfmRemoveSelectedRowMock() ? QMessageBox::StandardButton::Ok : QMessageBox::StandardButton::Cancel;
#else
  deleteCfmBtn = deleteConfirm.exec();
#endif
  if (deleteCfmBtn != QMessageBox::StandardButton::Ok) {
    LOG_INFO_NP("User cancel delete", "Skip remove records");
    return 0;
  }
  std::set<int> selectedRows;
  for (const auto& rowIndex : selectionModel()->selectedRows()) {
    const auto& srcIndex = mSortProxyModel->mapToSource(rowIndex);
    selectedRows.insert(srcIndex.row());
  }
  const int rowsDeleted = mPwdModel->RemoveIndexes(selectedRows);
  if (rowsDeleted < 0) {
    LOG_ERR_P("Row(s) deleted failed", "selected:%d , code:%d", nSelectedRowsCnt, rowsDeleted);
    return rowsDeleted;
  }
  LOG_OK_P("Row(s) deleted ok", "All %d row(s) deleted successfully", nSelectedRowsCnt);
  return rowsDeleted;
}

void AccountTableView::InsertNRows(int rowCnt) {
  const QModelIndex verHeaderIndex = verticalHeader()->currentIndex();
  if (!verHeaderIndex.isValid()) {
    LOG_WARN_NP("No row selected", "Select a row at first");
    return;
  }
  const int insertIndexBefore = verHeaderIndex.row();
  LOG_INFO_P("Insert at", "Index %d with %d row(s)", insertIndexBefore, rowCnt);
  mPwdModel->InsertNRows(insertIndexBefore, rowCnt);
}

void AccountTableView::AppendNRows(int rowCnt) {
  int lastRowIndex = verticalHeader()->count();
  LOG_INFO_P("Append", "Index %d with %d row(s)", lastRowIndex, rowCnt);
  mPwdModel->InsertNRows(lastRowIndex, rowCnt);
}

QModelIndex AccountTableView::GetSourceIndex(const QModelIndex& proxyIndex) const {
  if (!proxyIndex.isValid()) {
    return {};
  }
  return mSortProxyModel->mapToSource(proxyIndex);
}

AccountInfo* AccountTableView::GetAccountInfoByCurrentIndex(const QModelIndex& proxyIndex) {
  QModelIndex srcCurrentIndex = GetSourceIndex(proxyIndex);
  if (!srcCurrentIndex.isValid()) {
    return nullptr;
  }
  return mPwdModel->rowDataAt(srcCurrentIndex.row());
}

bool AccountTableView::ExportPlainCSV() {
  QMessageBox confirmDialog(this);
  confirmDialog.setWindowTitle("Export Plaintext List?");
  confirmDialog.setWindowIcon(QIcon(":/edit/EXPORT"));
  confirmDialog.setText("WARNING: This may cause <b>information leakage</b>. Please keep the exported file secure.");
  confirmDialog.setIcon(QMessageBox::Warning);

  QPushButton* exportButton = confirmDialog.addButton("Continue Export", QMessageBox::AcceptRole);
  confirmDialog.addButton(QMessageBox::StandardButton::Cancel);
  exportButton->setIcon(QIcon(":/edit/EXPORT"));
  confirmDialog.setDefaultButton(QMessageBox::StandardButton::Cancel);

  QAbstractButton* pBtn{nullptr};
#ifdef RUNNING_UNIT_TESTS
  pBtn = AccountTableViewMock::cfmExportPlainCSVMock() ? exportButton : nullptr;
#else
  confirmDialog.exec();
  pBtn = confirmDialog.clickedButton();
#endif
  if (pBtn != exportButton) {
    LOG_INFO_NP("User cancelled plaintext export", "return");
    return false;
  }
  const QString timeStr = QDateTime::currentDateTime().toString();
  bool exportResult = mPwdModel->ExportToPlainCSV();
  LOG_OE_P(exportResult, "Export plain CSV", "Store the file securely\n%s", qPrintable(timeStr));
  return true;
}

void AccountTableView::SetFilter(const QString& keywords) {
  CHECK_NULLPTR_RETURN_VOID(mSortProxyModel);
  mSortProxyModel->setFilterFixedString(keywords);
}
