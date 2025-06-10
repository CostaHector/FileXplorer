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
  FileOperationTest() : MyTestSuite{true} {}
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

  // link
  void test_link_a_file() {
    QVERIFY(CreateAFile(g_rootPath + "/a.txt", "contents in a.txt"));
    ON_SCOPE_EXIT {
      if (g_rootDir.exists("a.txt.lnk")) {
        QVERIFY(g_rootDir.remove("a.txt.lnk"));
      }
      if (g_rootDir.exists("a.txt")) {
        QVERIFY(g_rootDir.remove("a.txt"));
      }
    };

    QVERIFY2(g_rootDir.exists("a.txt"), "Precondition not required.");

    FileOperatorType::RETURN_TYPE retEle = FileOperation::link(g_rootPath, "a.txt", g_rootPath);
    auto aBatch = retEle.cmds;
    QCOMPARE(retEle.ret, ErrorCode::OK);

    QVERIFY(g_rootDir.exists("a.txt"));
    QVERIFY(g_rootDir.exists("a.txt.lnk"));
    QVERIFY(!aBatch.isEmpty());  // can recover

    BATCH_COMMAND_LIST_TYPE srcCommand;
    const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    QCOMPARE(exeRetEle.ret, ErrorCode::OK);
    QCOMPARE(exeRetEle.isRecoverable(), true);

    // lnk was removed, file linked to not removed
    QVERIFY(g_rootDir.exists("a.txt"));
    QVERIFY(!g_rootDir.exists("a.txt.lnk"));
  }

  void test_link_a_relative_file() {
    QVERIFY(g_rootDir.mkpath("a"));
    QVERIFY(CreateAFile(g_rootPath + "/a/a1.txt", "contents in a/a1.txt"));
    ON_SCOPE_EXIT {
      if (g_rootDir.exists("a")) {
        QVERIFY(QDir(g_rootPath + "/a").removeRecursively());
      }
    };

    QVERIFY2(g_rootDir.exists("a/a1.txt"), "Precondition not required.");

    FileOperatorType::RETURN_TYPE retEle = FileOperation::link(g_rootPath, "a/a1.txt", g_rootPath);
    auto aBatch = retEle.cmds;
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);

    QVERIFY(g_rootDir.exists("a/a1.txt"));
    QVERIFY(g_rootDir.exists("a/a1.txt.lnk"));

    BATCH_COMMAND_LIST_TYPE srcCommand;
    const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.ret;
    QCOMPARE(recoverRet, 0);
    QVERIFY(g_rootDir.exists("a/a1.txt"));
    QVERIFY(!g_rootDir.exists("a/a1.txt.lnk"));
  }

  void test_rename_a_txt_To_A_TXT_Not_Exists() {
    const QString lowerCaseName = "a.txt";
    const QString upperCaseName = "A.TXT";
    QVERIFY(CreateAFile(g_rootPath + "/" + lowerCaseName, "contents in a.txt"));
    ON_SCOPE_EXIT {
      QVERIFY(g_rootDir.entryList(QDir::Filter::Files).contains(lowerCaseName));
      QVERIFY(g_rootDir.remove(lowerCaseName));
    };
    QVERIFY2(g_rootDir.entryList(QDir::Filter::Files).contains(lowerCaseName), "A.txt should be recover to a.txt");

    RETURN_TYPE retEle =  //
        FileOperation::rename(g_rootPath, lowerCaseName, g_rootPath, upperCaseName);
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QVERIFY2(g_rootDir.entryList(QDir::Filter::Files).contains(upperCaseName), "A.txt should be recover to a.txt");

    auto aBatch = retEle.cmds;
    BATCH_COMMAND_LIST_TYPE srcCommand;
    const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    QCOMPARE(exeRetEle.ret, ErrorCode::OK);
    QCOMPARE(exeRetEle.isRecoverable(), true);
    QVERIFY2(g_rootDir.entryList(QDir::Filter::Files).contains(lowerCaseName), "A.txt should be recover to a.txt");
  }

  void test_rename_b_txt_to_A_TXT_failed_if_a_txt_already_exists() {
    QVERIFY(CreateAFile(g_rootPath + "/b.txt", "contents in b.txt"));
    QVERIFY(CreateAFile(g_rootPath + "/a.txt", "contents in a.txt"));
    ON_SCOPE_EXIT {
      QVERIFY(g_rootDir.remove("a.txt"));
      QVERIFY(g_rootDir.remove("b.txt"));
    };

    QVERIFY2(g_rootDir.exists("b.txt"), "Environment should met first");
    QVERIFY2(g_rootDir.exists("a.txt"), "Environment should met first");
    // rename b.txt -> A.TXT while {a.txt} already exist in destination
    const QString fileNameATXTDifferInCase = "A.TXT";
    RETURN_TYPE retEle =  //
        FileOperation::rename(g_rootPath, "b.txt", g_rootPath, fileNameATXTDifferInCase);

    QCOMPARE(retEle.ret, DST_FILE_OR_PATH_ALREADY_EXIST);
    QCOMPARE(retEle.isRecoverable(), false);
  }

  void test_rename_a_to_A_ok() {
    QVERIFY(g_rootDir.mkdir("a"));
    ON_SCOPE_EXIT {
      QVERIFY(g_rootDir.rmdir("A"));
    };

    QVERIFY2(g_rootDir.exists("a"), "Environment should met first");
    RETURN_TYPE retEle =  //
        FileOperation::rename(g_rootPath, "a", g_rootPath, "A");

    QCOMPARE(retEle.ret, OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QVERIFY2(g_rootDir.entryList(QDir::Filter::Dirs).contains("A"), "A should exist");
  }
};
#include "FileOperationTest.moc"
FileOperationTest g_FileOperationTest;
