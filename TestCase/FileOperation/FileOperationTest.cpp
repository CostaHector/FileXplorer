#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
// #include "TestCase/pub/FileSystemTestSuite.h"
#include "TestCase/pub/FileSystemHelper.h"
#include "TestCase/pub/MyTestSuite.h"
#include "TestCase/pub/OnScopeExit.h"
#include "TestCase/PathRelatedTool.h"
#include "FileOperation/FileOperation.h"

using namespace FileOperatorType;

QString g_rootPath{TestCaseRootPath() + "/test/TestEnv_FileOperation"};
QDir g_rootDir{g_rootPath};

class FileOperationTest : public MyTestSuite {
  Q_OBJECT

 public:
  FileOperationTest() : MyTestSuite{false} {}
  FileSystemHelper m_rootHelper{g_rootPath};
 private slots:
  void test_file_remove_not_recoverable() {
    QVERIFY(CreateAFile(g_rootPath + "/a.txt", "contents in a.txt"));
    ON_SCOPE_EXIT {
      if (g_rootDir.exists("a.txt")) {
        QVERIFY(g_rootDir.remove("a.txt"));
      }
    };
    QVERIFY(g_rootDir.exists("a.txt"));

    FileOperatorType::RETURN_TYPE retEle = FileOperation::rmfile(g_rootPath, "a.txt");
    QCOMPARE(retEle.ret, ErrorCode::OK);

    QVERIFY(!g_rootDir.exists("a.txt"));
    QVERIFY(retEle.cmds.isEmpty());  // not_recoverable
  }

  void test_rmpath() {
    // protect_remove_parent
    // a
    // a/a1
    // a/a1.txt
    QVERIFY(g_rootDir.mkpath("protect_remove_parent"));
    QVERIFY(g_rootDir.mkpath("a/a1/"));
    QVERIFY(CreateAFile(g_rootPath + "/a/a1/a2.txt", "contents in a1.txt"));
    ON_SCOPE_EXIT {
      if (g_rootDir.exists("a")) {
        QVERIFY(QDir{g_rootPath + "/a"}.removeRecursively());
      }
      if (g_rootDir.exists("protect_remove_parent")) {
        QVERIFY(QDir{g_rootPath}.rmdir("protect_remove_parent"));
      }
    };
    // a/a1
    // cannot remove at all because there is "a2.txt" under "a/a1"
    const auto& rmpthFailed = FileOperation::rmpath(g_rootPath, "a/a1");
    QCOMPARE(rmpthFailed.ret, CANNOT_REMOVE_PATH);
    QVERIFY(g_rootDir.exists("a/a1/a2.txt"));

    // remove the file
    QVERIFY(g_rootDir.remove("a/a1/a2.txt"));
    // a
    // a/a1
    // can remove both
    // parent contains protect_remove_parent.txt, will not remove parent
    const auto& rmpthok = FileOperation::rmpath(g_rootPath, "a/a1");
    QCOMPARE(rmpthok.ret, OK);
    QVERIFY(!g_rootDir.exists("a"));

    auto aBatch = rmpthok.cmds;
    QVERIFY(!aBatch.isEmpty());

    BATCH_COMMAND_LIST_TYPE srcCommand;
    BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};

    const auto& rmpthRecOk = FileOperation::executer(reversedaBatch, srcCommand);
    QCOMPARE(rmpthRecOk.ret, OK);
    QVERIFY(g_rootDir.exists("a/a1"));
  }

  void test_rmdir() {
    // protect_remove_parent
    // a
    // a/a1
    // a/a1/a2.txt
    QVERIFY(g_rootDir.mkpath("protect_remove_parent"));
    QVERIFY(g_rootDir.mkpath("a/a1"));
    QVERIFY(CreateAFile(g_rootPath + "/a/a1/a2.txt", "contents in a2.txt"));
    ON_SCOPE_EXIT {
      if (g_rootDir.exists("a")) {
        QVERIFY(QDir{g_rootPath + "/a"}.removeRecursively());
      }
      if (g_rootDir.exists("protect_remove_parent")) {
        QVERIFY(QDir{g_rootPath}.rmdir("protect_remove_parent"));
      }
    };
    // a/a1
    // cannot remove at all because there is "a2.txt" under "a/a1"
    const auto& rmDirFailed = FileOperation::rmdir(g_rootPath, "a/a1");
    QCOMPARE(rmDirFailed.ret, CANNOT_REMOVE_DIR);
    QVERIFY(g_rootDir.exists("a/a1/a2.txt"));

    // remove the file
    QVERIFY(g_rootDir.remove("a/a1/a2.txt"));
    // a
    // a/a1
    // can remove both
    // parent contains protect_remove_parent.txt, will not remove parent
    const auto& rmDirOk = FileOperation::rmdir(g_rootPath, "a/a1");
    QCOMPARE(rmDirOk.ret, OK);
    QVERIFY(!g_rootDir.exists("a/a1"));
    QVERIFY(g_rootDir.exists("a"));

    auto aBatch = rmDirOk.cmds;
    QVERIFY(!aBatch.isEmpty());

    BATCH_COMMAND_LIST_TYPE srcCommand;
    BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};

    const auto& rmDirRecOk = FileOperation::executer(reversedaBatch, srcCommand);
    QCOMPARE(rmDirRecOk.ret, OK);
    QVERIFY(g_rootDir.exists("a/a1"));
  }

  void test_rmFolderForce() {
    // a
    // a/a1
    // a/a1.txt
    QVERIFY(g_rootDir.mkpath("a/a1"));
    QVERIFY(CreateAFile(g_rootPath + "/a/a1.txt", "contents in a1.txt"));
    ON_SCOPE_EXIT {
      if (g_rootDir.exists("a")) {
        QVERIFY(QDir{g_rootPath + "/a"}.removeRecursively());
      }
    };

    QVERIFY(g_rootDir.exists("a"));
    QVERIFY(g_rootDir.exists("a/a1"));
    QVERIFY(g_rootDir.exists("a/a1.txt"));
    FileOperatorType::RETURN_TYPE retEle =  //
        FileOperation::rmFolderForce(g_rootPath, "a");
    auto ret = retEle.ret;
    auto aBatch = retEle.cmds;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(!g_rootDir.exists("a/a1.txt"));
    QVERIFY(!g_rootDir.exists("a/a1"));
    QVERIFY(!g_rootDir.exists("a"));
    QVERIFY(aBatch.isEmpty());  // remove cannot be recover
  }

  void test_file_to_trashbin_and_undo_ok() {
    QVERIFY(CreateAFile(g_rootPath + "/a.txt", "contents in a.txt"));
    ON_SCOPE_EXIT {
      if (g_rootDir.exists("a.txt")) {
        QVERIFY(g_rootDir.remove("a.txt"));
      }
    };
    QVERIFY(g_rootDir.exists("a.txt"));
    FileOperatorType::RETURN_TYPE retEle =  //
        FileOperation::moveToTrash(g_rootPath, "a.txt");

    auto ret = retEle.ret;
    auto aBatch = retEle.cmds;

    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(!g_rootDir.exists("a.txt"));
    QVERIFY(!aBatch.isEmpty());

    BATCH_COMMAND_LIST_TYPE srcCommand;
    BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.ret;
    QVERIFY(recoverRet == 0);
    QVERIFY(g_rootDir.exists("a.txt"));
  }

  // cpfile
  void test_absolute_file_copy() {
    // a.txt
    // b
    // b/a.txt
    QVERIFY(CreateAFile(g_rootPath + "/a.txt", "contents in a.txt"));
    QVERIFY(g_rootDir.mkdir("b"));
    ON_SCOPE_EXIT {
      if (g_rootDir.exists("a.txt")) {
        QVERIFY(g_rootDir.remove("a.txt"));
      }
      if (g_rootDir.exists("b")) {
        QVERIFY(QDir{g_rootPath + "/b"}.removeRecursively());
      }
    };

    // QVERIFY(CreateAFile(g_rootPath + "/b/a.txt", "contents in b/a.txt"));

    QString existFile("a.txt");
    QVERIFY(g_rootDir.exists(existFile));
    QVERIFY(g_rootDir.exists("b"));
    QVERIFY(!g_rootDir.exists(QString("b/%1").arg(existFile)));

    FileOperatorType::RETURN_TYPE retEle =  //
        FileOperation::cpfile(g_rootPath, existFile, QString("%1/b").arg(g_rootPath));
    auto ret = retEle.ret;
    auto aBatch = retEle.cmds;

    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(g_rootDir.exists(QString("b/%1").arg(existFile)));

    QVERIFY(!aBatch.isEmpty());

    BATCH_COMMAND_LIST_TYPE srcCommand;
    BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};

    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.ret;

    QCOMPARE(recoverRet, 0);
    QVERIFY(g_rootDir.exists(existFile));
    QVERIFY(g_rootDir.exists("b"));
    QVERIFY(!g_rootDir.exists(QString("b/%1").arg(existFile)));
  }

  void test_relative_file_copy() {
    QVERIFY(g_rootDir.mkdir("a"));
    QVERIFY(CreateAFile(g_rootPath + "/a/a1.txt", "contents in a/a1.txt"));
    QVERIFY(g_rootDir.mkdir("b"));
    ON_SCOPE_EXIT {
      if (g_rootDir.exists("a")) {
        QVERIFY(QDir{g_rootPath + "/a"}.removeRecursively());
      }
      if (g_rootDir.exists("b")) {
        QVERIFY(QDir{g_rootPath + "/b"}.removeRecursively());
      }
    };

    QString relativeExistFile("a/a1.txt");
    QVERIFY(g_rootDir.exists(relativeExistFile));
    QVERIFY(g_rootDir.exists("b"));
    QVERIFY(!g_rootDir.exists(QString("b/%1").arg(relativeExistFile)));

    FileOperatorType::RETURN_TYPE retEle =                    //
        FileOperation::cpfile(g_rootPath, relativeExistFile,  //
                              QString("%1/b").arg(g_rootPath));
    auto aBatch = retEle.cmds;

    QCOMPARE(retEle.ret, ErrorCode::OK);
    QVERIFY(g_rootDir.exists(QString("b/%1").arg(relativeExistFile)));

    QVERIFY(!aBatch.isEmpty());

    BATCH_COMMAND_LIST_TYPE srcCommand;
    BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};

    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.ret;

    QCOMPARE(recoverRet, 0);
    QVERIFY(g_rootDir.exists(relativeExistFile));
    QVERIFY(!g_rootDir.exists("b"));  // "b" was removed by "RMPATH" in reversedaBatch
  }

  void test_inexist_file_copy() {
    QVERIFY(g_rootDir.mkdir("b"));
    ON_SCOPE_EXIT {
      if (g_rootDir.exists("b")) {
        QVERIFY(QDir{g_rootPath + "/b"}.removeRecursively());
      }
    };

    QString inexistFileName("an inexist file blablablabla.txt");
    QVERIFY(!g_rootDir.exists(inexistFileName));
    QVERIFY(g_rootDir.exists("b"));
    QVERIFY(!g_rootDir.exists(QString("b/%1").arg(inexistFileName)));

    FileOperatorType::RETURN_TYPE retEle =                  //
        FileOperation::cpfile(g_rootPath, inexistFileName,  //
                              QString("%1/b").arg(g_rootPath));
    auto ret = retEle.ret;
    auto aBatch = retEle.cmds;

    QCOMPARE(ret, ErrorCode::SRC_INEXIST);
    QVERIFY(aBatch.isEmpty());

    QVERIFY(!g_rootDir.exists(inexistFileName));
    QVERIFY(g_rootDir.exists("b"));
    QVERIFY(!g_rootDir.exists(QString("b/%1").arg(inexistFileName)));
  }

};
#include "FileOperationTest.moc"
FileOperationTest g_FileOperationTest;
