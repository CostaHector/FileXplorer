#ifndef ACCOUNTTABLEVIEW_H
#define ACCOUNTTABLEVIEW_H

#include <QSortFilterProxyModel>
#include "CustomTableView.h"
#include "PwdTableModel.h"

#ifdef RUNNING_UNIT_TESTS
namespace AccountTableViewMock{
inline std::pair<bool, int>& getIntFromUserInputMock() {
  static std::pair<bool, int> accept2IntNumberPair{false, 0};
  return accept2IntNumberPair;
}
inline bool& cfmRemoveSelectedRowMock() {
  static bool cfmRemove = false;
  return cfmRemove;
}
inline bool& cfmExportPlainCSVMock() {
  static bool cfmExport = false;
  return cfmExport;
}
inline void clear() {
  getIntFromUserInputMock() = std::pair<bool, int>(false, 0);
  cfmRemoveSelectedRowMock() = false;
  cfmExportPlainCSVMock() = false;
}
}
#endif

class AccountTableView : public CustomTableView {
public:
  friend class PasswordManager;
  explicit AccountTableView(QWidget* parent = nullptr);
  void Subscribe();
  int GetRowsCountFromUserInput(const QString& method);
  int RemoveSelectedRows();
  void InsertNRows(int rowCnt = 1);
  void AppendNRows(int rowCnt);
  QModelIndex GetSourceIndex(const QModelIndex& proxyIndex) const;
  AccountInfo* GetAccountInfoByCurrentIndex(const QModelIndex& proxyIndex);
  bool IsLoadSucceed() const {
    return mPwdModel->GetLoadResult();
  }
  void SetFilter(const QString& keywords);

private:
  bool ExportPlainCSV();
  PwdTableModel* mPwdModel{nullptr};
  AccountSortFilterProxyModel* mSortProxyModel{nullptr};
};

#endif // ACCOUNTTABLEVIEW_H
