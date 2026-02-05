#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "AccountStorage.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"
#include "SimpleAES.h"
#include <openssl/err.h>
#include <openssl/evp.h>
// #include <openssl/provider.h>
#include <openssl/rand.h>

class AccountStorageTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  QString mEncCsvFilePath{tDir.itemPath("accounts_test.csv")};
  QString mPlainCsvFilePath{tDir.itemPath("exportedPlainAccounts_test.csv")};
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
    AccountStorageMock::GetFullPlainCsvFilePathMock() = mPlainCsvFilePath;
    QVERIFY(AccountStorage::IsAccountCSVFileInexistOrEmpty());

    // 初始化 AES 密钥
    SimpleAES::InitInst("TestKey1234567890");  // 16字符密钥
  }

  void cleanupTestCase() {
    // 清理 OpenSSL
    EVP_cleanup();
    ERR_free_strings();

    AccountStorageMock::clear();
  }

  void test_account_storage_operations() {
    // 1. 创建初始账户数据
    AccountStorage storage;
    QVector<AccountInfo> initialAccounts{
        {"Email", "Personal", "user@example.com", "pass123", "Additional info"},         //
        {"Social", "Facebook", "fb_user", "fb_pass", "Notes with, comma\nand newline"},  //
        {"Bank", "Savings", "bank_user", "bank_pass", ""}                                //
    };
    storage.mAccounts = initialAccounts;

    // 2. 验证初始状态
    QCOMPARE(storage.size(), 3);
    QCOMPARE(storage[0].typeStr, "Email");
    QCOMPARE(storage[1].accountStr, "fb_user");
    QCOMPARE(storage[2].othersStr, "");

    // 3. 测试保存和加载（加密）
    QVERIFY(storage.SaveAccounts(true));  // 加密保存
    QVERIFY(QFile::exists(mEncCsvFilePath));

    AccountStorage loadedStorage;
    QVERIFY(loadedStorage.LoadAccounts());
    QCOMPARE(loadedStorage.size(), 3);
    QCOMPARE(loadedStorage[0].typeStr, "Email");
    QCOMPARE(loadedStorage[1].accountStr, "fb_user");
    QCOMPARE(loadedStorage[2].othersStr, "");
    loadedStorage[-999];  // will not crash down

    const AccountStorage* pLoadedStorage = &loadedStorage;
    QCOMPARE((*pLoadedStorage)[0].typeStr, "Email");
    QCOMPARE((*pLoadedStorage)[1].accountStr, "fb_user");
    QCOMPARE((*pLoadedStorage)[2].othersStr, "");
    (*pLoadedStorage)[-999];  // will not crash down

    // 4. 测试保存和加载（明文）
    QVERIFY(storage.SaveAccounts(false));  // 明文保存
    QVERIFY(QFile::exists(mPlainCsvFilePath));

    // 读取明文文件内容
    QFile plainFile(mPlainCsvFilePath);
    QVERIFY(plainFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString plainContent = plainFile.readAll();
    plainFile.close();

    // 验证明文内容
    QVERIFY(plainContent.contains("Email,Personal,user@example.com,pass123,Additional info"));
    QVERIFY(plainContent.contains("Social,Facebook,fb_user,fb_pass,Notes with\\, comma\\nand newline"));
    QVERIFY(plainContent.contains("Bank,Savings,bank_user,bank_pass,"));

    // 5. 测试修改标志
    QVERIFY(!storage.IsDataDirty());
    storage.SetListModified();
    QVERIFY(storage.IsDataDirty());
    storage.AfterSave();
    QVERIFY(!storage.IsDataDirty());

    // 6. 测试账户修改标志
    storage[0].SetDetailModified();
    QVERIFY(storage.IsDataDirty());
    storage.ClearAllDetailModified();
    QVERIFY(!storage.IsDataDirty());
    QVERIFY(!storage[0].IsDetailModified());

    // 7. 测试添加和删除账户
    storage.AppendNRows(1);
    QCOMPARE(storage.size(), 4);
    QCOMPARE(storage[3].typeStr, "");  // 新账户为空

    std::set<int> rowsToRemove = {1, 3, 999};  // 删除第1和第3行, 999行为无效行
    int removedCount = storage.RemoveIndexes(rowsToRemove);
    QCOMPARE(removedCount, 2);
    QCOMPARE(storage.size(), 2);
    QCOMPARE(storage[0].typeStr, "Email");
    QCOMPARE(storage[1].typeStr, "Bank");

    // 8. 测试插入账户
    QVERIFY(!storage.InsertNRows(0, -999));
    QVERIFY(storage.InsertNRows(1, 2));  // 在第1行后插入2行
    QCOMPARE(storage.size(), 4);
    QCOMPARE(storage[0].typeStr, "Email");
    QCOMPARE(storage[1].typeStr, "");  // 新插入的空账户
    QCOMPARE(storage[2].typeStr, "");  // 新插入的空账户
    QCOMPARE(storage[3].typeStr, "Bank");

    // 9. 测试边界索引访问
    const AccountInfo& invalid1 = storage[-1];
    QVERIFY(invalid1.typeStr.isEmpty());

    const AccountInfo& invalid2 = storage[100];
    QVERIFY(invalid2.typeStr.isEmpty());

    // 10. GetExportCSVRecords
    QString exportContent = storage.GetExportCSVRecords();
    QVERIFY(exportContent.contains("Email,Personal,user@example.com,pass123,Additional info"));
    QVERIFY(exportContent.contains("Bank,Savings,bank_user,bank_pass,"));

    {
      // insert at -1 =>insert at front
      QVERIFY(storage.InsertNRows(-1, 1));
      QCOMPARE(storage.size(), 5);
      // insert at 999 => insert at back
      QVERIFY(storage.InsertNRows(999, 1));
      QCOMPARE(storage.size(), 6);
    }

    // 11. 测试从字符串加载账户
    int nonEmptyLines = 0;
    QVector<AccountInfo> accountsFromString = AccountStorage::GetAccountsFromPlainString(
        "Type1,Name1,Acc1,Pwd1,Other1\n"
        "Type2,Name2,Acc2,Pwd2,Other2",
        &nonEmptyLines);
    QCOMPARE(accountsFromString.size(), 2);
    QCOMPARE(nonEmptyLines, 2);
    QCOMPARE(accountsFromString[0].typeStr, "Type1");
    QCOMPARE(accountsFromString[1].accountStr, "Acc2");

    // 12. 测试无效CSV行处理
    QVector<AccountInfo> invalidAccounts = AccountStorage::GetAccountsFromPlainString(
        "InvalidLine\n"
        "Type,Name,Acc,Pwd,Other\n"
        "ShortLine",
        nullptr);
    QCOMPARE(invalidAccounts.size(), 1);  // 只有一行有效
    QCOMPARE(invalidAccounts[0].typeStr, "Type");
  }
};

#include "AccountStorageTest.moc"
REGISTER_TEST(AccountStorageTest, false)
