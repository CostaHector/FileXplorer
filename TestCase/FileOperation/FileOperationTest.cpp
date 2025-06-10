#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "TestCase/pub/TDir.h"
#include "TestCase/pub/MyTestSuite.h"
#include "FileOperation/FileOperation.h"

using namespace FileOperatorType;

class FileOperationTest : public MyTestSuite {
  Q_OBJECT

 public:
  FileOperationTest() : MyTestSuite{false} {}
 private slots:
  void test_file_remove_not_recoverable() {
    TDir dir;
    QVERIFY(dir.touch("a.txt", "contents in a.txt"));
    const QString workPath{dir.path()};

    QVERIFY(dir.fileExists("a.txt", false));

    RETURN_TYPE retEle = FileOperation::rmfile(workPath, "a.txt");
    QCOMPARE(retEle.ret, ErrorCode::OK);

    QVERIFY(!dir.fileExists("a.txt", false));
    QVERIFY(retEle.cmds.isEmpty());  // not_recoverable
  }

  void test_rmpath() {
    // protect_remove_parent
    // a
    // a/a1
    // a/a1.txt
    TDir dir;
    QVERIFY(dir.touch("a/a1/a2.txt", "contents in a/a1/a2.txt"));
    QVERIFY(dir.mkpath("protect_remove_parent"));
    const QString workPath{dir.path()};

    // a/a1
    // cannot remove at all because there is "a2.txt" under "a/a1"
    const auto& rmpthFailed = FileOperation::rmpath(workPath, "a/a1");
    QCOMPARE(rmpthFailed.ret, CANNOT_REMOVE_PATH);
    QVERIFY(dir.fileExists("a/a1/a2.txt", false));

    // remove the file
    QVERIFY(QDir{workPath}.remove("a/a1/a2.txt"));
    // a
    // a/a1
    // can remove both
    // parent contains protect_remove_parent.txt, will not remove parent
    const auto& rmpthok = FileOperation::rmpath(workPath, "a/a1");
    QCOMPARE(rmpthok.ret, OK);
    QVERIFY(!dir.dirExists("a", false));

    auto aBatch = rmpthok.cmds;
    QVERIFY(!aBatch.isEmpty());

    BATCH_COMMAND_LIST_TYPE srcCommand;
    BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};

    const auto& rmpthRecOk = FileOperation::executer(reversedaBatch, srcCommand);
    QCOMPARE(rmpthRecOk.ret, OK);
    QVERIFY(dir.dirExists("a/a1", false));
  }

  void test_rmdir() {
    // protect_remove_parent
    // a
    // a/a1
    // a/a1/a2.txt

    TDir dir;
    QVERIFY(dir.touch("a/a1/a2.txt", "contents in a/a1/a2.txt"));
    QVERIFY(dir.mkpath("protect_remove_parent"));
    const QString workPath{dir.path()};

    // a/a1
    // cannot remove at all because there is "a2.txt" under "a/a1"
    const auto& rmDirFailed = FileOperation::rmdir(workPath, "a/a1");
    QCOMPARE(rmDirFailed.ret, CANNOT_REMOVE_DIR);
    QVERIFY(dir.fileExists("a/a1/a2.txt", false));

    // remove the file
    QVERIFY(QDir{workPath}.remove("a/a1/a2.txt"));
    // a
    // a/a1
    // can remove both
    // parent contains protect_remove_parent.txt, will not remove parent
    const auto& rmDirOk = FileOperation::rmdir(workPath, "a/a1");
    QCOMPARE(rmDirOk.ret, OK);
    QVERIFY(!dir.dirExists("a/a1", false));
    QVERIFY(dir.dirExists("a", false));

    auto aBatch = rmDirOk.cmds;
    QVERIFY(!aBatch.isEmpty());

    BATCH_COMMAND_LIST_TYPE srcCommand;
    BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};

    const auto& rmDirRecOk = FileOperation::executer(reversedaBatch, srcCommand);
    QCOMPARE(rmDirRecOk.ret, OK);
    QVERIFY(dir.dirExists("a/a1", false));
  }

  void test_rmFolderForce() {
    // a
    // a/a1
    // a/a1.txt

    TDir dir;
    QVERIFY(dir.mkpath("a/a1"));
    QVERIFY(dir.touch("a/a1.txt", "contents in a/a1.txt"));

    const QString workPath{dir.path()};

    QVERIFY(dir.dirExists("a/a1", false));
    QVERIFY(dir.fileExists("a/a1.txt", false));
    RETURN_TYPE retEle =  //
        FileOperation::rmFolderForce(workPath, "a");
    auto aBatch = retEle.cmds;
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QVERIFY(!dir.dirExists("a"));  // deleted
    QVERIFY(aBatch.isEmpty());     // remove cannot be recover
  }

  void test_file_to_trashbin_and_undo_ok() {
    TDir dir;
    QVERIFY(dir.touch("a.txt", "contents in a.txt"));
    QVERIFY(dir.fileExists("a.txt", false));

    const QString workPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::moveToTrash(workPath, "a.txt");
    const auto& aBatch = retEle.cmds;

    QCOMPARE(retEle.ret, ErrorCode::OK);
    QVERIFY(!dir.fileExists("a.txt", false));
    QVERIFY(!aBatch.isEmpty());

    BATCH_COMMAND_LIST_TYPE srcCommand;
    BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    QCOMPARE(exeRetEle.ret, ErrorCode::OK);
    QVERIFY(dir.fileExists("a.txt", false));
  }

  // cpfile
  void test_absolute_file_copy() {
    // a.txt
    // b
    // b/a.txt
    TDir dir;
    QVERIFY(dir.touch("a.txt", "contents in a.txt"));
    QVERIFY(dir.mkdir("b"));

    QString existFile("a.txt");
    QVERIFY(dir.exists(existFile));
    QVERIFY(dir.exists("b"));
    QVERIFY(!dir.exists(QString("b/%1").arg(existFile)));

    const QString workPath{dir.path()};

    RETURN_TYPE retEle =  //
        FileOperation::cpfile(workPath, existFile, QString("%1/b").arg(workPath));
    auto ret = retEle.ret;
    auto aBatch = retEle.cmds;

    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(dir.exists(QString("b/%1").arg(existFile)));

    QVERIFY(!aBatch.isEmpty());

    BATCH_COMMAND_LIST_TYPE srcCommand;
    BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};

    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.ret;

    QCOMPARE(recoverRet, ErrorCode::OK);
    QVERIFY(dir.exists(existFile));
    QVERIFY(dir.exists("b"));
    QVERIFY(!dir.exists(QString("b/%1").arg(existFile)));
  }

  void test_relative_file_copy() {
    TDir dir;
    QVERIFY(dir.touch("a/a1.txt", "contents in a/a1.txt"));
    QVERIFY(dir.mkdir("b"));

    const QString relativeExistFile("a/a1.txt");
    QVERIFY(dir.exists(relativeExistFile));
    QVERIFY(dir.exists("b"));
    QVERIFY(!dir.exists(QString("b/%1").arg(relativeExistFile)));

    const QString workPath{dir.path()};
    RETURN_TYPE retEle =                                    //
        FileOperation::cpfile(workPath, relativeExistFile,  //
                              QString("%1/b").arg(workPath));
    auto aBatch = retEle.cmds;

    QCOMPARE(retEle.ret, ErrorCode::OK);
    QVERIFY(dir.exists(QString("b/%1").arg(relativeExistFile)));

    QVERIFY(!aBatch.isEmpty());

    BATCH_COMMAND_LIST_TYPE srcCommand;
    BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};

    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.ret;

    QCOMPARE(recoverRet, ErrorCode::OK);
    QVERIFY(dir.exists(relativeExistFile));
    QVERIFY(!dir.exists("b"));  // "b" was removed by "RMPATH" in reversedaBatch
  }

  void test_inexist_file_copy() {
    TDir dir;
    QVERIFY(dir.mkdir("b"));

    QString inexistFileName("an inexist file blablablabla.txt");
    QVERIFY(!dir.fileExists(inexistFileName, false));
    QVERIFY(dir.dirExists("b", false));
    QVERIFY(!dir.fileExists(QString("b/%1").arg(inexistFileName), false));

    const QString workPath{dir.path()};
    RETURN_TYPE retEle =                                  //
        FileOperation::cpfile(workPath, inexistFileName,  //
                              QString("%1/b").arg(workPath));
    const auto& aBatch = retEle.cmds;

    QCOMPARE(retEle.ret, ErrorCode::SRC_INEXIST);
    QVERIFY(aBatch.isEmpty());

    QVERIFY(!dir.fileExists(inexistFileName, false));
    QVERIFY(dir.dirExists("b", false));
    QVERIFY(!dir.fileExists(QString("b/%1").arg(inexistFileName), false));
  }

  // link
  void test_link_a_file() {
    TDir dir;
    QVERIFY(dir.touch("a.txt", "contents in a.txt"));
    QVERIFY2(dir.exists("a.txt"), "Precondition not required.");
    const QString workPath{dir.path()};

    RETURN_TYPE retEle = FileOperation::link(workPath, "a.txt", workPath);
    auto aBatch = retEle.cmds;
    QCOMPARE(retEle.ret, ErrorCode::OK);

    QVERIFY(dir.exists("a.txt"));
    QVERIFY(dir.exists("a.txt.lnk"));
    QVERIFY(!aBatch.isEmpty());  // can recover

    BATCH_COMMAND_LIST_TYPE srcCommand;
    const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    QCOMPARE(exeRetEle.ret, ErrorCode::OK);
    QCOMPARE(exeRetEle.isRecoverable(), true);

    // lnk was removed, file linked to not removed
    QVERIFY(dir.exists("a.txt"));
    QVERIFY(!dir.exists("a.txt.lnk"));
  }

  void test_link_a_relative_file() {
    TDir dir;
    dir.touch("a/a1.txt", "contents in a/a1.txt");

    QVERIFY2(dir.exists("a/a1.txt"), "Precondition not required.");

    const QString workPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::link(workPath, "a/a1.txt", workPath);
    auto aBatch = retEle.cmds;
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);

    QVERIFY(dir.exists("a/a1.txt"));
    QVERIFY(dir.exists("a/a1.txt.lnk"));

    BATCH_COMMAND_LIST_TYPE srcCommand;
    const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    QCOMPARE(exeRetEle.ret, ErrorCode::OK);
    QCOMPARE(exeRetEle.isRecoverable(), true);
    QVERIFY(dir.exists("a/a1.txt"));
    QVERIFY(!dir.exists("a/a1.txt.lnk"));
  }

  // 2 files rename testcase
  void test_rename_atxt_to_ATXT_existed_atxt_ok() {
    const QString lowerCaseName = "a.txt";
    const QString upperCaseName = "A.TXT";
    TDir dir;
    QVERIFY(dir.touch(lowerCaseName, "contents in a.txt"));
    QVERIFY2(dir.fileExists(lowerCaseName, true), "a.txt should exists");
    const QString workPath{dir.path()};
    RETURN_TYPE retEle =  //
        FileOperation::rename(workPath, lowerCaseName, upperCaseName);
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QVERIFY2(dir.fileExists(upperCaseName, true), "a.txt should be renamed to A.txt");

    auto aBatch = retEle.cmds;
    BATCH_COMMAND_LIST_TYPE srcCommand;
    const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    QCOMPARE(exeRetEle.ret, ErrorCode::OK);
    QCOMPARE(exeRetEle.isRecoverable(), true);
    QVERIFY2(dir.fileExists(lowerCaseName, true), "A.txt should be recover to a.txt");
  }

  void test_rename_btxt_to_ATXT_existed_atxt_btxt_windows_failed_linux_ok() {
    TDir dir;
    QVERIFY(dir.touch("a.txt", "contents in a.txt"));
    QVERIFY(dir.touch("b.txt", "contents in b.txt"));

    QCOMPARE(dir.entryList(QDir::Filter::Files, QDir::SortFlag::Name), (QStringList{"a.txt", "b.txt"}));
    // rename b.txt -> A.TXT while {a.txt} already exist in destination
    const QString workPath{dir.path()};
    const QString fileNameATXTDifferInCase = "A.TXT";
    RETURN_TYPE retEle = FileOperation::rename(workPath, "b.txt", fileNameATXTDifferInCase);

#ifdef WIN32
    QCOMPARE(retEle.ret, ErrorCode::DST_FILE_OR_PATH_ALREADY_EXIST);
    QCOMPARE(retEle.isRecoverable(), false);
    QCOMPARE(dir.entryList(QDir::Filter::Files, QDir::SortFlag::Name), (QStringList{"a.txt", "b.txt"}));
#else
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QCOMPARE(dir.entryList(QDir::Filter::Files, QDir::SortFlag::Name), (QStringList{"A.TXT", "a.txt"}));
#endif
  }

  // 8 folders rename testcase
  void test_rename_a_to_A_existed_a_ok() {
    TDir dir;
    QVERIFY(dir.mkdir("a"));
    QVERIFY2(dir.dirExists("a", true), "Environment should met first");
    const QString workPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::rename(workPath, "a", "A");
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QVERIFY2(dir.dirExists("A", true), "A should exist");
  }

  void test_rename_a_to_b_existed_a_ok() {
    TDir dir;
    QVERIFY(dir.mkdir("a"));
    QVERIFY2(dir.dirExists("a", true), "Environment should met first");
    const QString workPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::rename(workPath, "a", "b");
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QVERIFY2(dir.dirExists("b", true), "b should exist");
  }

  void test_rename_a_to_A_existed_a_A_linux_ok() {
    // only linux
#ifndef _WIN32
    TDir dir;
    QVERIFY(dir.mkdir("a"));
    QVERIFY(dir.mkdir("A"));
    QCOMPARE(dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"A", "a"}));
    const QString workPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::rename(workPath, "a", "A");
    QCOMPARE(retEle.ret, ErrorCode::DST_FILE_OR_PATH_ALREADY_EXIST);
    QCOMPARE(dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"A", "a"}));
#endif
  }

  void test_rename_a_to_a_skip_ok() {
    QString workPath;
    RETURN_TYPE retaaEle = FileOperation::rename(workPath, "a", "a");
    QCOMPARE(retaaEle.ret, ErrorCode::OK);

    RETURN_TYPE retAAEle = FileOperation::rename(workPath, "A", "A");
    QCOMPARE(retAAEle.ret, ErrorCode::OK);
  }

  void test_rename_a_to_b_existed_a_b_failed_dst_already_exists() {
    TDir dir;
    QVERIFY(dir.mkdir("a"));
    QVERIFY(dir.mkdir("b"));
    QCOMPARE(dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"a", "b"}));

    const QString workPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::rename(workPath, "a", "b");
    QCOMPARE(retEle.ret, ErrorCode::DST_FILE_OR_PATH_ALREADY_EXIST);
    QCOMPARE(dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"a", "b"}));
  }

  void test_rename_a_to_B_existed_a_B_failed_dst_already_exists() {
    TDir dir;
    QVERIFY(dir.mkdir("a"));
    QVERIFY(dir.mkdir("B"));
    QCOMPARE(dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"B", "a"}));

    const QString workPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::rename(workPath, "a", "B");
    QCOMPARE(retEle.ret, ErrorCode::DST_FILE_OR_PATH_ALREADY_EXIST);
    QCOMPARE(dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"B", "a"}));
  }

  void test_rename_a_to_b_existed_a_B_failed_dst_already_exists() {
    TDir dir;
    QVERIFY(dir.mkdir("a"));
    QVERIFY(dir.mkdir("B"));
    QCOMPARE(dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"B", "a"}));

    const QString workPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::rename(workPath, "a", "b");
#ifdef WIN32
    QCOMPARE(retEle.ret, ErrorCode::DST_FILE_OR_PATH_ALREADY_EXIST);
    QCOMPARE(dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"B", "a"}));
#else
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QCOMPARE(dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"B", "b"}));
#endif
  }
  void test_rename_a_to_B_existed_a_b_failed_dst_already_exists() {
    TDir dir;
    QVERIFY(dir.mkdir("a"));
    QVERIFY(dir.mkdir("b"));
    QCOMPARE(dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"a", "b"}));

    const QString workPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::rename(workPath, "a", "B");
#ifdef WIN32
    QCOMPARE(retEle.ret, ErrorCode::DST_FILE_OR_PATH_ALREADY_EXIST);
    QCOMPARE(dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"a", "b"}));
#else
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QCOMPARE(dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"B", "b"}));
#endif
  }

  // 9 mv file(s)/folder(s) testcase
  void test_mv_any_from_home_to_home_skip() {
    const QString fromPath{"home"};
    const QString dstPath{"home"};
    RETURN_TYPE retEle = FileOperation::mv(fromPath, "AnyFile", dstPath);
    QCOMPARE(retEle.ret, ErrorCode::OK);
  }

  void test_mv_any_from_home_to_HOME_skip() {
    const QString fromPath{"home"};
    const QString dstPath{"HOME"};
    RETURN_TYPE retEle = FileOperation::mv(fromPath, "AnyFile", dstPath);
    QCOMPARE(retEle.ret, ErrorCode::OK);
  }

  void test_mv_atxt_from_home_to_bin_ok() {
    TDir dir;
    dir.touch("home/a.txt", "contents in a.txt");
    dir.mkdir("bin");
    QVERIFY(dir.fileExists("home/a.txt", false));
    RETURN_TYPE retEle = FileOperation::mv(dir.itemPath("home"), "a.txt", dir.itemPath("bin"));
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QVERIFY(dir.fileExists("bin/a.txt", false));

    auto aBatch = retEle.cmds;
    BATCH_COMMAND_LIST_TYPE srcCommand;
    const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    QCOMPARE(exeRetEle.ret, ErrorCode::OK);
    QCOMPARE(exeRetEle.isRecoverable(), true);
    QVERIFY(dir.fileExists("home/a.txt", false));
  }

  void test_mv_atxt_from_home_to_bin_existed_atxt_ok() {
    TDir dir;
    dir.touch("home/a.txt", "contents in a.txt");
    dir.touch("bin/a.txt");

    QVERIFY(dir.fileExists("home/a.txt", false));
    RETURN_TYPE retEle = FileOperation::mv(dir.itemPath("home"), "a.txt", dir.itemPath("bin"));
    QCOMPARE(retEle.ret, ErrorCode::DST_FILE_OR_PATH_ALREADY_EXIST);
    QCOMPARE(retEle.isRecoverable(), false);
    QVERIFY(dir.fileExists("home/a.txt", false));
    QVERIFY(dir.fileExists("bin/a.txt", false));
  }

  void test_mv_atxt_from_home_to_bin_existed_ATXT_ok() {
    TDir dir;
    dir.touch("home/a.txt", "contents in a.txt");
    dir.touch("bin/A.TXT");

    QVERIFY(dir.fileExists("home/a.txt", false));
    RETURN_TYPE retEle = FileOperation::mv(dir.itemPath("home"), "a.txt", dir.itemPath("bin"));
#ifdef _WIN32
    QCOMPARE(retEle.ret, ErrorCode::DST_FILE_OR_PATH_ALREADY_EXIST);
    QCOMPARE(retEle.isRecoverable(), false);
    QVERIFY(dir.fileExists("home/a.txt", false));
    QVERIFY(dir.fileExists("bin/A.TXT", false));

#else  // in linux "a.txt" "A.TXT" exists in bin
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QVERIFY(!dir.fileExists("home/a.txt", false));

    QDir binDir{dir.itemPath("bin")};
    QCOMPARE(binDir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name), (QStringList{"A.TXT", "a.txt"}));
#endif
  }

  void test_mv_path_to_a_from_home_to_bin_ok() {
    TDir dir;
    dir.mkpath("home/path/to/a");
    dir.mkpath("bin");

    RETURN_TYPE retEle = FileOperation::mv(dir.itemPath("home"), "path/to/a", dir.itemPath("bin"));

    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QVERIFY(!dir.dirExists("home/path/to/a", false));
    QVERIFY(dir.dirExists("bin/path/to/a", false));
  }

  void test_mv_path_to_a_from_home_to_bin_existed_path_to_ok() {
    TDir dir;
    dir.mkpath("home/path/to/a");
    dir.mkpath("bin/path/to");

    RETURN_TYPE retEle = FileOperation::mv(dir.itemPath("home"), "path/to/a", dir.itemPath("bin"));

    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QVERIFY(!dir.dirExists("home/path/to/a", false));
    QVERIFY(dir.dirExists("bin/path/to/a", false));
  }

  void test_mv_path_to_a_from_home_to_bin_existed_path_to_a_dst_already_exist() {
    TDir dir;
    dir.mkpath("home/path/to/a");
    dir.mkpath("bin/path/to/a");

    RETURN_TYPE retEle = FileOperation::mv(dir.itemPath("home"), "path/to/a", dir.itemPath("bin"));

    QCOMPARE(retEle.ret, ErrorCode::DST_FILE_OR_PATH_ALREADY_EXIST);
    QCOMPARE(retEle.isRecoverable(), false);
    QVERIFY(dir.dirExists("home/path/to/a", false));
    QVERIFY(dir.dirExists("bin/path/to/a", false));
  }

  void test_mv_path_to_atxt_from_home_to_bin_ok() {
    TDir dir;
    dir.touch("home/path/to/a.md", "content in markdown");
    dir.mkpath("bin");

    RETURN_TYPE retEle = FileOperation::mv(dir.itemPath("home"), "path/to/a.md", dir.itemPath("bin"));

    QCOMPARE(retEle.ret, ErrorCode::OK);
    QCOMPARE(retEle.isRecoverable(), true);
    QVERIFY(!dir.fileExists("home/path/to/a.md", false));
    QVERIFY(dir.fileExists("bin/path/to/a.md", false));
  }

  // cpdir
  void test_cpdir_absolute_folder_copy() {
    QString existFolder("a");
    QString subDir("a/a1");
    QString subFile("a/a1.txt");

    TDir dir;
    QVERIFY(dir.mkpath("a/a1"));
    QVERIFY(dir.touch("a/a1.txt", "contents in a/a1.txt"));
    QVERIFY(dir.mkpath("b"));

    const QString mTestPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::cpdir(mTestPath, existFolder,  //
                                              QString("%1/b").arg(mTestPath));
    auto aBatch = retEle.cmds;

    QCOMPARE(retEle.ret, ErrorCode::OK);
    QVERIFY(dir.dirExists(existFolder, false));
    QVERIFY(dir.dirExists(subDir, false));
    QVERIFY(dir.fileExists(subFile, false));
    QVERIFY(dir.dirExists("b", false));
    QVERIFY(dir.dirExists(QString("b/%1").arg(subDir), false));
    QVERIFY(dir.dirExists(QString("b/%1").arg(existFolder), false));
    QVERIFY(dir.fileExists(QString("b/%1").arg(subFile), false));
    QVERIFY(!aBatch.isEmpty());
    BATCH_COMMAND_LIST_TYPE srcCommand;
    const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);

    QCOMPARE(exeRetEle.ret, ErrorCode::OK);
    QVERIFY(dir.dirExists(existFolder, false));
    QVERIFY(dir.dirExists(subDir, false));
    QVERIFY(dir.fileExists(subFile, false));
    QVERIFY(!dir.dirExists("b", false));  // mkpath -> rmpath vice function
    QVERIFY(!dir.dirExists(QString("b/%1").arg(existFolder), false));
    QVERIFY(!dir.dirExists(QString("b/%1").arg(subDir), false));
    QVERIFY(!dir.fileExists(QString("b/%1").arg(subFile), false));
  }

  void test_relative_folder_copy() {
    QString relativeExistFolder{"a/a1/a2"};
    QString subDir("a/a1/a2/a3");
    QString subFile("a/a1/a2/a3.txt");

    TDir dir;
    QVERIFY(dir.mkpath(relativeExistFolder));
    QVERIFY(dir.mkpath(subDir));
    QVERIFY(dir.touch(subFile, ""));
    QVERIFY(dir.mkpath("b"));

    QVERIFY(dir.dirExists(relativeExistFolder, false));
    QVERIFY(dir.dirExists(subDir, false));
    QVERIFY(dir.fileExists(subFile, false));
    QVERIFY(dir.dirExists("b", false));
    QVERIFY(!dir.dirExists(QString("b/%1").arg(relativeExistFolder), false));
    QVERIFY(!dir.dirExists(QString("b/%1").arg(subDir), false));
    QVERIFY(!dir.fileExists(QString("b/%1").arg(subFile), false));
    const QString mTestPath{dir.path()};
    RETURN_TYPE retEle =  //
        FileOperation::cpdir(mTestPath, relativeExistFolder, QString("%1/b").arg(mTestPath));

    auto aBatch = retEle.cmds;
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QVERIFY(dir.dirExists(relativeExistFolder, false));
    QVERIFY(dir.dirExists(subDir, false));
    QVERIFY(dir.fileExists(subFile, false));
    QVERIFY(dir.dirExists("b", false));
    QVERIFY(dir.dirExists(QString("b/%1").arg(subDir), false));
    QVERIFY(dir.dirExists(QString("b/%1").arg(relativeExistFolder), false));
    QVERIFY(dir.fileExists(QString("b/%1").arg(subFile), false));
    QVERIFY(!aBatch.isEmpty());

    BATCH_COMMAND_LIST_TYPE srcCommand;
    const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);

    QCOMPARE(exeRetEle.ret, ErrorCode::OK);
    QVERIFY(dir.dirExists(relativeExistFolder, false));
    QVERIFY(dir.dirExists(subDir, false));
    QVERIFY(dir.fileExists(subFile, false));
    QVERIFY(!dir.dirExists("b", false));  // mkpath->rmpath vice function
    QVERIFY(!dir.dirExists(QString("b/%1").arg(relativeExistFolder), false));
    QVERIFY(!dir.dirExists(QString("b/%1").arg(subDir), false));
    QVERIFY(!dir.fileExists(QString("b/%1").arg(subFile), false));
  }

  void test_inexist_folder_copy() {
    QString inexistFolder("an inexist folder blablablabla");
    TDir dir;
    QVERIFY(dir.mkdir("b"));

    QVERIFY(!dir.dirExists(inexistFolder, false));
    QVERIFY(dir.dirExists("b", false));
    QVERIFY(!dir.dirExists(QString("b/%1").arg(inexistFolder), false));

    const QString mTestPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::cpdir(mTestPath, inexistFolder, QString("%1/b").arg(mTestPath));
    const auto& aBatch = retEle.cmds;

    QCOMPARE(retEle.ret, ErrorCode::SRC_INEXIST);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(!dir.dirExists(inexistFolder, false));
    QVERIFY(dir.dirExists("b", false));
    QVERIFY(!dir.dirExists(QString("b/%1").arg(inexistFolder), false));
  }

  void test_executer_return_if_any_cmd_failed() {
    QString inexistFolder("inexistFolder");
    QString existFile("b.txt");

    TDir dir;
    QVERIFY(dir.touch("b.txt", "contents in b.txt"));
    QVERIFY(!dir.dirExists(inexistFolder, false));
    QVERIFY(dir.fileExists(existFile, false));

    const QString mTestPath{dir.path()};
    BATCH_COMMAND_LIST_TYPE aBatch, srcCommand;
    aBatch.append(ACMD::GetInstRENAME(mTestPath, inexistFolder, "NewName inexistFolder"));
    aBatch.append(ACMD::GetInstRENAME(mTestPath, existFile, "NewName b.txt"));

    RETURN_TYPE retEle = FileOperation::executer(aBatch, srcCommand);
    QCOMPARE(retEle.ret, ErrorCode::SRC_INEXIST);
    QVERIFY(!dir.dirExists("NewName inexistFolder", false));
    QVERIFY(!dir.fileExists("NewName b.txt"));
    QVERIFY(dir.fileExists(existFile));
  }

  void test_touch_file_ok() {
    TDir dir;
    const QString mTestPath{dir.path()};
    QVERIFY(!dir.fileExists("a new json file.json", false));
    RETURN_TYPE retEle = FileOperation::touch(mTestPath, "a new json file.json");
    auto aBatch = retEle.cmds;
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QVERIFY(QDir(mTestPath).exists("a new json file.json"));
    QVERIFY(!aBatch.isEmpty());
    BATCH_COMMAND_LIST_TYPE srcCommand;
    const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    QCOMPARE(exeRetEle.ret, ErrorCode::OK);
    QVERIFY(!dir.fileExists("a new json file.json", false));
  }

  void test_touch_relative_path_file() {
    TDir dir;
    const QString mTestPath{dir.path()};
    QVERIFY(!dir.fileExists("path/to/a new json file.json", false));

    RETURN_TYPE retEle = FileOperation::touch(mTestPath, "path/to/a new json file.json");

    const auto& aBatch = retEle.cmds;
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QVERIFY(dir.fileExists("path/to/a new json file.json", false));
    QVERIFY(!aBatch.isEmpty());
    BATCH_COMMAND_LIST_TYPE srcCommand;
    const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
    const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.ret;

    QCOMPARE(recoverRet, ErrorCode::OK);
    QVERIFY(!dir.fileExists("path/to/a new json file.json", false));
  }

  void test_touch_existed_file() {
    TDir dir;
    QVERIFY(dir.touch("a.txt", ""));
    const QString mTestPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::touch(mTestPath, "a.txt");
    auto aBatch = retEle.cmds;
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(dir.fileExists("a.txt", false));
  }

  void test_touch_existed_relative_path_file() {
    TDir dir;
    QVERIFY(dir.touch("a/a.txt", ""));
    const QString mTestPath{dir.path()};
    RETURN_TYPE retEle = FileOperation::touch(mTestPath, "a/a.txt");
    auto aBatch = retEle.cmds;
    QCOMPARE(retEle.ret, ErrorCode::OK);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(dir.fileExists("a/a.txt", false));
  }
};
#include "FileOperationTest.moc"
FileOperationTest g_FileOperationTest;
