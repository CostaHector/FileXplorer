#include "CSVInputDialog.h"
#include <QIcon>
#include <QMessageBox>
#include "AccountStorage.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include "NotificatorMacro.h"

CSVInputDialog::CSVInputDialog(QWidget* parent)  //
    : QDialog{parent}                            //
{
  textEdit = new (std::nothrow) QTextEdit(this);
  CHECK_NULLPTR_RETURN_VOID(textEdit);
  textEdit->setPlaceholderText("Enter your text here...");

  buttonBox = new (std::nothrow) QDialogButtonBox(this);
  CHECK_NULLPTR_RETURN_VOID(buttonBox);
  buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
  pOkBtn = buttonBox->button(QDialogButtonBox::Ok);
  pCancelBtn = buttonBox->button(QDialogButtonBox::Cancel);
  pHelpBtn = buttonBox->button(QDialogButtonBox::Help);

  QPalette pal = pOkBtn->palette();
  pal.setColor(QPalette::ButtonText, QColor(30, 144, 255));
  pOkBtn->setPalette(pal);
  pOkBtn->setEnabled(false);
  pOkBtn->setStyleSheet(StyleSheet::SUBMIT_BTN_STYLE);

  mainLayout = new (std::nothrow) QVBoxLayout(this);
  CHECK_NULLPTR_RETURN_VOID(mainLayout);
  mainLayout->addWidget(textEdit);
  mainLayout->addWidget(buttonBox);

  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(buttonBox, &QDialogButtonBox::helpRequested, this, &CSVInputDialog::onHelpRequest);

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
  const QString& text = textEdit->toPlainText();
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
    pOkBtn->setEnabled(false);
    return;
  }
  const QString msg{QString{"%1/%2 line(s) are ok. no unexpected line(s) find"}.arg(tempAccounts.size()).arg(nonEmptyLineCount)};
  LOG_OK_NP("Parse ok", qPrintable(msg));
  setWindowTitle(msg);
  pOkBtn->setEnabled(true);
}
