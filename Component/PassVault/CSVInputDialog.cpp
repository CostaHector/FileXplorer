#include "CSVInputDialog.h"
#include "AccountFileHelper.h"
#include "PublicCheckMacro.h"
#include "NotificatorMacro.h"
#include "SystemPath.h"
#include "StyleSheet.h"

#include <QIcon>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMenu>
#include <QFileDialog>

DragDropTextEdit::DragDropTextEdit(const QString& aesKey, QWidget* parent) : QPlainTextEdit(parent), mAesKey{aesKey} {
  setAcceptDrops(true);

  mMenu = createStandardContextMenu();
  mMenu->addSeparator();
  mSelectEncCsvFile = mMenu->addAction(QIcon{":/edit/PARSE_ENCRYPT_CSV_FILE"}, tr("select encrypt CSV file"));
  connect(mSelectEncCsvFile, &QAction::triggered, this, &DragDropTextEdit::onSelectEncCsvFileToParse);
}

void DragDropTextEdit::dragEnterEvent(QDragEnterEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  const QMimeData* mimeData = event->mimeData();
  CHECK_NULLPTR_RETURN_VOID(mimeData);
  if (!isContainsOneFile(*mimeData)) {
    event->ignore();
    return;
  }
  event->acceptProposedAction();
}

void DragDropTextEdit::dropEvent(QDropEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  const QMimeData* mimeData = event->mimeData();
  CHECK_NULLPTR_RETURN_VOID(mimeData);
  if (!isContainsOneFile(*mimeData)) {
    event->ignore();
    return;
  }
  const QList<QUrl> lst = mimeData->urls();
  QString encryptCsvFilePath = lst.front().toLocalFile();
  if (!ParseEncryptCsvFileContents(encryptCsvFilePath)) {
    event->ignore();
    return;
  }
  event->acceptProposedAction();
}

void DragDropTextEdit::contextMenuEvent(QContextMenuEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  CHECK_NULLPTR_RETURN_VOID(mMenu);
  mMenu->exec(event->globalPos());
  event->accept();
}

bool DragDropTextEdit::isContainsOneFile(const QMimeData& mimeData) {
  return mimeData.hasUrls() && mimeData.urls().size() == 1;
}

bool DragDropTextEdit::onSelectEncCsvFileToParse() {
  const QString encryptCsvFilePath = QFileDialog::getOpenFileName(this, "Select encrypt csv file", SystemPath::RoamingPath(), "CSV Files (*.csv)");
  if (encryptCsvFilePath.isEmpty()) {
    LOG_INFO_NP("Cancel", "user cancel select encrypt file");
    return false;
  }
  return ParseEncryptCsvFileContents(encryptCsvFilePath);
}

bool DragDropTextEdit::ParseEncryptCsvFileContents(const QString& encryptCsvFilePath) {
  emit windowTitleChanged(encryptCsvFilePath);

  QString plainCsvFileContent;
  if (!AccountFileHelper::ParseEncryptCsvFile(encryptCsvFilePath, plainCsvFileContent, mAesKey)) {
    setPlainText(QString{"Parse encrypt csv file[%1] failed"}.arg(encryptCsvFilePath));
    return false;
  }
  setPlainText(plainCsvFileContent);
  return true;
}

CSVInputDialog::CSVInputDialog(const QString& aesKey, QWidget* parent) : QDialog{parent}, mAesKey{aesKey}
{
  textEdit = new (std::nothrow) DragDropTextEdit(mAesKey, this);
  textEdit->setProperty("UseCodeFontFamily", true);
  CHECK_NULLPTR_RETURN_VOID(textEdit);
  textEdit->setPlaceholderText("Enter your csv contents here or drag a encrypt csv file here...");

  buttonBox = new (std::nothrow) QDialogButtonBox(this);
  CHECK_NULLPTR_RETURN_VOID(buttonBox);
  buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
  pOkBtn = buttonBox->button(QDialogButtonBox::Ok);
  pOkBtn->setText(tr("Accept and Quit"));
  pOkBtn->setToolTip("Pass the result of csv lines to caller and quit");

  StyleSheet::UpdateApplyPushButton(pOkBtn);
  pParseBtn = buttonBox->button(QDialogButtonBox::Help);
  pParseBtn->setText(tr("Parse"));
  pParseBtn->setToolTip("Parse current text in qtextedit");

  mainLayout = new (std::nothrow) QVBoxLayout(this);
  CHECK_NULLPTR_RETURN_VOID(mainLayout);
  mainLayout->addWidget(textEdit);
  mainLayout->addWidget(buttonBox);

  connect(buttonBox, &QDialogButtonBox::accepted, this, &CSVInputDialog::onAcceptAndQuit);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(pParseBtn, &QPushButton::clicked, this, &CSVInputDialog::onParseRequest);

  setWindowIcon(QIcon(":/edit/LOAD_FROM_INPUT"));
  setWindowTitle("Get record(s) from CSV input");
}

bool CSVInputDialog::onParseRequest() {
  const QString text = textEdit->toPlainText();

  QList<int> incompleteLines;
  mAppendAccounts = AccountFileHelper::GetAccountsFromPlainString(text, incompleteLines);
  const bool bAllLinesOk{incompleteLines.isEmpty()};

  QString msg;
  if (bAllLinesOk) {
    msg = QString{"%1 line(s) incomplete. Need fix at first"}.arg(incompleteLines.size());
    LOG_ERR_NP("Some unexpected line(s) find", qPrintable(msg));
  } else {
    msg = QString{"%1 line(s) accounts found."}.arg(mAppendAccounts.size());
    LOG_OK_NP("Parse ok", qPrintable(msg));
  }
  setWindowTitle(msg);
  return bAllLinesOk;
}

void CSVInputDialog::onAcceptAndQuit() {
  if (!onParseRequest()) {
    return;
  }
  accept();
}
