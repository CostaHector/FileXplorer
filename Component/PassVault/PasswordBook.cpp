#include "PasswordBook.h"
#include "FileTool.h"
#include "NotificatorMacro.h"
#include "Configuration.h"
#include "CSVInputDialog.h"
#include "InputDialogHelper.h"
#include "PublicCheckMacro.h"
#include "PathTool.h"
#include "StyleSheet.h"
#include "SizeTool.h"

#include <QFileInfo>
#include <QDateTime>

PasswordBook::PasswordBook(QWidget* parent)
  : QMainWindow{parent} {
  setObjectName("PasswordBook");

  mTopAreaToolBar = new (std::nothrow) QToolBar{"AccountTopAreaToolBar", this};
  CHECK_NULLPTR_RETURN_VOID(mTopAreaToolBar);
  {
    mTopAreaToolBar->setObjectName(mTopAreaToolBar->windowTitle());
    addToolBar(Qt::ToolBarArea::TopToolBarArea, mTopAreaToolBar);

    mAccountFileAndAesWid = new (std::nothrow) AccountFileAndAESKey{this};
    CHECK_NULLPTR_RETURN_VOID(mAccountFileAndAesWid);
    mTopAreaToolBar->addWidget(mAccountFileAndAesWid);
    mTopAreaToolBar->addSeparator();

    mSearchText = new (std::nothrow) QLineEdit{""};
    CHECK_NULLPTR_RETURN_VOID(mSearchText);
    mSearchText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSearchText->setPlaceholderText("Search records by keyword here");
    mSearchText->setClearButtonEnabled(true);
    mSearchNowAct = mSearchText->addAction(QIcon{":/pwd/action/SEARCH_BY"}, QLineEdit::LeadingPosition);
    mSearchNowAct->setToolTip("Search now.");
    mTopAreaToolBar->addWidget(mSearchText);
  }

  mLeftAreaToolBar = new (std::nothrow) QToolBar{"AccountLeftAreaToolBar", this};
  CHECK_NULLPTR_RETURN_VOID(mLeftAreaToolBar);
  {
    mLeftAreaToolBar->setObjectName(mLeftAreaToolBar->windowTitle());
    mLeftAreaToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    addToolBar(Qt::ToolBarArea::LeftToolBarArea, mLeftAreaToolBar);

    mAccountViewSwitcher = new QComboBox{this};
    CHECK_NULLPTR_RETURN_VOID(mAccountViewSwitcher);
    mAccountViewSwitcher->addItem(tr("List"),  ViewType::LIST_VIEW);
    mAccountViewSwitcher->addItem(tr("Table"), ViewType::TABLE_VIEW);
    mAccountViewSwitcher->setCurrentIndex(ViewType::LIST_VIEW);
    mLeftAreaToolBar->addWidget(mAccountViewSwitcher);
    mLeftAreaToolBar->addSeparator();

    // ---- 行操作 ----
    mAppendRowAct = mLeftAreaToolBar->addAction(QIcon{":/pwd/action/APPEND_ROW"}, tr("Append Row"));
    mAppendRowAct->setToolTip("Append a new empty row to the end of the list.");

    mMarkRowsDeletedAct = mLeftAreaToolBar->addAction(QIcon{":img/DELETE_FROM_TABLE"}, tr("Mark Deleted"));
    mMarkRowsDeletedAct->setToolTip("Mark the selected row(s) as deleted. Changes take effect after saving.");

    mRestoreRowsAct = mLeftAreaToolBar->addAction(QIcon{":/pwd/action/RESTORE"}, tr("Restore Rows"));
    mRestoreRowsAct->setToolTip("Clear the deleted mark on the selected row(s).");
    mLeftAreaToolBar->addSeparator();

    // ---- 导入 / 预览 / 导出 ----
    mImportFromCsvAct = mLeftAreaToolBar->addAction(QIcon{":/pwd/action/LOAD_FROM_INPUT"}, tr("Import from CSV"));
    mImportFromCsvAct->setToolTip("Import account records from CSV text you provide.");
    mPreviewPlainCsvAct = mLeftAreaToolBar->addAction(QIcon{":/pwd/action/SHOW_CSV_CONTENTS"}, tr("Preview Plain CSV"));
    mPreviewPlainCsvAct->setToolTip("Show the account records as plaintext CSV in a message box.");
    mExportToPlainCsvAct = mLeftAreaToolBar->addAction(QIcon{":/pwd/action/EXPORT"}, tr("Export as Plain CSV"));
    mExportToPlainCsvAct->setToolTip("Export account records to a plaintext CSV file. "
                                     "<b>Unencrypted and human-readable.</b>");
    mLeftAreaToolBar->addSeparator();

    // ---- 打开目录 ----
    mOpenContainingFolderAct = mLeftAreaToolBar->addAction(QIcon{":/pwd/action/OPEN_DIRECTORY"}, tr("Open Containing Folder"));
    mOpenContainingFolderAct->setToolTip("Open the folder that contains the data file in the system file explorer.");
    mLeftAreaToolBar->addSeparator();

    // ---- 保存 ----
    mSaveChangesAct = mLeftAreaToolBar->addAction(QIcon{":/pwd/action/SAVE_CHANGES"}, tr("Save Changes"));
    mSaveChangesAct->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_S));
    mSaveChangesAct->setToolTip(QString{"<b>%1 (%2)</b><br/>Save all changes to the encrypted local file."}.arg(mSaveChangesAct->text(), mSaveChangesAct->shortcut().toString()));
  }

  mPwdModel = new (std::nothrow) AccountTableModel{mAccountFileAndAesWid->GetAccountFileFullPath(), mAccountFileAndAesWid->GetAesKey(), this};
  CHECK_NULLPTR_RETURN_VOID(mPwdModel);

  mSortProxyModel = new (std::nothrow) AccountSortFilterProxyModel{this};
  CHECK_NULLPTR_RETURN_VOID(mSortProxyModel);
  mSortProxyModel->setSourceModel(mPwdModel);

  mMainWidget = new QStackedWidget{this};
  onAccountViewChanged(mAccountViewSwitcher->currentIndex());
  setCentralWidget(mMainWidget);

  onDecryptResultChanged(mPwdModel->IsDecryptedOk());

  Subscribe();

  setWindowIcon(QIcon(":/pwd/app/PASSWORD_BOOK"));
  SetPWBookName();
}

void PasswordBook::showEvent(QShowEvent* event) {
  if (!m_geometryRestoredOnce) {
    ReadSettings();
  }
  StyleSheet::UpdateTitleBar(this);
  QMainWindow::showEvent(event);
}

void PasswordBook::closeEvent(QCloseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event)
  Configuration().setValue("Geometry/PasswordBook", saveGeometry());
  QMainWindow::closeEvent(event);
}

void PasswordBook::ReadSettings() {
  m_geometryRestoredOnce = true;
  if (Configuration().contains("Geometry/PasswordBook")) {
    restoreGeometry(Configuration().value("Geometry/PasswordBook").toByteArray());
  } else {
    setGeometry(SizeTool::DEFAULT_GEOMETRY);
  }
}

void PasswordBook::Subscribe() {
  connect(mAccountFileAndAesWid, &AccountFileAndAESKey::encFileFullPathChanged, mPwdModel, &AccountTableModel::SetEncFileFullPath);
  connect(mAccountFileAndAesWid, &AccountFileAndAESKey::aesKeyChanged, mPwdModel, &AccountTableModel::SetAesKey);
  connect(mAccountFileAndAesWid, &AccountFileAndAESKey::encFileFullPathAndAesKeyChanged, mPwdModel, &AccountTableModel::SetEncFileFullPathAndAesKey);
  connect(mPwdModel, &AccountTableModel::decryptedResultChanged, this, &PasswordBook::onDecryptResultChanged);

  connect(mSearchNowAct, &QAction::triggered, mSearchText, &QLineEdit::returnPressed);
  connect(mSearchText, &QLineEdit::returnPressed, this, [this]() { SetFilter(mSearchText->text()); });
  connect(mAccountViewSwitcher, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PasswordBook::onAccountViewChanged);

  connect(mAppendRowAct, &QAction::triggered, this, [this]() { AppendNRows(1); });
  connect(mMarkRowsDeletedAct, &QAction::triggered, this, [this](){ MarkSelectedRowsDeleted(true); });
  connect(mRestoreRowsAct, &QAction::triggered, this, [this](){ MarkSelectedRowsDeleted(false); });

  connect(mImportFromCsvAct, &QAction::triggered, this, &PasswordBook::onGetRecordsFromInput);
  connect(mPreviewPlainCsvAct, &QAction::triggered, this, &PasswordBook::ShowPlainCSVContents);
  connect(mExportToPlainCsvAct, &QAction::triggered, this, &PasswordBook::ExportToPlainCSVFile);

  connect(mOpenContainingFolderAct, &QAction::triggered, this, &PasswordBook::OpenEncFileLocatedIn);
  connect(mSaveChangesAct, &QAction::triggered, this, &PasswordBook::onSave);
}

void PasswordBook::SetPWBookName() {
  QString title;
  title.reserve(40);
  title += "Password Book";
  title += " | ";
  title += mAccountFileAndAesWid->GetAccountFileFullPath();
  setWindowTitle(title);
}

void PasswordBook::onSave() {
  const bool saveResult = mPwdModel->onSave();
  LOG_OE_NP(saveResult, "Save at", QDateTime::currentDateTime().toString());
}

void PasswordBook::onGetRecordsFromInput() {
  CSVInputDialog dlg{mAccountFileAndAesWid->GetAesKey()};
  if (dlg.exec() != QDialog::DialogCode::Accepted) {
    return;
  }
  mPwdModel->AppendAccountRecords(dlg.GetAppendAccounts());
}

void PasswordBook::ShowPlainCSVContents() {
  if (mPlainCSVContentWid == nullptr) {
    mPlainCSVContentWid = new (std::nothrow) QTextEdit{this};
    CHECK_NULLPTR_RETURN_VOID(mPlainCSVContentWid);
    mPlainCSVContentWid->setWindowFlags(Qt::Dialog);
    mPlainCSVContentWid->setWindowTitle("Show plain CSV Contents");
    mPlainCSVContentWid->setWindowIcon(QIcon{":/edit/SHOW_CSV_CONTENTS"});
    mPlainCSVContentWid->setReadOnly(true);
    mPlainCSVContentWid->setMinimumSize(600, 400);
  }
  mPlainCSVContentWid->setPlainText(mPwdModel->GetExportCSVRecords());
  mPlainCSVContentWid->raise();
  mPlainCSVContentWid->show();
}

bool PasswordBook::OpenEncFileLocatedIn() const {
  const QString locatedInPath = QFileInfo{mAccountFileAndAesWid->GetAccountFileFullPath()}.absolutePath();
  const QUrl url = QUrl::fromLocalFile(PathTool::absolutePath(locatedInPath));
  return FileTool::OpenLocalFileUsingDesktopService(url.toLocalFile());
}

void PasswordBook::SetFilter(const QString& keywords) {
  mSortProxyModel->setFilterFixedString(keywords);
}

void PasswordBook::onDecryptResultChanged(bool bDecryptOk) {
  mMainWidget->setEnabled(bDecryptOk);
  mLeftAreaToolBar->setEnabled(bDecryptOk);
}

bool PasswordBook::ExportToPlainCSVFile() const {
  if (!InputDialogHelper::YesOrCancelBox(QMessageBox::Warning, QIcon(":/edit/EXPORT"),
                                         "Export Plaintext List?", "WARNING: This may cause <b>information leakage</b>. Please keep the exported file secure.",
                                         "", "")) {
    LOG_INFO_NP("User cancelled plaintext export", "return");
    return false;
  }
  const QString timeStr = QDateTime::currentDateTime().toString();
  bool exportResult = mPwdModel->ExportToPlainCSVFile();
  LOG_OE_P(exportResult, "Export plain CSV", "Store the file securely\n%s", qPrintable(timeStr));
  return true;
}

bool PasswordBook::onAccountViewChanged(int viewType) {
  switch (viewType) {
    case ViewType::LIST_VIEW: {
      if (mAccountListView == nullptr) {
        mAccountListView = new (std::nothrow) AccountListView{mPwdModel, mSortProxyModel, this};
        mMainWidget->addWidget(mAccountListView);
      }
      break;
    }
    case ViewType::TABLE_VIEW: {
      if (mAccountTableView == nullptr) {
        mAccountTableView = new (std::nothrow) AccountTableView{mPwdModel, mSortProxyModel, this};
        mMainWidget->addWidget(mAccountTableView);
        connect(mAccountTableView, &AccountTableView::insertLinesReq, this, &PasswordBook::InsertNRows);
      }
      break;
    }
    default:
      return false;
  }
  mMainWidget->setCurrentIndex(viewType);
  return true;
}

std::pair<bool, QModelIndexList> PasswordBook::GetSelectedRows() const {
  switch (mMainWidget->currentIndex()) {
    case ViewType::LIST_VIEW: {
      if (mAccountListView == nullptr) {
        return {false, {}};
      }
      return {true, mAccountListView->selectedRowsSource()};
      break;
    }
    case ViewType::TABLE_VIEW: {
      if (mAccountTableView == nullptr) {
        return {false, {}};
      }
      return {true, mAccountTableView->selectedRowsSource()};
      break;
    }
    default:
      return {false, {}};
  }
}

int PasswordBook::MarkSelectedRowsDeleted(bool bDeleted) {
  bool bGetOk{false};
  QModelIndexList srcIndexes;
  std::tie(bGetOk, srcIndexes) = GetSelectedRows();
  if (!bGetOk) {
    LOG_ERR_NP("Get Selected Rows failed", "Cannot mark");
    return -1;
  }
  if (srcIndexes.isEmpty()) {
    LOG_INFO_NP("Nothing were selected", "Skip mark");
    return 0;
  }
  const int rowsMarked = mPwdModel->MarkDeletedIndexes(srcIndexes, bDeleted);
  if (rowsMarked < 0) {
    LOG_ERR_P("Row(s) mark failed", "selected:%d , code:%d", srcIndexes.size(), rowsMarked);
    return rowsMarked;
  }
  LOG_OK_P("Row(s) marked ok", "All %d row(s) marked successfully", srcIndexes.size());
  return rowsMarked;
}

bool PasswordBook::InsertNRows(int insertIndexBefore, int rowCnt) {
  if (mPwdModel->InsertNRows(insertIndexBefore, rowCnt) <= 0) {
    return false;
  }
  return ScrollToSourceRow(insertIndexBefore);
}

bool PasswordBook::AppendNRows(int rowCnt) {
  return InsertNRows(mPwdModel->rowCount(), rowCnt);
}

bool PasswordBook::ScrollToSourceRow(int sourceRow) {
  QAbstractItemView* view = nullptr;
  switch (mMainWidget->currentIndex()) {
    case ViewType::LIST_VIEW:  view = mAccountListView;  break;
    case ViewType::TABLE_VIEW: view = mAccountTableView; break;
    default: return false;
  }
  if (!view || !mSortProxyModel || !mPwdModel) {
    return false;
  }

  const QModelIndex srcIdx = mPwdModel->index(sourceRow, 0);
  if (!srcIdx.isValid()) {
    return false;
  }

  // 关键：source → proxy，排序/过滤后才对应视图里的真实行
  const QModelIndex proxyIdx = mSortProxyModel->mapFromSource(srcIdx);
  if (!proxyIdx.isValid()) {
    // 新行被过滤规则挡掉了，视图里根本不存在，滚动无意义
    LOG_INFO_NP("New row filtered out", "Skip scroll");
    return true;
  }

  view->scrollTo(proxyIdx, QAbstractItemView::EnsureVisible);
  view->setCurrentIndex(proxyIdx);
  return true;
}

