#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "CSVInputDialog.h"
#include "EndToExposePrivateMember.h"

#include <QPushButton>

class CSVInputDialogTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void DragDropTextEdit_ok() {
    DragDropTextEdit te;
    te.contextMenuEvent(nullptr);
    te.dragEnterEvent(nullptr);
    te.dropEvent(nullptr);

    QMimeData mimeData;
    QVERIFY(!te.isContainsOneFile(mimeData));

    const QUrl fileUrl{QUrl::fromLocalFile(__FILE__)};
    mimeData.setUrls(QList<QUrl>{fileUrl});
    QVERIFY(te.isContainsOneFile(mimeData));

    mimeData.setUrls(QList<QUrl>{fileUrl, fileUrl});
    QVERIFY(!te.isContainsOneFile(mimeData));
  }

  void test_initialization() {
    CSVInputDialog dialog;
    QVERIFY(dialog.textEdit != nullptr);
    QVERIFY(dialog.buttonBox != nullptr);
    QVERIFY(dialog.mainLayout != nullptr);

    QCOMPARE(dialog.textEdit->placeholderText(), "Enter your text here or drag encrypt csv file here...");

    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Ok) != nullptr);
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Cancel) != nullptr);
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Help) != nullptr);

    QVERIFY(!dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());

    QCOMPARE(dialog.mainLayout->count(), 2);
    QCOMPARE(dialog.mainLayout->itemAt(0)->widget(), dialog.textEdit);
    QCOMPARE(dialog.mainLayout->itemAt(1)->widget(), dialog.buttonBox);

    QCOMPARE(dialog.windowTitle(), "Get record(s) from CSV input");
    QVERIFY(!dialog.windowIcon().isNull());
    dialog.sizeHint();
  }

  void test_raise_method() {
    CSVInputDialog dialog;
    dialog.setWindowTitle("Modified Title");
    dialog.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

    dialog.raise();

    QCOMPARE(dialog.windowTitle(), "Get record(s) from CSV input");
    QVERIFY(!dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_onHelpRequest_empty_input() {
    CSVInputDialog dialog;
    dialog.textEdit->setText("");
    dialog.onHelpRequest();

    QVERIFY(dialog.tempAccounts.isEmpty());
    QCOMPARE(dialog.windowTitle(), "0/0 line(s) are ok. no unexpected line(s) find");
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_onHelpRequest_valid_input() {
    QString validCSV =
        "Email,Personal,user@example.com,pass123,Additional info\n"
        "Social Media,Facebook,fb_user,fb_pass,Notes with\\, comma\\nand newline\n"
        "Bank,Savings,bank_user,bank_pass,";
    CSVInputDialog dialog;
    dialog.textEdit->setText(validCSV);
    dialog.onHelpRequest();

    QCOMPARE(dialog.tempAccounts.size(), 3);
    QCOMPARE(dialog.tempAccounts[0].typeStr, "Email");
    QCOMPARE(dialog.tempAccounts[1].accountStr, "fb_user");
    QCOMPARE(dialog.tempAccounts[2].othersStr, "");

    QCOMPARE(dialog.windowTitle(), "3/3 line(s) are ok. no unexpected line(s) find");
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_onHelpRequest_partial_valid_input() {
    CSVInputDialog dialog;
    QString partialCSV =
        "Email,Personal,user@example.com,pass123,Additional info\n"
        "Invalid Line\n"
        "Bank,Savings,bank_user,bank_pass,";
    dialog.textEdit->setText(partialCSV);
    dialog.onHelpRequest();

    QCOMPARE(dialog.tempAccounts.size(), 2);
    QCOMPARE(dialog.tempAccounts[0].typeStr, "Email");
    QCOMPARE(dialog.tempAccounts[1].typeStr, "Bank");

    QCOMPARE(dialog.windowTitle(), "2/3 line(s) are ok. Others need fixed at first.");
    QVERIFY(!dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_onHelpRequest_invalid_input() {
    QString invalidCSV =
        "Invalid Line 1\n"
        "Invalid Line 2\n"
        "Invalid Line 3";
    CSVInputDialog dialog;
    dialog.textEdit->setText(invalidCSV);
    dialog.onHelpRequest();

    QVERIFY(dialog.tempAccounts.isEmpty());

    QCOMPARE(dialog.windowTitle(), "0/3 line(s) are ok. Others need fixed at first.");
    QVERIFY(!dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_button_signals() {
    CSVInputDialog dialog;

    emit dialog.buttonBox->accepted();
    emit dialog.buttonBox->rejected();
    emit dialog.buttonBox->helpRequested();
  }
};

#include "CSVInputDialogTest.moc"
REGISTER_TEST(CSVInputDialogTest, false)
