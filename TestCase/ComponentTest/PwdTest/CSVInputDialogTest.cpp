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
  void test_initialization() {
    // 创建对话框
    CSVInputDialog dialog;

    // 验证初始状态
    QVERIFY(dialog.textEdit != nullptr);
    QVERIFY(dialog.buttonBox != nullptr);
    QVERIFY(dialog.mainLayout != nullptr);

    // 验证文本编辑框
    QCOMPARE(dialog.textEdit->placeholderText(), "Enter your text here...");

    // 验证按钮
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Ok) != nullptr);
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Cancel) != nullptr);
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Help) != nullptr);

    // 验证OK按钮初始状态
    QVERIFY(!dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());

    // 验证布局
    QCOMPARE(dialog.mainLayout->count(), 2);
    QCOMPARE(dialog.mainLayout->itemAt(0)->widget(), dialog.textEdit);
    QCOMPARE(dialog.mainLayout->itemAt(1)->widget(), dialog.buttonBox);

    // 验证窗口属性
    QCOMPARE(dialog.windowTitle(), "Get record(s) from CSV input");
    QVERIFY(!dialog.windowIcon().isNull());
    dialog.sizeHint();
  }

  void test_raise_method() {
    CSVInputDialog dialog;

    // 修改状态
    dialog.setWindowTitle("Modified Title");
    dialog.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

    // 调用raise方法
    dialog.raise();

    // 验证状态重置
    QCOMPARE(dialog.windowTitle(), "Get record(s) from CSV input");
    QVERIFY(!dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_onHelpRequest_empty_input() {
    CSVInputDialog dialog;

    // 设置空输入
    dialog.textEdit->setText("");

    // 调用帮助请求
    dialog.onHelpRequest();

    // 验证结果
    QVERIFY(dialog.tempAccounts.isEmpty());
    QCOMPARE(dialog.windowTitle(), "0/0 line(s) are ok. no unexpected line(s) find");
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_onHelpRequest_valid_input() {
    CSVInputDialog dialog;

    // 设置有效CSV输入
    QString validCSV =
        "Email,Personal,user@example.com,pass123,Additional info\n"
        "Social Media,Facebook,fb_user,fb_pass,Notes with\\, comma\\nand newline\n"
        "Bank,Savings,bank_user,bank_pass,";
    dialog.textEdit->setText(validCSV);

    // 调用帮助请求
    dialog.onHelpRequest();

    // 验证结果
    QCOMPARE(dialog.tempAccounts.size(), 3);
    QCOMPARE(dialog.tempAccounts[0].typeStr, "Email");
    QCOMPARE(dialog.tempAccounts[1].accountStr, "fb_user");
    QCOMPARE(dialog.tempAccounts[2].othersStr, "");

    // 验证状态
    QCOMPARE(dialog.windowTitle(), "3/3 line(s) are ok. no unexpected line(s) find");
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_onHelpRequest_partial_valid_input() {
    CSVInputDialog dialog;

    // 设置部分有效CSV输入
    QString partialCSV =
        "Email,Personal,user@example.com,pass123,Additional info\n"
        "Invalid Line\n"
        "Bank,Savings,bank_user,bank_pass,";
    dialog.textEdit->setText(partialCSV);

    // 调用帮助请求
    dialog.onHelpRequest();

    // 验证结果
    QCOMPARE(dialog.tempAccounts.size(), 2);
    QCOMPARE(dialog.tempAccounts[0].typeStr, "Email");
    QCOMPARE(dialog.tempAccounts[1].typeStr, "Bank");

    // 验证状态
    QCOMPARE(dialog.windowTitle(), "2/3 line(s) are ok. Others need fixed at first.");
    QVERIFY(!dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_onHelpRequest_invalid_input() {
    CSVInputDialog dialog;

    // 设置无效CSV输入
    QString invalidCSV =
        "Invalid Line 1\n"
        "Invalid Line 2\n"
        "Invalid Line 3";
    dialog.textEdit->setText(invalidCSV);

    // 调用帮助请求
    dialog.onHelpRequest();

    // 验证结果
    QVERIFY(dialog.tempAccounts.isEmpty());

    // 验证状态
    QCOMPARE(dialog.windowTitle(), "0/3 line(s) are ok. Others need fixed at first.");
    QVERIFY(!dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_button_signals() {
    CSVInputDialog dialog;

    dialog.buttonBox->button(QDialogButtonBox::Ok)->click();
    dialog.buttonBox->button(QDialogButtonBox::Cancel)->click();
    dialog.buttonBox->button(QDialogButtonBox::Help)->click();
  }
};

#include "CSVInputDialogTest.moc"
REGISTER_TEST(CSVInputDialogTest, false)
