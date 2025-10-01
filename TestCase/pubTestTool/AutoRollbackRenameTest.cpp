#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "AutoRollbackRename.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

class AutoRollbackRenameTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir tDir;
private slots:
  void initTestCase() { QVERIFY(tDir.IsValid()); }

  void init() { QVERIFY(tDir.ClearAll()); }

  // 基本文件重命名和回滚
  void test_FileRenameAndRollback() {
    const QString srcFile = "source.txt";
    QVERIFY(tDir.touch(srcFile));
    const QString dstFile = "destination.txt";

    {
      AutoRollbackRename renamer(tDir.path(), srcFile, dstFile);
      QVERIFY(renamer.Execute());
      QVERIFY(!tDir.fileExists(srcFile));
      QVERIFY(tDir.fileExists(dstFile));
    } // 作用域结束，触发回滚

    QVERIFY(tDir.fileExists(srcFile));
    QVERIFY(!tDir.fileExists(dstFile));
  }

  // 基本目录重命名和回滚
  void test_DirectoryRenameAndRollback() {
    const QString srcDir = "source_dir";
    QVERIFY(tDir.mkdir(srcDir));
    const QString dstDir = "destination_dir";

    {
      AutoRollbackRename renamer(tDir.path(), srcDir, dstDir);
      QVERIFY(renamer.Execute());
      QVERIFY(!tDir.dirExists(srcDir));
      QVERIFY(tDir.dirExists(dstDir));
    }

    QVERIFY(tDir.dirExists(srcDir));
    QVERIFY(!tDir.dirExists(dstDir));
  }

  // 源文件不存在的情况
  void test_SourceNotExist() {
    const QString nonExisting = "non_existing.txt";
    const QString dstFile = "destination.txt";

    AutoRollbackRename renamer(tDir.path(), nonExisting, dstFile);
    QVERIFY(!renamer.Execute()); // 应失败
    QVERIFY(!tDir.fileExists(nonExisting));
    QVERIFY(!tDir.fileExists(dstFile));
  }

  // 目标文件已存在的情况
  void test_DestinationAlreadyExists() {
    const QString srcFile = "source.txt";
    QVERIFY(tDir.touch(srcFile));
    const QString dstFile = "existing.txt";
    QVERIFY(tDir.touch(dstFile));

    AutoRollbackRename renamer(tDir.path(), srcFile, dstFile);
    QVERIFY(!renamer.Execute()); // 应失败
    QVERIFY(tDir.fileExists(srcFile));
    QVERIFY(tDir.fileExists(dstFile));
  }

  // 使用相对路径构造函数
  void test_RelativePathConstructor() {
    const QString srcFile = "source.txt";
    QVERIFY(tDir.touch(srcFile));
    const QString dstFile = "destination.txt";

    AutoRollbackRename renamer(tDir.path(), srcFile, dstFile);
    QVERIFY(renamer.Execute());
    QVERIFY(!tDir.fileExists(srcFile));
    QVERIFY(tDir.fileExists(dstFile));
  }

  // 使用绝对路径构造函数
  void test_AbsolutePathConstructor() {
    const QString srcFile = "source.txt";
    QVERIFY(tDir.touch(srcFile));
    const QString dstFile = "destination.txt";

    AutoRollbackRename renamer(tDir.itemPath(srcFile), tDir.itemPath(dstFile));
    QVERIFY(renamer.Execute());
    QVERIFY(!tDir.fileExists(srcFile));
    QVERIFY(tDir.fileExists(dstFile));
  }
};

#include "AutoRollbackRenameTest.moc"
REGISTER_TEST(AutoRollbackRenameTest, false)
