#include "PasswordBook.h"
#include "PublicTool.h"
#include "PwdPublicVariable.h"
#include "PwdTableEditActions.h"
#include "NotificatorMacro.h"
#include "StyleSheet.h"
#include "MemoryKey.h"
#include "LoginQryWidget.h"
#include <QDateTime>
#include <QFileInfo>

PasswordBook* PasswordBook::Creater(QWidget* parent) {
  LoginQryWidget* loginQryWidget = new (std::nothrow) LoginQryWidget;
  loginQryWidget->onStartTimer();
  if (loginQryWidget->exec() != QDialog::DialogCode::Accepted) {
    LOG_INFO_NP("Cancel", "user skip");
    return nullptr;
  }
  return new (std::nothrow) PasswordBook{parent};
}

PasswordBook::PasswordBook(QWidget* parent)
  : QMainWindow{parent} {
  setObjectName("PasswordBook");
  mAccountListView = new (std::nothrow) AccountTableView{this};
  CHECK_NULLPTR_RETURN_VOID(mAccountListView);
  if (!mAccountListView->IsLoadSucceed()) {
    LOG_CRIT_NP("Load failed", "Key error");
    this->setEnabled(false);
  }

  mAccountDetailView = new (std::nothrow) AccountDetailView{"AccountDetailView", this};
  CHECK_NULLPTR_RETURN_VOID(mAccountDetailView);
  mAccountDetailView->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
  addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, mAccountDetailView);

  mMainWidget = new QStackedWidget{this};
  mMainWidget->addWidget(mAccountListView);
  mMainWidget->setCurrentIndex(ViewType::LIST_VIEW);
  setCentralWidget(mMainWidget);

  const auto& tblEditInst = GetTableEditActionsInst();
  mSearchText = new (std::nothrow) QLineEdit{""};
  CHECK_NULLPTR_RETURN_VOID(mSearchText);
  mSearchText->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  mSearchText->setPlaceholderText("Input to search accounts by keywords here");
  mSearchText->setClearButtonEnabled(true);
  mSearchText->addAction(tblEditInst.SEARCH_BY, QLineEdit::ActionPosition::LeadingPosition);

  QToolBar* insertRowsTB = new (std::nothrow) QToolBar{"Insert Rows", this};
  CHECK_NULLPTR_RETURN_VOID(insertRowsTB);
  insertRowsTB->setOrientation(Qt::Orientation::Vertical);
  insertRowsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  insertRowsTB->addAction(tblEditInst.INSERT_A_ROW);
  insertRowsTB->addAction(tblEditInst.INSERT_ROWS);
  SetLayoutAlightment(insertRowsTB->layout(), Qt::AlignmentFlag::AlignLeft);

  mToolBar = new (std::nothrow) QToolBar{"EditToolbar", this};
  CHECK_NULLPTR_RETURN_VOID(mToolBar);
  mToolBar->setObjectName(mToolBar->windowTitle());
  mToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  mToolBar->addWidget(insertRowsTB);
  mToolBar->addAction(tblEditInst.APPEND_ROWS);
  mToolBar->addAction(tblEditInst.DELETE_ROWS);
  mToolBar->addSeparator();
  mToolBar->addWidget(mSearchText);
  mToolBar->addSeparator();
  mToolBar->addAction(tblEditInst.LOAD_FROM_INPUT);
  mToolBar->addAction(tblEditInst.SHOW_PLAIN_CSV_CONTENT);
  mToolBar->addAction(tblEditInst.EXPORT_TO_PLAIN_CSV);
  mToolBar->addAction(tblEditInst.OPEN_DIRECTORY);
  mToolBar->addAction(tblEditInst.SAVE_CHANGES);
  addToolBar(Qt::ToolBarArea::TopToolBarArea, mToolBar);
  mToolBar->setFont(StyleSheet::TEXT_EDIT_FONT);

  mStatusBar = new (std::nothrow) QStatusBar{this};
  CHECK_NULLPTR_RETURN_VOID(mStatusBar);
  setStatusBar(mStatusBar);

  Subscribe();

  ReadSettings();
  setWindowIcon(QIcon(":/PASSWORD_TABLE"));
  SetPWBookName();
}

void PasswordBook::closeEvent(QCloseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event)
  Configuration().setValue("PASSWORD_TABLEVIEW_STATE", saveState());
  Configuration().setValue("ACCOUNT_DETAIL_VIEW_GEOMETRY", mAccountDetailView->saveGeometry());
  QMainWindow::closeEvent(event);
}

void PasswordBook::ReadSettings() {
  if (Configuration().contains("PASSWORD_TABLEVIEW_STATE")) {
    restoreState(Configuration().value("PASSWORD_TABLEVIEW_STATE").toByteArray());
  }
  if (Configuration().contains("ACCOUNT_DETAIL_VIEW_GEOMETRY")) {
    mAccountDetailView->restoreGeometry(Configuration().value("ACCOUNT_DETAIL_VIEW_GEOMETRY").toByteArray());
  }
}

void PasswordBook::Subscribe() {
  connect(mAccountListView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &PasswordBook::onUpdateDetailView);
  connect(mSearchText, &QLineEdit::returnPressed, this, [this]() { mAccountListView->SetFilter(mSearchText->text()); });

  auto& ins = GetTableEditActionsInst();
  connect(ins.SHOW_PLAIN_CSV_CONTENT, &QAction::triggered, this, &PasswordBook::ShowPlainCSVContents);
  connect(ins.EXPORT_TO_PLAIN_CSV, &QAction::triggered, mAccountListView, &AccountTableView::ExportPlainCSV);
  connect(ins.LOAD_FROM_INPUT, &QAction::triggered, this, &PasswordBook::onGetRecordsFromInput);
  connect(ins.OPEN_DIRECTORY, &QAction::triggered, this, &PasswordBook::openEncFileLocatedIn);
  connect(ins.SAVE_CHANGES, &QAction::triggered, this, &PasswordBook::onSave);
  connect(ins.SEARCH_BY, &QAction::triggered, mSearchText, &QLineEdit::returnPressed);
}

void PasswordBook::SetPWBookName() {
  QString title;
  title.reserve(40);
  title += "Pwd";
  title += " | ";
  title += AccountStorage::GetFullEncCsvFilePath();
  setWindowTitle(title);
}

void PasswordBook::onUpdateDetailView(const QModelIndex& proxyIndex) {
  mMainWidget->setCurrentIndex(ViewType::DETAIL_VIEW);
  auto* pData = mAccountListView->GetAccountInfoByCurrentIndex(proxyIndex);
  mAccountDetailView->UpdateDisplay(pData);
}

void PasswordBook::onSave() {
  using namespace PwdPublicVariable;
  QString changedMessage;
  SAVE_RESULT saveResult = mAccountListView->mPwdModel->onSave(&changedMessage);

  QString message;
  message.reserve(30);
  message += SAVE_RESULT_STR[(int) saveResult];
  message += ' ';
  message += "Try save record(s) at ";
  message += QDateTime::currentDateTime().toString();
  switch (saveResult) {
    case SAVE_RESULT::FAILED:
      LOG_ERR_NP("Failed to save", message);
      break;
    case SAVE_RESULT::SKIP:
      LOG_WARN_NP("Nothing changed(Skip save)", message);
      break;
    default:
      LOG_OK_NP("Save successfully", message);
      break;
  }
  mStatusBar->showMessage(message);
}

void PasswordBook::onGetRecordsFromInput() {
  if (mCsvInputDialog == nullptr) {
    mCsvInputDialog = new (std::nothrow) CSVInputDialog{this};
    CHECK_NULLPTR_RETURN_VOID(mCsvInputDialog);
    mCsvInputDialog->setWindowTitle("Input plain csv accounts info here");
    connect(mCsvInputDialog, &CSVInputDialog::accepted, this, &PasswordBook::onLoadRecordsFromCSVInput);
  }
  mCsvInputDialog->raise();
  mCsvInputDialog->show();
}

void PasswordBook::onLoadRecordsFromCSVInput() {
  CHECK_NULLPTR_RETURN_VOID(mCsvInputDialog);
  if (mCsvInputDialog->tempAccounts.isEmpty()) {
    LOG_INFO_NP("Skip", "No record in CSV plain text input");
    return;
  }
  mAccountListView->mPwdModel->AppendAccountRecords(mCsvInputDialog->tempAccounts);
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
  mPlainCSVContentWid->setPlainText(mAccountListView->mPwdModel->GetExportCSVRecords());
  mPlainCSVContentWid->raise();
  mPlainCSVContentWid->show();
}

bool PasswordBook::openEncFileLocatedIn() const {
  const QString locatedIn{QFileInfo(AccountStorage::GetFullEncCsvFilePath()).absolutePath()};
  const QUrl url = QUrl::fromLocalFile(locatedIn);
  const bool bRet = FileTool::OpenLocalFileUsingDesktopService(url.toLocalFile());
  LOG_OE_P(bRet, "Open enc located in", "path:%s,ret:%d", qPrintable(locatedIn), bRet);
  return bRet;
}
