#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "AccountStorage.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class AccountStorageTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
  const QString mEncCsvFileName{"test_accounts.enc.csv"};
  const QString mPlainCsvFileName{"accounts_export.csv"};
  const QString mAES_KEY{"0123456789012345"};
 private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
  }

  void init() {  //
    GlobalMockObject::reset();
  }
  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void main_functions_ok() {
    // 允许默认构造
    AccountStorage temp;
    QVERIFY(!temp.IsValid());

    // 1. 允许注册
    QVERIFY(!mTDir.exists(mEncCsvFileName));
    AccountStorage initAnEncFile{mTDir.itemPath(mEncCsvFileName), mAES_KEY};
    const QList<AccountInfo> emptyListBackUp = initAnEncFile.mAccounts;
    QVERIFY(emptyListBackUp.isEmpty());

    QVERIFY(initAnEncFile.IsValid());
    QVERIFY(initAnEncFile.empty());
    QCOMPARE(initAnEncFile.size(), 0);
    QCOMPARE(initAnEncFile.toPlainCsv(), "");
    QVERIFY(!initAnEncFile.markRowRangeDeleted(0, 20, true)); // out-of-range not crash
    QVERIFY(!initAnEncFile.clearRowRangeModified(0, 20)); // out-of-range not crash

    // 不真实写入文件, 检查转调函数入参就行, 转调函数的正确性由专门的测试用例验证
    MOCKER(AccountFileHelper::CommitChangesToEncryptedCsvFile).expects(exactly(2))
        .with(eq(initAnEncFile.mAccounts), eq(mTDir.itemPath(mEncCsvFileName)), eq(mAES_KEY)).will(returnValue(true));
    MOCKER(AccountFileHelper::ExportAccountsToPlainCsvFile).expects(exactly(2))
        .with(eq(initAnEncFile.mAccounts), eq(mTDir.itemPath(mPlainCsvFileName))).will(returnValue(true));
    QVERIFY(initAnEncFile.CommitChangesToEncryptedCsvFile());
    QVERIFY(initAnEncFile.ExportToPlainCsvFile());

    const AccountInfo emptyElement;
    const AccountInfo acc2{"game", "MineCraft", "DiamondTuber", "123", "..."};
    initAnEncFile.append(acc2);
    // acc
    QVERIFY(initAnEncFile.AppendNRows(2));
    // acc, Empty, Empty
    QVERIFY(initAnEncFile.InsertNRows(0, 2));
    // Empty, Empty, acc2, Empty, Empty
    const AccountInfo acc5{"game", "Stardew Valley", "BurningFire", "123", "..."};
    const AccountInfo acc6{"game", "Terraria", "DancingFire", "123", "......"};
    QList<AccountInfo> parseFromInput{acc5, acc6};
    initAnEncFile += parseFromInput;
    // Empty, Empty, acc2, Empty, Empty, acc5, acc6

    QVERIFY(!initAnEncFile.empty());
    QCOMPARE(initAnEncFile.size(), 7);
    QCOMPARE(initAnEncFile[0], emptyElement);
    QCOMPARE(initAnEncFile[1], emptyElement);
    QCOMPARE(initAnEncFile[2], acc2);
    QCOMPARE(initAnEncFile[3], emptyElement);
    QCOMPARE(initAnEncFile[4], emptyElement);
    QCOMPARE(initAnEncFile[5], acc5);
    QCOMPARE(initAnEncFile[6], acc6);

    initAnEncFile[2].SetDeleted(true);
    initAnEncFile[5].SetDeleted(true);
    initAnEncFile[6].SetDeleted(true);

    initAnEncFile[2].SetModified(true);
    initAnEncFile[5].SetModified(true);
    initAnEncFile[6].SetModified(true);

    QVERIFY(!initAnEncFile.markRowRangeDeleted(-5, 20, false));                  // out-of-range
    QVERIFY(initAnEncFile.markRowRangeDeleted(0, initAnEncFile.size(), true));   // ok 全部标记为删除
    QVERIFY(initAnEncFile[0].IsDeleted());
    QVERIFY(initAnEncFile[1].IsDeleted());
    QVERIFY(initAnEncFile[2].IsDeleted());
    QVERIFY(initAnEncFile[3].IsDeleted());
    QVERIFY(initAnEncFile[4].IsDeleted());
    QVERIFY(initAnEncFile[5].IsDeleted());
    QVERIFY(initAnEncFile[6].IsDeleted());
    QVERIFY(initAnEncFile.markRowRangeDeleted(0, initAnEncFile.size(), false));  // 全部去除删除标记
    QVERIFY(!initAnEncFile[0].IsDeleted());
    QVERIFY(!initAnEncFile[1].IsDeleted());
    QVERIFY(!initAnEncFile[2].IsDeleted());
    QVERIFY(!initAnEncFile[3].IsDeleted());
    QVERIFY(!initAnEncFile[4].IsDeleted());
    QVERIFY(!initAnEncFile[5].IsDeleted());
    QVERIFY(!initAnEncFile[6].IsDeleted());

    QVERIFY(!initAnEncFile.clearRowRangeModified(-5, 20));                    // out-of-range
    QVERIFY(initAnEncFile.clearRowRangeModified(0, initAnEncFile.size()));    // ok
    QVERIFY(!initAnEncFile[2].IsModified());
    QVERIFY(!initAnEncFile[5].IsModified());
    QVERIFY(!initAnEncFile[6].IsModified());

    // 解析失败, 拒绝写入
    initAnEncFile.SetEncFileFullPath("Inexist/Path/To/accounts.enc.csv");
    QVERIFY(!initAnEncFile.IsValid());
    QVERIFY(!initAnEncFile.CommitChangesToEncryptedCsvFile());
    QVERIFY(!initAnEncFile.ExportToPlainCsvFile());

    // 修改回正确的文件
    initAnEncFile.SetEncFileFullPath(mTDir.itemPath(mEncCsvFileName));
    initAnEncFile.mAccounts = emptyListBackUp; // 恢复列表, 因为MOCKER中的with检查入参时用的是值拷贝,
    QVERIFY(initAnEncFile.IsValid());
    QVERIFY(initAnEncFile.CommitChangesToEncryptedCsvFile());
    QVERIFY(initAnEncFile.ExportToPlainCsvFile());

    initAnEncFile.SetAesKey("AnyAesIsAccepted"); // 现在文件还是空的, 可以修改AesKey
    QVERIFY(initAnEncFile.IsValid());
  }
};

#include "AccountStorageTest.moc"
REGISTER_TEST(AccountStorageTest, false)
