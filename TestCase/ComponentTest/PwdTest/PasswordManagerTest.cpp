#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "PasswordManager.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"
#include "SimpleAES.h"
#include "PwdTableEditActions.h"

class PasswordManagerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  QString mEncCsvFilePath{tDir.itemPath("accounts_test.csv")};

  void prepareTestData(AccountStorage& storage) {
    QVector<AccountInfo> initialAccounts{
        {"Email", "Henry", "Henry@example.com", "pass123", "Additional info"},                 //
        {"Social Media", "Facebook", "fb_user", "fb_pass", "Notes with, comma\nand newline"},  //
        {"Bank", "Chris", "bank_user_chris", "bank_pass", ""},                                 //
    };
    storage.mAccounts = initialAccounts;
  }

 private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());  //
    QVERIFY(!tDir.exists("accounts_test.csv"));
    QVERIFY(!tDir.exists("exportedPlainAccounts_test.csv"));
    AccountStorageMock::clear();

    // register
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    AccountStorageMock::GetFullEncCsvFilePathMock() = mEncCsvFilePath;
    QVERIFY(AccountStorage::IsAccountCSVFileInExistOrEmpty());

    // 初始化 AES 密钥
    SimpleAES::setKey("TestKey1234567890");  // 16字符密钥
    AccountTableViewMock::clear();
    Configuration().clear();
  }

  void cleanupTestCase() {
    // 清理 OpenSSL
    EVP_cleanup();
    ERR_free_strings();

    AccountStorageMock::clear();
    AccountTableViewMock::clear();
    Configuration().clear();
  }

  void firstTimeCreateTable_file_not_exist() {
    QVERIFY(!tDir.exists("accounts_test.csv"));
    auto& ins = GetTableEditActionsInst();

    PasswordManager pwdMgr;
    {
      QCOMPARE(pwdMgr.mAccountListView->mPwdModel->rowCount(), 0);  // no file no row
      // title correct
      QVERIFY(pwdMgr.windowTitle().contains(mEncCsvFilePath));
      pwdMgr.onSave();  // nothing changed, skip save, still no file norow
      // statusBar message correct
      QVERIFY(pwdMgr.mStatusBar->currentMessage().contains("SKIP", Qt::CaseInsensitive));
      QVERIFY(!tDir.exists("accounts_test.csv"));
      QCOMPARE(pwdMgr.mAccountListView->mPwdModel->rowCount(), 0);

      emit ins.OPEN_DIRECTORY->triggered();  // should not crash down
    }

    const QString partial2LineValidCSV{
        "Email,Personal,user@example.com,pass123,Chris Evans info\n"
        "Invalid Line\n"
        "Bank,Savings,bank_user,bank_pass,Henry Cavill info"};
    const QString expectPlainCsvContentShow{
        "Email,Personal,user@example.com,pass123,Chris Evans info\n"
        "Bank,Savings,bank_user,bank_pass,Henry Cavill info"};

    {
      QVERIFY(pwdMgr.mCsvInputDialog == nullptr);
      pwdMgr.onGetRecordsFromInput();
      QVERIFY(pwdMgr.mCsvInputDialog != nullptr);
      QVERIFY(pwdMgr.mCsvInputDialog->pOkBtn != nullptr);
      QVERIFY(pwdMgr.mCsvInputDialog->pHelpBtn != nullptr);
      QVERIFY(pwdMgr.mCsvInputDialog->pCancelBtn != nullptr);
      QVERIFY(pwdMgr.mCsvInputDialog->textEdit != nullptr);

      //  no contents at all. accept still no row
      pwdMgr.mCsvInputDialog->textEdit->setPlainText("");
      emit pwdMgr.mCsvInputDialog->pHelpBtn->clicked();
      emit pwdMgr.mCsvInputDialog->pOkBtn->clicked();
      QCOMPARE(pwdMgr.mAccountListView->mPwdModel->rowCount(), 0);  // still no row

      // 2 valid row contents, cancel still no row
      pwdMgr.mCsvInputDialog->textEdit->setPlainText(partial2LineValidCSV);
      emit pwdMgr.mCsvInputDialog->pHelpBtn->clicked();
      QCOMPARE(pwdMgr.mCsvInputDialog->tempAccounts.size(), 2);
      emit pwdMgr.mCsvInputDialog->pCancelBtn->clicked();
      QCOMPARE(pwdMgr.mAccountListView->mPwdModel->rowCount(), 0);  // still no row
      QCOMPARE(pwdMgr.mCsvInputDialog->tempAccounts.size(), 2);

      // 2 valid row contents, accept get 2 row
      pwdMgr.onGetRecordsFromInput();
      QCOMPARE(pwdMgr.mCsvInputDialog->tempAccounts.size(), 2);
      emit pwdMgr.mCsvInputDialog->pOkBtn->clicked();
      QCOMPARE(pwdMgr.mAccountListView->mPwdModel->rowCount(), 2);  // 2 row exist
    }

    {
      // saved ok, statusBar mesage correct,  exist file exist 2 row
      pwdMgr.onSave();
      QVERIFY(pwdMgr.mStatusBar->currentMessage().contains("OK", Qt::CaseInsensitive));
      QVERIFY(tDir.exists("accounts_test.csv"));
      QCOMPARE(pwdMgr.mAccountListView->mPwdModel->rowCount(), 2);  // 2 row exist
    }

    {  // now file exist
      emit ins.OPEN_DIRECTORY->triggered();
      QVERIFY(pwdMgr.openEncFileLocatedIn());
    }

    {
      // show plain csv contents ok
      QVERIFY(pwdMgr.mPlainCSVContentWid == nullptr);
      pwdMgr.ShowPlainCSVContents();
      QVERIFY(pwdMgr.mPlainCSVContentWid != nullptr);
      QCOMPARE(pwdMgr.mPlainCSVContentWid->toPlainText(), expectPlainCsvContentShow);

      pwdMgr.mPlainCSVContentWid->setPlainText("");  // clear it manually
      pwdMgr.ShowPlainCSVContents();                 // show correct contents ok
      QCOMPARE(pwdMgr.mPlainCSVContentWid->toPlainText(), expectPlainCsvContentShow);
    }

    {  // signal currentRowChanged connect correct
      QVERIFY(pwdMgr.mAccountDetailView->pAccount == nullptr);
      QCOMPARE(pwdMgr.mAccountListView->mPwdModel->rowCount(), 2);  // 2 row exist
      pwdMgr.mAccountListView->selectionModel()->clear();

      pwdMgr.mAccountListView->selectRow(0);
      const auto* const p1 = pwdMgr.mAccountDetailView->pAccount;
      QVERIFY(p1 != nullptr);

      pwdMgr.mAccountListView->selectRow(1);
      const auto* const p2 = pwdMgr.mAccountDetailView->pAccount;
      QVERIFY(p2 != nullptr);

      QVERIFY(p1 != p2);
    }

    { // filter ok
      QVERIFY(pwdMgr.mAccountListView->mPwdModel != nullptr);
      QVERIFY(pwdMgr.mAccountListView->mSortProxyModel != nullptr);
      QCOMPARE(pwdMgr.mAccountListView->mPwdModel->rowCount(), 2);
      QCOMPARE(pwdMgr.mAccountListView->mSortProxyModel->rowCount(), 2);

      pwdMgr.mSearchText->setText("Henry Cavill");
      emit pwdMgr.mSearchText->returnPressed();
      QCOMPARE(pwdMgr.mAccountListView->mPwdModel->rowCount(), 2);
      QCOMPARE(pwdMgr.mAccountListView->mSortProxyModel->rowCount(), 1);

      pwdMgr.mSearchText->setText("");
      emit pwdMgr.mSearchText->returnPressed();
      QCOMPARE(pwdMgr.mAccountListView->mPwdModel->rowCount(), 2);
      QCOMPARE(pwdMgr.mAccountListView->mSortProxyModel->rowCount(), 2);
    }

    pwdMgr.closeEvent(nullptr);
    QCloseEvent closeEve;
    pwdMgr.closeEvent(&closeEve);
    Configuration().contains("PASSWORD_TABLEVIEW_GEOMETRY");
    Configuration().contains("PASSWORD_TABLEVIEW_STATE");
    Configuration().contains("ACCOUNT_DETAIL_VIEW_GEOMETRY");
  }

  void secondTime_FromExistsEncrypedFile_directly() {
    QVERIFY(tDir.exists("accounts_test.csv"));

    PasswordManager pwdMgr;
    QCOMPARE(pwdMgr.mAccountListView->mPwdModel->rowCount(), 2);  // 2 rows once opened
    pwdMgr.onSave();                                              // no need save at all
    QVERIFY(pwdMgr.mStatusBar->currentMessage().contains("SKIP", Qt::CaseInsensitive));
    QCOMPARE(pwdMgr.mAccountListView->mPwdModel->rowCount(), 2);
  }
};

#include "PasswordManagerTest.moc"
REGISTER_TEST(PasswordManagerTest, false)
