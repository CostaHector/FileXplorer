#ifndef PASSWORDBOOK_H
#define PASSWORDBOOK_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QToolBar>
#include <QComboBox>
#include "AccountTableView.h"
#include "AccountListView.h"
#include "AccountFileAndAESKey.h"

class PasswordBook : public QMainWindow {
public:
  explicit PasswordBook(QWidget* parent = nullptr);

  void ReadSettings();
  void SetPWBookName();
  void SwitchToListView();

protected:
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

private:
  void Subscribe();
  void SetFilter(const QString& keywords);
  void SetEncFileFullPath(const QString& encFileFullPath);
  void SetAesKey(const QString& aesKey);

  enum ViewType {
    LIST_VIEW = 0,
    TABLE_VIEW = 1, // desktop list and detail share one
  };
  void onUpdateDetailView(const QModelIndex& proxyIndex);
  void onSave();
  void onGetRecordsFromInput();
  bool onAccountViewChanged(int viewType);

  void ShowPlainCSVContents();
  bool OpenEncFileLocatedIn() const;
  bool ExportToPlainCSVFile() const;
  int MarkSelectedRowsDeleted(bool bDeleted);

  bool InsertNRows(int insertIndexBefore, int rowCnt = 1);
  bool AppendNRows(int rowCnt = 1);
  std::pair<bool, QModelIndexList> GetSelectedRows() const;
  bool ScrollToSourceRow(int sourceRow);

  QToolBar* mTopAreaToolBar{nullptr}, *mLeftAreaToolBar{nullptr};

  // ---- 登录, 视图切换
  AccountFileAndAESKey* mAccountFileAndAesWid{nullptr};
  QComboBox* mAccountViewSwitcher{nullptr};

  // ---- 搜索 ----
  QLineEdit* mSearchText{nullptr};
  QAction *mSearchNowAct{nullptr};

  // ---- 行操作 ----
  QAction *mAppendRowAct{nullptr};
  QAction *mMarkRowsDeletedAct{nullptr};
  QAction *mRestoreRowsAct{nullptr};

  // ---- 导入 / 预览 / 导出 ----
  QAction *mImportFromCsvAct{nullptr};
  QAction *mPreviewPlainCsvAct{nullptr};
  QAction *mExportToPlainCsvAct{nullptr};

  // ---- 打开目录 ----
  QAction *mOpenContainingFolderAct{nullptr};

  // ---- 保存 ----
  QAction *mSaveChangesAct{nullptr};

  AccountTableModel* mPwdModel{nullptr};
  AccountSortFilterProxyModel* mSortProxyModel{nullptr};

  AccountListView* mAccountListView{nullptr};
  AccountTableView* mAccountTableView{nullptr};
  QStackedWidget* mMainWidget{nullptr};

  QTextEdit* mPlainCSVContentWid{nullptr};

  bool m_geometryRestoredOnce{false};
};
#endif // PASSWORDBOOK_H
