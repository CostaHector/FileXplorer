#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "MemoryKey.h"

#include "BeginToExposePrivateMember.h"
#include "LoginQryWidget.h"
#include "SimpleAES.h"
#include "EndToExposePrivateMember.h"
#include <openssl/err.h>
#include <openssl/evp.h>

#include "TDir.h"
#include "AccountStorage.h"
#include "CredentialUtil.h"
#include <QSignalSpy>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>

#include "UserInteractiveMock.h"
#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class LoginQryWidgetTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir tDir;
  QString mEncCsvFilePath{tDir.itemPath("accounts_test.csv")};
  QString backUpCredKey;
  const CredentialUtil& credUtil = CredentialUtil::GetInst();

  void simulateRegister(LoginQryWidget& widget, const QString& key) {
    widget.mLoginRegisterTab->setCurrentIndex(LoginQryWidget::REGISTER);
    QCOMPARE(widget.mLoginRegisterStk->currentIndex(), LoginQryWidget::REGISTER);

    RegisterWid* registerPage = widget.mRegisterWid;
    QVERIFY(registerPage != nullptr);

    QLineEdit* keyInput = registerPage->inputKeyLe;
    QVERIFY(keyInput != nullptr);
    QLineEdit* confirmInput = registerPage->inputKeyAgainLe;
    QVERIFY(confirmInput != nullptr);
    QDialogButtonBox* buttonBox = widget.mDlgBtnBox;
    QVERIFY(buttonBox != nullptr);

    QSignalSpy regSpy{registerPage, &RegisterWid::registerAccepted};

    keyInput->setText(key);
    confirmInput->setText("mismatch key");
    emit buttonBox->button(QDialogButtonBox::Ok)->clicked();
    QVERIFY(widget.result() != QDialog::Accepted);
    QCOMPARE(regSpy.count(), 0);

    keyInput->setText("mismatch key");
    confirmInput->setText(key);
    emit buttonBox->button(QDialogButtonBox::Ok)->clicked();
    QVERIFY(widget.result() != QDialog::Accepted);
    QCOMPARE(regSpy.count(), 0);

    keyInput->setText(key);
    confirmInput->setText(key);
    emit buttonBox->button(QDialogButtonBox::Ok)->clicked();
    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(regSpy.count(), 1);
  }

  void simulateAutoLogin(LoginQryWidget& widget, const QString& key) {
    widget.mLoginRegisterTab->setCurrentIndex(LoginQryWidget::LOGIN);
    QCOMPARE(widget.mLoginRegisterStk->currentIndex(), LoginQryWidget::LOGIN);
    LoginWid* loginPage = widget.mLoginWid;
    QVERIFY(loginPage != nullptr);

    loginPage->inputKeyLe->setText(key);

    QSignalSpy timeoutSpy(loginPage, &LoginWid::timeoutAccepted);
    QSignalSpy acceptSpy(&widget, &LoginQryWidget::accepted);
    // user uncheck AutoLogin before timeout
    LoginQryWidgetMock::beforeTimeOutIsAutoLoginCheckedMock() = false;
    loginPage->AutoLoginTimeoutCallback();

    QVERIFY(widget.result() != QDialog::Accepted);
    QCOMPARE(timeoutSpy.count(), 0);
    QCOMPARE(acceptSpy.count(), 0);

    // user click ok directly before timeout, 点击登录 将会触发自动保存密钥
#ifdef _WIN32
    QVERIFY(credUtil.deletePassword(GetCredTargetName()));
    QCOMPARE(credUtil.readPassword(GetCredTargetName()), "");
#endif
    emit widget.mDlgBtnBox->button(QDialogButtonBox::Ok)->clicked();
    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(timeoutSpy.count(), 0);
    QCOMPARE(acceptSpy.count(), 1);
#ifdef _WIN32
    QCOMPARE(credUtil.readPassword(GetCredTargetName()), key);
#endif

    // timeout trigger accept, 超时自动登录, 不会触发自动保存密钥
    LoginQryWidgetMock::beforeTimeOutIsAutoLoginCheckedMock() = true; // user accept before timeout
    loginPage->AutoLoginTimeoutCallback();
    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(timeoutSpy.count(), 1);
    QCOMPARE(acceptSpy.count(), 2);
  }

private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid()); //
    QVERIFY(!tDir.exists("accounts_test.csv"));
    AccountStorageMock::GetFullEncCsvFilePathMock() = mEncCsvFilePath;
    LoginQryWidgetMock::clear();
    Configuration().clear(); //

    backUpCredKey = credUtil.readPassword(GetCredTargetName());
  }

  void cleanupTestCase() {
    LoginQryWidgetMock::clear();
    Configuration().clear(); //

    credUtil.savePassword(GetCredTargetName(), backUpCredKey);
  }

  void init() { GlobalMockObject::reset(); }

  void cleanup() { GlobalMockObject::verify(); }
  void firstTime_registerNeeded() {
    QVERIFY(!tDir.exists("accounts_test.csv"));
    QVERIFY(AccountStorage::IsAccountCSVFileInexistOrEmpty());

    LoginQryWidget widget;

    // 3. 验证初始页面状态
    QVERIFY(widget.mLoginWid != nullptr);
    QVERIFY(widget.mRegisterWid != nullptr);
    QVERIFY(!widget.mLoginWid->isEnabled());   // disable login
    QVERIFY(widget.mRegisterWid->isEnabled()); // enable register

    // 4. 模拟注册过程
    const QString testKey = "MySecureKey123";
    simulateRegister(widget, testKey);

    // 5. 验证结果
    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(widget.getAESKey(), testKey);
  }

  void secondTime_autoLogin() {
    // 1. 后面必须用此密钥才能登陆
    const QString testKey = "AutoLoginKey456";
    SimpleAES::InitInst(testKey);
    AccountStorage storage;
    storage.mAccounts = {
        {"Email", "Personal", "user@example.com", "pass123", "Additional info"},        //
        {"Social", "Facebook", "fb_user", "fb_pass", "Notes with, comma\nand newline"}, //
        {"Bank", "Savings", "bank_user", "bank_pass", ""}                               //
    };
    QVERIFY(storage.SaveAccounts(true));
    QVERIFY(tDir.exists("accounts_test.csv"));
    QVERIFY(!AccountStorage::IsAccountCSVFileInexistOrEmpty());

    // 2. 配置自动登录
    Configuration().setValue("REMEMBER_KEY", Qt::Checked);
    Configuration().setValue("LOG_IN_AUTOMATICALLY", Qt::Checked);

    // 3. 保存密钥到凭证管理器
    QVERIFY(credUtil.savePassword(GetCredTargetName(), testKey));
    QCOMPARE(credUtil.readPassword(GetCredTargetName()), testKey);

    // 4. 创建登录窗口
    LoginQryWidget widget;

    // 5. 验证初始页面状态
    QVERIFY(widget.mLoginWid->isEnabled());     // login enabled
    QVERIFY(!widget.mRegisterWid->isEnabled()); // register disabled
    // 6. 验证自动填充密钥
    QLineEdit* keyInput = widget.mLoginWid->inputKeyLe;
    QCOMPARE(keyInput->text(), testKey);

    // 7. 模拟自动登录
    simulateAutoLogin(widget, testKey);

    // 8. 验证结果
    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(widget.getAESKey(), testKey);
  }

  void firstTime_secondTime_manualClickLogin() {
    // last time used key
    QCOMPARE(credUtil.readPassword(GetCredTargetName()), "AutoLoginKey456");

    // 1. user input a new key
    const QString newTestKey = "AutoLoginKeyNew";
    QVERIFY(!AccountStorage::IsAccountCSVFileInexistOrEmpty());
    Configuration().setValue("REMEMBER_KEY", Qt::Unchecked);
    Configuration().setValue("LOG_IN_AUTOMATICALLY", Qt::Unchecked);

    LoginQryWidget widget;
    QVERIFY(widget.mLoginWid->isEnabled());     // login enabled
    QVERIFY(!widget.mRegisterWid->isEnabled()); // register disabled

    QLineEdit* keyInput = widget.mLoginWid->inputKeyLe;
    keyInput->setText(newTestKey);
    QCOMPARE(keyInput->text(), newTestKey);

    QCheckBox* remeberKey = widget.mLoginWid->remeberKey;
    QCheckBox* autoLogin = widget.mLoginWid->autoLogin;
    { // inital state is from Configuration
      QVERIFY(remeberKey != nullptr);
      QVERIFY(autoLogin != nullptr);
      QCOMPARE(remeberKey->isChecked(), false);
      QCOMPARE(autoLogin->isChecked(), false);
    }
    { // when check state changed. it will be write into system credUtil, can QSettings
      QCOMPARE(credUtil.readPassword(GetCredTargetName()), "AutoLoginKey456");
      remeberKey->setChecked(true);
      emit remeberKey->stateChanged(Qt::CheckState::Checked);
      QCOMPARE(credUtil.readPassword(GetCredTargetName()), newTestKey); // get updated
      QCOMPARE(Configuration().value("REMEMBER_KEY").toInt(), Qt::Checked);

      remeberKey->setChecked(false);
      emit remeberKey->stateChanged(Qt::CheckState::Unchecked);
      QCOMPARE(credUtil.readPassword(GetCredTargetName()), ""); // get removed
      QCOMPARE(Configuration().value("REMEMBER_KEY").toInt(), Qt::Unchecked);

      autoLogin->setChecked(true);
      emit autoLogin->stateChanged(Qt::CheckState::Checked);
      QCOMPARE(Configuration().value("LOG_IN_AUTOMATICALLY").toInt(), Qt::Checked);

      autoLogin->setChecked(false);
      emit autoLogin->stateChanged(Qt::CheckState::Unchecked);
      QCOMPARE(Configuration().value("LOG_IN_AUTOMATICALLY").toInt(), Qt::Unchecked);
    }

    // 7. click cancel. Timer should stopped
    QDialogButtonBox* buttonBox = widget.mDlgBtnBox;
    QVERIFY(buttonBox != nullptr);
    if (!widget.mLoginWid->autoLoginTimer.isActive()) {
      widget.mLoginWid->autoLoginTimer.start();
    }
    QVERIFY(widget.mLoginWid->autoLoginTimer.isActive());
    emit buttonBox->button(QDialogButtonBox::Cancel)->clicked();
    QVERIFY(widget.result() != QDialog::Accepted);
    QVERIFY(!widget.mLoginWid->autoLoginTimer.isActive());

    // 8. click Ok. Timer should stopped, aes key should correct
    if (!widget.mLoginWid->autoLoginTimer.isActive()) {
      widget.mLoginWid->autoLoginTimer.start();
    }
    QVERIFY(widget.mLoginWid->autoLoginTimer.isActive());
    emit buttonBox->button(QDialogButtonBox::Ok)->clicked();
    QVERIFY(!widget.mLoginWid->autoLoginTimer.isActive());
    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(widget.getAESKey(), newTestKey);
  }

  void encFileDelete_WillClearConfigures_ok() {
    // precondition:
    QVERIFY(tDir.exists("accounts_test.csv"));
    QVERIFY(!AccountStorage::IsAccountCSVFileInexistOrEmpty());
    tDir.ClearAll();
    QVERIFY(!tDir.exists("accounts_test.csv"));
    QVERIFY(AccountStorage::IsAccountCSVFileInexistOrEmpty());
    Configuration().setValue("REMEMBER_KEY", Qt::Checked);
    Configuration().setValue("LOG_IN_AUTOMATICALLY", Qt::Checked);

    LoginQryWidget widget;
    QCOMPARE(credUtil.readPassword(GetCredTargetName()), "");                       // AES key cleared
    QCOMPARE(Configuration().value("REMEMBER_KEY").toInt(), Qt::Unchecked);         // unchecked
    QCOMPARE(Configuration().value("LOG_IN_AUTOMATICALLY").toInt(), Qt::Unchecked); // unchecked
    QCheckBox* remeberKey = widget.mLoginWid->remeberKey;
    QCheckBox* autoLogin = widget.mLoginWid->autoLogin;
    QVERIFY(!remeberKey->isChecked());
    QVERIFY(!autoLogin->isChecked());
  }

  void QueryWhenPasswordBookFileNotExist_ok() {
    using namespace UserInteractiveMock;
    const QString inExistedFile{"Inexist_File.csv"};
    const QString selectExistedFile{"exist.csv"};
    const QString destFile = AccountStorage::GetFullEncCsvFilePath();

    MOCKER((FILE_EXIST_TYPE) QFile::exists).stubs().will(invoke(invoke_exists));
    MOCKER((QUESTION_TYPE) QMessageBox::question)
        .stubs()                                            //
        .will(returnValue(QMessageBox::StandardButton::No)) //
        .then(returnValue(QMessageBox::StandardButton::Yes));
    // User Click No
    QVERIFY(QueryWhenPasswordBookFileNotExist(nullptr));

    MOCKER(QFileDialog::getOpenFileName)
        .stubs()                          //
        .will(returnValue(inExistedFile)) //
        .then(returnValue(selectExistedFile));
    existsSet() = QSet<QString>{selectExistedFile, destFile};
    // Yes, "Inexist_File.csv"(false: not exist)
    QVERIFY(!QueryWhenPasswordBookFileNotExist(nullptr));

    // Yes, "exist.csv"(exist), destinationFile(exists) already occupied
    QVERIFY(!QueryWhenPasswordBookFileNotExist(nullptr));

    existsSet() = QSet<QString>{selectExistedFile};
    // Yes, "exist.csv"(exist), destinationFile(not exist)
    MOCKER((FILE_COPY_TYPE) QFile::copy)           //
        .expects(once())                                   //
        .with(eq(selectExistedFile), eq(destFile)) //
        .will(returnValue(true));
    QVERIFY(QueryWhenPasswordBookFileNotExist(nullptr));
  }
};

#include "LoginQryWidgetTest.moc"
REGISTER_TEST(LoginQryWidgetTest, false)
