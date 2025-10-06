#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "MemoryKey.h"

#include "BeginToExposePrivateMember.h"
#include "LoginQryWidget.h"
#include "SimpleAES.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"
#include "AccountStorage.h"
#include "CredentialUtil.h"
#include <QPushButton>

class LoginQryWidgetTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  QString mEncCsvFilePath{tDir.itemPath("accounts_test.csv")};

  void simulateRegister(LoginQryWidget& widget, const QString& key) {
    // 切换到注册页
    QVERIFY(widget.mRegisterWid != nullptr);
    QWidget* registerPage = widget.mRegisterWid;

    widget.mLoginRegisterTab->setCurrentIndex(LoginQryWidget::REGISTER);
    QCOMPARE(widget.mLoginRegisterStkLo->currentIndex(), LoginQryWidget::REGISTER);

    QLineEdit* keyInput = registerPage->findChild<QLineEdit*>();
    QVERIFY(keyInput != nullptr);
    QLineEdit* confirmInput = registerPage->findChildren<QLineEdit*>()[1];
    QVERIFY(confirmInput != nullptr);
    QDialogButtonBox* buttonBox = registerPage->findChild<QDialogButtonBox*>();
    QVERIFY(buttonBox != nullptr);

    keyInput->setText(key);
    confirmInput->setText("mismatch key");
    emit buttonBox->button(QDialogButtonBox::Ok)->clicked();
    QVERIFY(widget.result() != QDialog::Accepted);

    keyInput->setText("mismatch key");
    confirmInput->setText(key);
    emit buttonBox->button(QDialogButtonBox::Ok)->clicked();
    QVERIFY(widget.result() != QDialog::Accepted);

    keyInput->setText(key);
    confirmInput->setText(key);
    emit buttonBox->button(QDialogButtonBox::Ok)->clicked();
    QCOMPARE(widget.result(), QDialog::Accepted);
  }

  void simulateAutoLogin(LoginQryWidget& widget, const QString& key) {
    // actually the key here will not get checked to see if it is correct
    // the check if key correct task is delivered to SimpleAES decrpyt.
    QVERIFY(widget.mLoginWid != nullptr);
    QWidget* loginPage = widget.mLoginWid;

    // 确保在登录页
    widget.mLoginRegisterTab->setCurrentIndex(LoginQryWidget::LOGIN);
    QCOMPARE(widget.mLoginRegisterStkLo->currentIndex(), LoginQryWidget::LOGIN);

    // 获取登录页控件
    QLineEdit* keyInput = loginPage->findChild<QLineEdit*>();
    QVERIFY(keyInput != nullptr);
    QDialogButtonBox* buttonBox = loginPage->findChild<QDialogButtonBox*>();
    QVERIFY(buttonBox != nullptr);

    QVERIFY(widget.mAutoLoginTimeoutFunc != nullptr);

    keyInput->setText(key);

    // user uncheck AutoLogin before timeout
    LoginQryWidgetMock::beforeTimeOutIsAutoLoginCheckedMock() = false;
    widget.mAutoLoginTimeoutFunc();
    QVERIFY(widget.result() != QDialog::Accepted);

    // user click ok directly before timeout
    emit buttonBox->button(QDialogButtonBox::Ok)->clicked();
    QCOMPARE(widget.result(), QDialog::Accepted);

    // user checked AutoLogin before timeout
    LoginQryWidgetMock::beforeTimeOutIsAutoLoginCheckedMock() = true;  // user accept before timeout
    widget.mAutoLoginTimeoutFunc();
    QCOMPARE(widget.result(), QDialog::Accepted);
  }

 private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());  //
    QVERIFY(!tDir.exists("accounts_test.csv"));
    AccountStorageMock::GetFullEncCsvFilePathMock() = mEncCsvFilePath;
    LoginQryWidgetMock::clear();
    Configuration().clear();  //
  }

  void cleanupTestCase() {
    LoginQryWidgetMock::clear();
    Configuration().clear();  //
  }

  void firstTime_registerNeeded() {
    QVERIFY(!tDir.exists("accounts_test.csv"));
    QVERIFY(AccountStorage::IsAccountCSVFileInexistOrEmpty());

    LoginQryWidget widget;

    // 3. 验证初始页面状态
    QVERIFY(widget.mLoginWid != nullptr);
    QVERIFY(widget.mRegisterWid != nullptr);
    QVERIFY(!widget.mLoginWid->isEnabled());    // disable login
    QVERIFY(widget.mRegisterWid->isEnabled());  // enable register

    // 4. 模拟注册过程
    const QString testKey = "MySecureKey123";
    simulateRegister(widget, testKey);

    // 5. 验证结果
    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(widget.getAESKey(), testKey);
  }

  void firstTime_secondTime_autoLogin() {
    // 1. 后面必须用此密钥才能登陆
    const QString testKey = "AutoLoginKey456";
    SimpleAES::setKey(testKey);
    AccountStorage storage;
    storage.mAccounts = {
        {"Email", "Personal", "user@example.com", "pass123", "Additional info"},         //
        {"Social", "Facebook", "fb_user", "fb_pass", "Notes with, comma\nand newline"},  //
        {"Bank", "Savings", "bank_user", "bank_pass", ""}                                //
    };
    QVERIFY(storage.SaveAccounts(true));
    QVERIFY(tDir.exists("accounts_test.csv"));
    QVERIFY(!AccountStorage::IsAccountCSVFileInexistOrEmpty());

    // 2. 配置自动登录
    Configuration().setValue("REMEMBER_KEY", Qt::Checked);
    Configuration().setValue("LOG_IN_AUTOMATICALLY", Qt::Checked);

    // 3. 保存密钥到凭证管理器
    const CredentialUtil& credUtil = CredentialUtil::GetInst();
    QVERIFY(credUtil.savePassword("PASSWORD_MANAGER_AES_KEY", testKey));
    QCOMPARE(credUtil.readPassword("PASSWORD_MANAGER_AES_KEY"), testKey);

    // 4. 创建登录窗口
    LoginQryWidget widget;

    // 5. 验证初始页面状态
    QVERIFY(widget.mLoginWid->isEnabled());      // login enabled
    QVERIFY(!widget.mRegisterWid->isEnabled());  // register disabled
    QVERIFY(widget.mAutoLoginTimeoutFunc != nullptr);

    // 6. 验证自动填充密钥
    QLineEdit* keyInput = widget.mLoginWid->findChild<QLineEdit*>();
    QCOMPARE(keyInput->text(), testKey);

    // 7. 模拟自动登录
    simulateAutoLogin(widget, testKey);

    // 8. 验证结果
    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(widget.getAESKey(), testKey);
  }

  void firstTime_secondTime_manualClickLogin() {
    // last time used key
    const CredentialUtil& credUtil = CredentialUtil::GetInst();
    QCOMPARE(credUtil.readPassword("PASSWORD_MANAGER_AES_KEY"), "AutoLoginKey456");

    // 1. user input a new key
    const QString newTestKey = "AutoLoginKeyNew";
    QVERIFY(!AccountStorage::IsAccountCSVFileInexistOrEmpty());
    Configuration().setValue("REMEMBER_KEY", Qt::Unchecked);
    Configuration().setValue("LOG_IN_AUTOMATICALLY", Qt::Unchecked);

    LoginQryWidget widget;
    QVERIFY(widget.mLoginWid->isEnabled());           // login enabled
    QVERIFY(!widget.mRegisterWid->isEnabled());       // register disabled
    QCOMPARE(widget.mAutoLoginTimeoutFunc, nullptr);  // Callback Func must be a nullptr

    QLineEdit* keyInput = widget.mLoginWid->findChild<QLineEdit*>();
    keyInput->setText(newTestKey);

    QCheckBox* remeberKey = widget.mLoginWid->findChild<QCheckBox*>();
    QCheckBox* autoLogin = widget.mLoginWid->findChildren<QCheckBox*>()[1];
    {  // inital state is from Configuration
      QVERIFY(remeberKey != nullptr);
      QVERIFY(autoLogin != nullptr);
      QCOMPARE(remeberKey->isChecked(), false);
      QCOMPARE(autoLogin->isChecked(), false);
    }
    {  // when check state changed. it will be write into system credUtil, can QSettings
      QCOMPARE(credUtil.readPassword("PASSWORD_MANAGER_AES_KEY"), "AutoLoginKey456");
      remeberKey->setChecked(true);
      emit remeberKey->stateChanged(Qt::CheckState::Checked);
      QCOMPARE(credUtil.readPassword("PASSWORD_MANAGER_AES_KEY"), newTestKey);  // get updated
      QCOMPARE(Configuration().value("REMEMBER_KEY").toInt(), Qt::Checked);

      remeberKey->setChecked(false);
      emit remeberKey->stateChanged(Qt::CheckState::Unchecked);
      QCOMPARE(credUtil.readPassword("PASSWORD_MANAGER_AES_KEY"), "");  // get removed
      QCOMPARE(Configuration().value("REMEMBER_KEY").toInt(), Qt::Unchecked);

      autoLogin->setChecked(true);
      emit autoLogin->stateChanged(Qt::CheckState::Checked);
      QCOMPARE(Configuration().value("LOG_IN_AUTOMATICALLY").toInt(), Qt::Checked);

      autoLogin->setChecked(false);
      emit autoLogin->stateChanged(Qt::CheckState::Unchecked);
      QCOMPARE(Configuration().value("LOG_IN_AUTOMATICALLY").toInt(), Qt::Unchecked);
    }

    // 7. user input key and click ok directly
    QWidget* loginPage = widget.mLoginWid;
    QDialogButtonBox* buttonBox = loginPage->findChild<QDialogButtonBox*>();
    QVERIFY(buttonBox != nullptr);
    emit buttonBox->button(QDialogButtonBox::Cancel)->clicked();
    QVERIFY(widget.result() != QDialog::Accepted);

    emit buttonBox->button(QDialogButtonBox::Ok)->clicked();
    QCOMPARE(widget.result(), QDialog::Accepted);

    // 8. 验证结果
    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(widget.getAESKey(), newTestKey);
  }
};

#include "LoginQryWidgetTest.moc"
REGISTER_TEST(LoginQryWidgetTest, false)
