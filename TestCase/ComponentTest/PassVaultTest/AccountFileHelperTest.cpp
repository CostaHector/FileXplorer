#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "AccountFileHelper.h"
#include "TDir.h"
#include "SimpleAES.h"
#include "FileTool.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

using namespace AccountFileHelper;

class AccountFileHelperTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
  const QList<AccountInfo> mAccList //
      {
       AccountInfo{"E-mail", "Gmail", "MichaelFassbender@gmail.com", "123456", ""},
       AccountInfo{"Game", "Minecraft", "MichaelFassbender", "654321", "Include:\nJava Edition\nBedrock Edition"},
       };
  const QString AES_KEY{"AES_KEY"};
  const QString REGISTER_KEY{"REGISTER_KEY"};
  const QString WRONG_AES_KEY{"WRONG_AES_KEY"};
private slots:
  void initTestCase() {
    QString plainCSV = GetExportCSVRecords(mAccList);
    QString encryptedCSV;
    QVERIFY(SimpleAES(AES_KEY).encrypt_GCM(plainCSV, encryptedCSV));
    QVERIFY(encryptedCSV.size() > 0);

    QVERIFY(mTDir.IsValid());
    QList<FsNodeEntry> nodes {
        {"FolderOccupied_accounts.enc.csv", true, ""},    // 非法(文件夹占用了本路径)
        {"EmptyFile_accounts.enc.csv", false, ""},        // 合法(空文件, 视为注册)
        {"PrepathExist", true, ""},                       // 合法(前置路径存在且自身不存在的文件, 视为注册)
        {"FileCorrupt_accounts.enc.csv", false, "BrokenFile"},  // 存在但是损坏
        {"FileComplete_accounts.enc.csv", false, encryptedCSV.toUtf8()}, // 存在且完整
    };
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());

    IconForType("game");
    IconForType("inexist");

    QVERIFY(GetDefaultEncFileFullPath().endsWith(".csv"));
  }

  void init() {  //
    GlobalMockObject::reset();
  }
  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void ValidateAndLoad_ParseEncryptCsvFile_ok() {
    const LoadResult invalidAndEmptyListResult{false, {}};
    const LoadResult validAndEmptyListResult{true, {}};
    const LoadResult validAnd2ElementResult{true, mAccList};

    // 不可为空
    QCOMPARE(ValidateAndLoad("", ""), invalidAndEmptyListResult);

    // 文件夹占用了本路径
    QCOMPARE(ValidateAndLoad(mTDir.itemPath("FolderOccupied_accounts.enc.csv"), AES_KEY), invalidAndEmptyListResult);

    // 空文件, 视为注册
    QCOMPARE(ValidateAndLoad(mTDir.itemPath("EmptyFile_accounts.enc.csv"), AES_KEY), validAndEmptyListResult);
    QCOMPARE(ValidateAndLoad(mTDir.itemPath("EmptyFile_accounts.enc.csv"), REGISTER_KEY), validAndEmptyListResult);
    QCOMPARE(ValidateAndLoad(mTDir.itemPath("EmptyFile_accounts.enc.csv"), WRONG_AES_KEY), validAndEmptyListResult);

    // 前置路径存在且自身不存在的文件, 视为注册
    QCOMPARE(ValidateAndLoad(mTDir.itemPath("PrepathExist/NotOcupied.enc.csv"), REGISTER_KEY), validAndEmptyListResult);
    // 前置路径不存在， touch不可以生成文件， 所以失败
    QCOMPARE(ValidateAndLoad(mTDir.itemPath("PrepathInexist/NotOcupied.enc.csv"), REGISTER_KEY), invalidAndEmptyListResult);

    // 存在但是损坏
    QCOMPARE(ValidateAndLoad(mTDir.itemPath("FileCorrupt_accounts.enc.csv"), AES_KEY), invalidAndEmptyListResult);
    // 存在但是密钥错误
    QCOMPARE(ValidateAndLoad(mTDir.itemPath("FileComplete_accounts.enc.csv"), WRONG_AES_KEY), invalidAndEmptyListResult);

    // 存在且密钥正确， 解析成功
    QCOMPARE(ValidateAndLoad(mTDir.itemPath("FileComplete_accounts.enc.csv"), AES_KEY), validAnd2ElementResult);
  }

  void GetAccountsFromPlainString_GetExportCSVRecords_ok() {
    QList<int> incompleteLines;

    // GetExportCSVRecords: list->text->list 没有被标记为删除的行时, 可逆且必定不会有不完整行
    {
      QCOMPARE(GetAccountsFromPlainString(GetExportCSVRecords(mAccList), incompleteLines), mAccList);
      QVERIFY(incompleteLines.isEmpty());
      incompleteLines.clear();
    }

    // GetExportCSVRecords 会丢弃已被标记为删除的行
    {
      QList<AccountInfo> accListWithDeletedMark;
      {
        AccountInfo accMarkedAsDeleted{"Shopping", "Temu", "TemuUser1", "123456", "..."}; // 此行在转list时被丢弃
        accMarkedAsDeleted.SetDeleted(true);

        accListWithDeletedMark += accMarkedAsDeleted;
        accListWithDeletedMark += mAccList;
        accListWithDeletedMark += accMarkedAsDeleted;
      }
      QCOMPARE(GetAccountsFromPlainString(GetExportCSVRecords(accListWithDeletedMark), incompleteLines), mAccList);
      QVERIFY(incompleteLines.isEmpty());
      incompleteLines.clear();
    }


    // GetAccountsFromPlainString: text->list->text 如果text中有非法行则不可逆， 会抛弃掉不完整行
    {
      const QString complete2Lines = GetExportCSVRecords(mAccList);
      QString incompleteLines_1_in_5; // 1 in 5 lines incomplete
      incompleteLines_1_in_5 += complete2Lines; // complete 0,1
      incompleteLines_1_in_5 += "\n";           // incomplete 2
      incompleteLines_1_in_5 += complete2Lines; // complete3,4
      QList<AccountInfo> expect4Elements;
      {
        expect4Elements += mAccList;
        expect4Elements += mAccList;
      }
      QList<AccountInfo> accList4Elements = GetAccountsFromPlainString(incompleteLines_1_in_5, incompleteLines);
      QCOMPARE(accList4Elements, expect4Elements);
      QCOMPARE(incompleteLines, (QList<int>{2}));
      incompleteLines.clear();
    }
  }

  void ExportAccountsToPlainCsvFile_CommitChangesToEncryptedCsvFile_ok() {
    QIODevice::OpenMode expectWriteMode{QIODevice::WriteOnly};
    {
      const QList<AccountInfo> emptyAccounts;
      QString emptyRecordsStr;
      QCOMPARE(GetExportCSVRecords(emptyAccounts), emptyRecordsStr); // 空列表对应空记录

      // FileTool::StringTextWriter, 导出两次, 首次导出失败, 第二次成功
      QString exportToPlainCSVFullPath{"ExportPlainFile.enc.csv"};
      MOCKER(FileTool::StringTextWriter).expects(exactly(2)).with(eq(exportToPlainCSVFullPath), eq(emptyRecordsStr), eq(expectWriteMode)).will(returnValue(false)).then(returnValue(true));
      QVERIFY(!ExportAccountsToPlainCsvFile(emptyAccounts, exportToPlainCSVFullPath));
      QVERIFY(ExportAccountsToPlainCsvFile(emptyAccounts, exportToPlainCSVFullPath));
    }

    {
      AccountInfo accMarkedAsDeleted{"Shopping", "Temu", "TemuUser1", "123456", "..."};
      accMarkedAsDeleted.SetDeleted(true);
      AccountInfo accMarkedAsModified{"Shopping", "Amazon", "AmazonUser1", "123456", "..."};
      accMarkedAsModified.SetModified(true);

      QList<AccountInfo> mixedAccList;
      mixedAccList += mAccList;
      mixedAccList += accMarkedAsDeleted;
      mixedAccList += accMarkedAsModified;
      QVERIFY(!mixedAccList[0].IsModified());
      QVERIFY(!mixedAccList[1].IsModified());
      QVERIFY(!mixedAccList[2].IsModified());
      QVERIFY(mixedAccList[3].IsModified()); // 修改标记
      QVERIFY(mixedAccList[2].IsDeleted()); // 删除标记

      const QString recordsStrOf3Elements = GetExportCSVRecords(mixedAccList);

      // FileTool::CreateABackupFile
      // FileTool::AtomicTextWriter
      // 总是会尝试备份, 备份失败也会继续保存流程； 原子化首次保存失败, 第二次成功
      QString exportToEncCSVFullPath{"ExportEncFile.enc.csv"};
      MOCKER(FileTool::CreateABackupFile).expects(exactly(2)).with(eq(exportToEncCSVFullPath)).will(returnValue(false)).then(returnValue(true));
      MOCKER(FileTool::AtomicTextWriter).expects(exactly(2)).with(eq(exportToEncCSVFullPath), any(), eq(expectWriteMode))
          .will(returnValue(false)).then(returnValue(true));

      QVERIFY(!CommitChangesToEncryptedCsvFile(mixedAccList, exportToEncCSVFullPath, REGISTER_KEY));
      QVERIFY(CommitChangesToEncryptedCsvFile(mixedAccList, exportToEncCSVFullPath, REGISTER_KEY));

      // 落盘时清除了 isModified标志，
      // 落盘时直接丢弃deleted的行， 不去删列表内的元素了， 给用户撤销删除的机会（撤销删除标记+落盘即可恢复）
      QCOMPARE(mixedAccList.size(), 4);
      QVERIFY(!mixedAccList[0].IsModified());
      QVERIFY(!mixedAccList[1].IsModified());
      QVERIFY(!mixedAccList[2].IsModified());
      QVERIFY(!mixedAccList[3].IsModified()); // 修改标记已清除
      QVERIFY(mixedAccList[2].IsDeleted());   // 删除标记为变
    }
  }
};

#include "AccountFileHelperTest.moc"
REGISTER_TEST(AccountFileHelperTest, false)
