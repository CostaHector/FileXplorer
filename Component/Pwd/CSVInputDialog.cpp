#include "CSVInputDialog.h"
#include "AccountStorage.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include "NotificatorMacro.h"
#include <QIcon>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMenu>
#include <QFileDialog>

DragDropTextEdit::DragDropTextEdit(QWidget *parent)
  : QTextEdit(parent) {
  setAcceptDrops(true);

  mMenu = createStandardContextMenu();
  mMenu->addSeparator();
  QAction *mSelectEncCsvFile = mMenu->addAction(QIcon{":/edit/PARSE_ENCRYPT_CSV_FILE"}, tr("select encrypt CSV file"));
  connect(mSelectEncCsvFile, &QAction::triggered, this, &DragDropTextEdit::onSelectEncCsvFileToParse);
}

void DragDropTextEdit::dragEnterEvent(QDragEnterEvent *event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  const QMimeData *mimeData = event->mimeData();
  CHECK_NULLPTR_RETURN_VOID(mimeData);
  if (!isContainsOneFile(*mimeData)) {
    event->ignore();
    return;
  }
  event->acceptProposedAction();
}

void DragDropTextEdit::dropEvent(QDropEvent *event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  const QMimeData *mimeData = event->mimeData();
  CHECK_NULLPTR_RETURN_VOID(mimeData);
  if (!isContainsOneFile(*mimeData)) {
    event->ignore();
    return;
  }
  QString encryptCsvFilePath = mimeData->urls().front().toLocalFile();
  if (!ParseEncryptCsvFileContents(encryptCsvFilePath)) {
    event->ignore();
    return;
  }
  event->acceptProposedAction();
}

void DragDropTextEdit::contextMenuEvent(QContextMenuEvent *event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  CHECK_NULLPTR_RETURN_VOID(mMenu);
  mMenu->exec(event->globalPos());
  event->accept();
}

bool DragDropTextEdit::isContainsOneFile(const QMimeData &mimeData) {
  return mimeData.hasUrls() && mimeData.urls().size() == 1;
}
bool DragDropTextEdit::onSelectEncCsvFileToParse() {
  const QString encryptCsvFilePath = QFileDialog::getOpenFileName(this,
                                                                  "Select encrypt csv file",
                                                                  AccountStorage::GetFullEncCsvFilePath(),
                                                                  "CSV Files (*.csv)");
  return ParseEncryptCsvFileContents(encryptCsvFilePath);
}

bool DragDropTextEdit::ParseEncryptCsvFileContents(const QString encryptCsvFilePath) {
  QString plainCsvFileContent;
  if (!AccountStorage::ParseEncryptCsvFile(encryptCsvFilePath, plainCsvFileContent)) {
    setText(QString{"Parse encrypt csv file[%1] failed"}.arg(encryptCsvFilePath));
    return false;
  }
  setText(plainCsvFileContent);
  return true;
}

CSVInputDialog::CSVInputDialog(QWidget *parent) //
  : QDialog{parent}                             //
{
  textEdit = new (std::nothrow) DragDropTextEdit(this);
  CHECK_NULLPTR_RETURN_VOID(textEdit);
  textEdit->setPlaceholderText("Enter your text here or drag encrypt csv file here...");

  buttonBox = new (std::nothrow) QDialogButtonBox(this);
  CHECK_NULLPTR_RETURN_VOID(buttonBox);
  buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
  pOkBtn = buttonBox->button(QDialogButtonBox::Ok);
  pCancelBtn = buttonBox->button(QDialogButtonBox::Cancel);
  pHelpBtn = buttonBox->button(QDialogButtonBox::Help);

  QPalette pal = pOkBtn->palette();
  pal.setColor(QPalette::ButtonText, QColor(30, 144, 255));
  pOkBtn->setPalette(pal);
  onContentsChanged();

  mainLayout = new (std::nothrow) QVBoxLayout(this);
  CHECK_NULLPTR_RETURN_VOID(mainLayout);
  mainLayout->addWidget(textEdit);
  mainLayout->addWidget(buttonBox);

  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(buttonBox, &QDialogButtonBox::helpRequested, this, &CSVInputDialog::onHelpRequest);
  connect(textEdit->document(), &QTextDocument::contentsChanged, this, &CSVInputDialog::onContentsChanged);

  setFont(StyleSheet::TEXT_EDIT_FONT);
  setWindowIcon(QIcon(":/edit/LOAD_FROM_INPUT"));
  setWindowTitle("Get record(s) from CSV input");
}

void CSVInputDialog::raise() {
  QDialog::raise();
  setWindowTitle("Get record(s) from CSV input");
  pOkBtn->setEnabled(false);
}

void CSVInputDialog::onHelpRequest() {
  CHECK_NULLPTR_RETURN_VOID(textEdit);
  const QString &text = textEdit->toPlainText();
  int nonEmptyLineCount{0};
  tempAccounts = AccountStorage::GetAccountsFromPlainString(text, &nonEmptyLineCount);

  if (nonEmptyLineCount != tempAccounts.size()) {
    QString msg;
    msg.reserve(40);
    msg += QString::number(tempAccounts.size());
    msg += "/";
    msg += QString::number(nonEmptyLineCount);
    msg += " line(s) are ok. Others need fixed at first.";
    LOG_ERR_NP("Some unexpected line(s) find", qPrintable(msg));
    setWindowTitle(msg);
    onContentsChanged();
    return;
  }
  const QString msg{QString{"%1/%2 line(s) are ok. no unexpected line(s) find"}.arg(tempAccounts.size()).arg(nonEmptyLineCount)};
  LOG_OK_NP("Parse ok", qPrintable(msg));
  setWindowTitle(msg);
  onCheckPassed();
}

void CSVInputDialog::onContentsChanged() {
  if (!pOkBtn->isEnabled()) {
    return; // already met requirement. skip
  }
  pOkBtn->setEnabled(false);
  pOkBtn->setToolTip("Click help at first");
  pOkBtn->setStyleSheet("");
}

void CSVInputDialog::onCheckPassed() {
  pOkBtn->setEnabled(true);
  pOkBtn->setToolTip("");
  pOkBtn->setStyleSheet(StyleSheet::SUBMIT_BTN_STYLE);
}
