#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "LoginQryWidget.h"
#include "SimpleAES.h"
#include "EndToExposePrivateMember.h"

#include "Configuration.h"

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

private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid()); //
    QVERIFY(!tDir.exists("accounts_test.csv"));
    AccountStorageMock::GetFullEncCsvFilePathMock() = mEncCsvFilePath;
    LoginQryWidgetMock::clear();
    Configuration().clear(); //
  }

  void cleanupTestCase() {
    LoginQryWidgetMock::clear();
    Configuration().clear(); //
  }

  void init() { //
    GlobalMockObject::reset();
  }

  void cleanup() { //
    GlobalMockObject::verify();
  }

  void firstTime_registerNeeded() {
    QVERIFY(!tDir.exists("accounts_test.csv"));
    QVERIFY(AccountStorage::IsAccountCSVFileInexistOrEmpty());

    LoginQryWidget widget;
    QVERIFY(widget.mLoginWid != nullptr);
    QVERIFY(widget.mRegisterWid != nullptr);
    QVERIFY(!widget.mLoginWid->isEnabled());   // disable login
    QVERIFY(widget.mRegisterWid->isEnabled()); // enable register
    QVERIFY(widget.mLoginRegisterTab != nullptr);
    QVERIFY(widget.mLoginRegisterStk != nullptr);

    // 4. 模拟注册过程
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

    keyInput->setText("MySecureKey123");
    confirmInput->setText("mismatch key");
    emit buttonBox->button(QDialogButtonBox::Ok)->clicked();
    QVERIFY(widget.result() != QDialog::Accepted);
    QCOMPARE(regSpy.count(), 0);

    keyInput->setText("mismatch key");
    confirmInput->setText("MySecureKey123");
    emit buttonBox->button(QDialogButtonBox::Ok)->clicked();
    QVERIFY(widget.result() != QDialog::Accepted);
    QCOMPARE(regSpy.count(), 0);

    keyInput->setText("MySecureKey123");
    confirmInput->setText("MySecureKey123");
    emit buttonBox->button(QDialogButtonBox::Ok)->clicked();
    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(regSpy.count(), 1);

    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(widget.getAESKey(), "MySecureKey123");
  }

  void secondTime_autoLogin() {
    const QString testKey{"AutoLoginKey456"};
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

    // 模拟读取密钥成功
    MOCKER(CredUtilHelper::readPassword).expects(exactly(1)).will(returnValue(testKey));

    Configuration().setValue("REMEMBER_KEY", Qt::Checked);
    Configuration().setValue("LOG_IN_AUTOMATICALLY", Qt::Checked);

    LoginQryWidget widget;
    LoginWid* loginPage = widget.mLoginWid;
    QVERIFY(loginPage != nullptr);
    QVERIFY(loginPage->isEnabled()); // login enabled
    QVERIFY(loginPage->inputKeyLe != nullptr);
    QCOMPARE(loginPage->inputKeyLe->text(), testKey); // 加密读取到输入框ok

    QVERIFY(widget.mRegisterWid != nullptr);
    QVERIFY(!widget.mRegisterWid->isEnabled()); // register disabled
    QVERIFY(widget.mLoginRegisterTab != nullptr);
    QVERIFY(widget.mLoginRegisterStk != nullptr);

    //  模拟自动登录
    widget.mLoginRegisterTab->setCurrentIndex(LoginQryWidget::LOGIN);
    QCOMPARE(widget.mLoginRegisterStk->currentIndex(), LoginQryWidget::LOGIN);

    QSignalSpy timeoutSpy(loginPage, &LoginWid::timeoutAccepted);
    QSignalSpy acceptSpy(&widget, &LoginQryWidget::accepted);
    // user uncheck AutoLogin before timeout
    LoginQryWidgetMock::beforeTimeOutIsAutoLoginCheckedMock() = false;
    loginPage->AutoLoginTimeoutCallback();

    QVERIFY(widget.result() != QDialog::Accepted);
    QCOMPARE(timeoutSpy.count(), 0);
    QCOMPARE(acceptSpy.count(), 0);

    // user click ok directly before timeout, 点击登录 将会触发自动保存密钥
    MOCKER(CredUtilHelper::savePassword).expects(exactly(1)).with(eq(GetCredTargetName()), eq(testKey)).will(returnValue(true));
    emit widget.mDlgBtnBox->button(QDialogButtonBox::Ok)->clicked();
    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(timeoutSpy.count(), 0);
    QCOMPARE(acceptSpy.count(), 1);

    // timeout trigger accept, 超时自动登录, 不会触发自动保存密钥
    LoginQryWidgetMock::beforeTimeOutIsAutoLoginCheckedMock() = true; // user accept before timeout
    loginPage->AutoLoginTimeoutCallback();
    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(timeoutSpy.count(), 1);
    QCOMPARE(acceptSpy.count(), 2);

    QCOMPARE(widget.result(), QDialog::Accepted);
    QCOMPARE(widget.getAESKey(), testKey);
  }

  void firstTime_secondTime_manualClickLogin() {
    // 1. user input a new key
    const QString newTestKey = "AutoLoginKeyNew";
    QVERIFY(!AccountStorage::IsAccountCSVFileInexistOrEmpty());
    Configuration().setValue("REMEMBER_KEY", Qt::Unchecked);
    Configuration().setValue("LOG_IN_AUTOMATICALLY", Qt::Unchecked);

    MOCKER(CredUtilHelper::readPassword).expects(never()); // last time used key

    LoginQryWidget widget;
    QVERIFY(widget.mLoginWid->isEnabled());     // login enabled
    QVERIFY(!widget.mRegisterWid->isEnabled()); // register disabled

    QLineEdit* keyInput = widget.mLoginWid->inputKeyLe;
    keyInput->setText(newTestKey);
    QCOMPARE(keyInput->text(), newTestKey);

    QCheckBox* remeberKey = widget.mLoginWid->remeberKey;
    QCheckBox* autoLogin = widget.mLoginWid->autoLogin;
    { // initial state is from Configuration
      QVERIFY(remeberKey != nullptr);
      QVERIFY(autoLogin != nullptr);
      QCOMPARE(remeberKey->isChecked(), false);
      QCOMPARE(autoLogin->isChecked(), false);
    }

    // when check state changed. it will be write into system credUtil, can QSettings
    {
      MOCKER(CredUtilHelper::savePassword).expects(atLeast(1)).will(returnValue(true));
      remeberKey->setCheckState(Qt::Checked); // save 1st time
      emit remeberKey->stateChanged(Qt::CheckState::Checked);
      QCOMPARE(Configuration().value("REMEMBER_KEY").toInt(), Qt::Checked);

      MOCKER(CredUtilHelper::deletePassword).expects(atLeast(1)).will(returnValue(true));
      remeberKey->setCheckState(Qt::Unchecked);
      emit remeberKey->stateChanged(Qt::CheckState::Unchecked);
      QCOMPARE(Configuration().value("REMEMBER_KEY").toInt(), Qt::Unchecked);

      autoLogin->setCheckState(Qt::Checked); // save 2nd time
      emit autoLogin->stateChanged(Qt::CheckState::Checked);
      QCOMPARE(Configuration().value("LOG_IN_AUTOMATICALLY").toInt(), Qt::Checked);

      autoLogin->setCheckState(Qt::Unchecked);
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

    MOCKER(CredUtilHelper::readPassword).stubs().will(returnValue(QString(""))); // AES key cleared
    LoginQryWidget widget;
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
        .expects(once())                           //
        .with(eq(selectExistedFile), eq(destFile)) //
        .will(returnValue(true));
    QVERIFY(QueryWhenPasswordBookFileNotExist(nullptr));
  }
};

#include "LoginQryWidgetTest.moc"
REGISTER_TEST(LoginQryWidgetTest, false)
