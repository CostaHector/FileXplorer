#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "pub/FileSystemTestSuite.h"
#include "FileOperation/FileOperation.h"
#include "FileOperation/FileOperatorPub.h"

using namespace FileOperatorType;

class FileOperationTest : public FileSystemTestSuite {
  Q_OBJECT

 public:
  FileOperationTest() : FileSystemTestSuite{"TestEnv_FileOperation", true, false} {}
 private slots:
  void init() {
    // a, b, a.txt, b.txt
    m_rootHelper << FileSystemNode{"a"} << FileSystemNode{"b"} << FileSystemNode{"a.txt", false, ""} << FileSystemNode{"b.txt", false, ""};
    // a{a1{a2{a3,a3.txt}, a2.txt}, a1.txt}
    m_rootHelper.GetSubHelper("a") << FileSystemNode{"a1"} << FileSystemNode{"a1.txt", false, ""};
    m_rootHelper.GetSubHelper("a").GetSubHelper("a1") << FileSystemNode{"a2"} << FileSystemNode{"a2.txt", false, ""};
    m_rootHelper.GetSubHelper("a").GetSubHelper("a1").GetSubHelper("a2") << FileSystemNode{"a3"} << FileSystemNode{"a3.txt", false, ""};
    // b{b1{b2, b2.txt}, b1.txt}
    m_rootHelper.GetSubHelper("b") << FileSystemNode{"b1"} << FileSystemNode{"b1.txt", false, ""};
    m_rootHelper.GetSubHelper("b").GetSubHelper("b1") << FileSystemNode{"b2"} << FileSystemNode{"b2.txt", false, ""};
  }

  void test_file_remove();
  void test_folder_remove();

  void test_file_to_trashbin();

  // cpfile
  void test_absolute_file_copy();
  void test_relative_file_copy();
  void test_inexist_file_copy();

  // cpdir
  void test_absolute_folder_copy();
  void test_relative_folder_copy();
  void test_inexist_folder_copy();

  // rename file same folder
  void test_file_move_same_directory_unique_name();
  void test_file_move_same_directory_conflict_name();
  // rename folder same folder
  void test_folder_move_same_directory_unique_name();
  void test_folder_move_same_directory_conflict_name();

  // rename file differ folder
  void test_file_move_jump_directory_unique_name();
  void test_file_move_jump_directory_conflict_name();
  // rename folder differ folder
  void test_folder_move_jump_directory_unique_name();
  void test_folder_move_jump_directory_conflict_name();

  void test_rename_relative_file();

  void test_touch_a_json_file_in_direct_path();
  void test_touch_a_json_file_in_relative_path();
  void test_touch_an_existed_txt_file_in_direct_path();
  void test_touch_an_existed_txt_file_in_relative_path();
  void test_touch_a_folder_in_direct_path();
  void test_touch_a_folder_in_relative_path();
  void test_touch_an_existed_folder_in_direct_path();
  void test_touch_an_existed_folder_in_relative_path();

  void test_link_a_file();
  void test_link_a_relative_file();

  void test_rename_a_txt_To_A_TXT_Not_Exists();
  void test_rename_b_txt_To_A_TXT_Already_Exists();

  void test_executer_return_if_any_cmd_failed();
};

void FileOperationTest::test_file_remove() {
  QVERIFY(QDir(mTestPath).exists("a.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rmfile(mTestPath, "a.txt");
  QCOMPARE(retEle.ret, ErrorCode::OK);

  QVERIFY(not QDir(mTestPath).exists("a.txt"));
  QVERIFY(retEle.cmds.isEmpty());
}

void FileOperationTest::test_folder_remove() {
  QVERIFY(QDir(mTestPath).exists("a"));
  QVERIFY(QDir(mTestPath).exists("a/a1"));
  QVERIFY(QDir(mTestPath).exists("a/a1.txt"));

  FileOperatorType::RETURN_TYPE retEle = FileOperation::rmdir(mTestPath, "a");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(!QDir(mTestPath).exists("a/a1.txt"));
  QVERIFY(!QDir(mTestPath).exists("a/a1"));
  QVERIFY(!QDir(mTestPath).exists("a"));
  QVERIFY(aBatch.isEmpty());  // remove cannot be recover
}

void FileOperationTest::test_file_to_trashbin() {
  QVERIFY(QDir(mTestPath).exists("a.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::moveToTrash(mTestPath, "a.txt");

  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(not QDir(mTestPath).exists("a.txt"));

  QVERIFY(not aBatch.isEmpty());

  BATCH_COMMAND_LIST_TYPE srcCommand;
  BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;
  QVERIFY(recoverRet == 0);
  QVERIFY(QDir(mTestPath).exists("a.txt"));
}

void FileOperationTest::test_absolute_file_copy() {
  QString existFile("a.txt");
  QVERIFY(QDir(mTestPath).exists(existFile));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(existFile)));

  FileOperatorType::RETURN_TYPE retEle = FileOperation::cpfile(mTestPath, existFile, QString("%1/b").arg(mTestPath));
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(mTestPath).exists(QString("b/%1").arg(existFile)));

  QVERIFY(not aBatch.isEmpty());

  BATCH_COMMAND_LIST_TYPE srcCommand;
  BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};

  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(mTestPath).exists(existFile));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(not QDir(mTestPath).exists(QString("b/%1").arg(existFile)));
}

// a, b, a.txt, b.txt
// a{a1{a2{a3,a3.txt}, a2.txt}, a1.txt}
// b{b1{b2, b2.txt}, b1.txt}

void FileOperationTest::test_relative_file_copy() {
  QString relativeExistFile("a/a1.txt");
  QVERIFY(QDir(mTestPath).exists(relativeExistFile));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(relativeExistFile)));

  FileOperatorType::RETURN_TYPE retEle = FileOperation::cpfile(mTestPath, relativeExistFile, QString("%1/b").arg(mTestPath));
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(mTestPath).exists(QString("b/%1").arg(relativeExistFile)));

  QVERIFY(!aBatch.isEmpty());

  BATCH_COMMAND_LIST_TYPE srcCommand;
  BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};

  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(mTestPath).exists(relativeExistFile));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(relativeExistFile)));
}

void FileOperationTest::test_inexist_file_copy() {
  QString inexistFileName("an inexist file blablablabla.txt");
  QVERIFY(!QDir(mTestPath).exists(inexistFileName));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(inexistFileName)));

  FileOperatorType::RETURN_TYPE retEle = FileOperation::cpfile(mTestPath, inexistFileName, QString("%1/b").arg(mTestPath));
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QVERIFY(ret != ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(!QDir(mTestPath).exists(inexistFileName));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(inexistFileName)));
}

void FileOperationTest::test_absolute_folder_copy() {
  QString existFolder("a");
  QString subDir("a/a1");
  QString subFile("a/a1.txt");

  QVERIFY(QDir(mTestPath).exists(existFolder));
  QVERIFY(QDir(mTestPath).exists(subDir));
  QVERIFY(QDir(mTestPath).exists(subFile));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(existFolder)));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(subDir)));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(subFile)));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::cpdir(mTestPath, existFolder, QString("%1/b").arg(mTestPath));

  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(mTestPath).exists(existFolder));
  QVERIFY(QDir(mTestPath).exists(subDir));
  QVERIFY(QDir(mTestPath).exists(subFile));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(QDir(mTestPath).exists(QString("b/%1").arg(subDir)));
  QVERIFY(QDir(mTestPath).exists(QString("b/%1").arg(existFolder)));
  QVERIFY(QDir(mTestPath).exists(QString("b/%1").arg(subFile)));
  QVERIFY(!aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(mTestPath).exists(existFolder));
  QVERIFY(QDir(mTestPath).exists(subDir));
  QVERIFY(QDir(mTestPath).exists(subFile));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(existFolder)));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(subDir)));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(subFile)));
}
// a, b, a.txt, b.txt
// a{a1{a2{a3,a3.txt}, a2.txt}, a1.txt}
// b{b1{b2, b2.txt}, b1.txt}
void FileOperationTest::test_relative_folder_copy() {
  QString relativeExistFolder{"a/a1/a2"};
  QString subDir("a/a1/a2/a3");
  QString subFile("a/a1/a2/a3.txt");

  QVERIFY(QDir(mTestPath).exists(relativeExistFolder));
  QVERIFY(QDir(mTestPath).exists(subDir));
  QVERIFY(QDir(mTestPath).exists(subFile));

  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(relativeExistFolder)));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(subDir)));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(subFile)));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::cpdir(mTestPath, relativeExistFolder, QString("%1/b").arg(mTestPath));

  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(mTestPath).exists(relativeExistFolder));
  QVERIFY(QDir(mTestPath).exists(subDir));
  QVERIFY(QDir(mTestPath).exists(subFile));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(QDir(mTestPath).exists(QString("b/%1").arg(subDir)));
  QVERIFY(QDir(mTestPath).exists(QString("b/%1").arg(relativeExistFolder)));
  QVERIFY(QDir(mTestPath).exists(QString("b/%1").arg(subFile)));
  QVERIFY(!aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(mTestPath).exists(relativeExistFolder));
  QVERIFY(QDir(mTestPath).exists(subDir));
  QVERIFY(QDir(mTestPath).exists(subFile));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(relativeExistFolder)));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(subDir)));
  QVERIFY(!QDir(mTestPath).exists(QString("b/%1").arg(subFile)));
}

void FileOperationTest::test_inexist_folder_copy() {
  QString inexistFolder("an inexist folder blablablabla");
  QVERIFY(not QDir(mTestPath).exists(inexistFolder));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(not QDir(mTestPath).exists(QString("b/%1").arg(inexistFolder)));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::cpdir(mTestPath, inexistFolder, QString("%1/b").arg(mTestPath));

  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QVERIFY(ret != ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(!QDir(mTestPath).exists(inexistFolder));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(not QDir(mTestPath).exists(QString("b/%1").arg(inexistFolder)));
}

void FileOperationTest::test_file_move_same_directory_unique_name() {
  QVERIFY(QDir(mTestPath).exists("a.txt"));
  QVERIFY(!QDir(mTestPath).exists("a moved.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(mTestPath, "a.txt", mTestPath, "a moved.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(not QDir(mTestPath).exists("a.txt"));
  QVERIFY(QDir(mTestPath).exists("a moved.txt"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(mTestPath).exists("a.txt"));
  QVERIFY(not QDir(mTestPath).exists("a moved.txt"));
}

void FileOperationTest::test_file_move_same_directory_conflict_name() {
  QVERIFY(QDir(mTestPath).exists("a.txt"));
  QVERIFY(QDir(mTestPath).exists("b.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(mTestPath, "a.txt", mTestPath, "b.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QVERIFY(ret != ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(mTestPath).exists("a.txt"));
  QVERIFY(QDir(mTestPath).exists("b.txt"));
}

void FileOperationTest::test_folder_move_same_directory_unique_name() {
  QVERIFY(QDir(mTestPath).exists("a"));
  QVERIFY(not QDir(mTestPath).exists("a moved"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(mTestPath, "a", mTestPath, "a moved");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(not QDir(mTestPath).exists("a"));
  QVERIFY(QDir(mTestPath).exists("a moved"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(mTestPath).exists("a"));
  QVERIFY(not QDir(mTestPath).exists("a moved"));
}
void FileOperationTest::test_folder_move_same_directory_conflict_name() {
  QVERIFY(QDir(mTestPath).exists("a"));
  QVERIFY(QDir(mTestPath).exists("b"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(mTestPath, "a", mTestPath, "b");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QVERIFY(ret != ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(mTestPath).exists("a"));
  QVERIFY(QDir(mTestPath).exists("b"));
}
void FileOperationTest::test_file_move_jump_directory_unique_name() {
  const QString& TO = QDir(mTestPath).absoluteFilePath("b");
  QVERIFY(QDir(mTestPath).exists("a.txt"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(!QDir(TO).exists("path/to/a moved.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(mTestPath, "a.txt", TO, "path/to/a moved.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(not QDir(mTestPath).exists("a.txt"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(QDir(TO).exists("path/to/a moved.txt"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(mTestPath).exists("a.txt"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(not QDir(TO).exists("path/to/a moved.txt"));
}

void FileOperationTest::test_file_move_jump_directory_conflict_name() {
  const QString& TO = QDir(mTestPath).absoluteFilePath("b");
  QVERIFY(QDir(mTestPath).exists("a.txt"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(QDir(TO).exists("b1/b2.txt"));  // conflict here;
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(mTestPath, "a.txt", TO, "b1/b2.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QVERIFY(ret != ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(mTestPath).exists("a.txt"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(QDir(TO).exists("b1/b2.txt"));
}
void FileOperationTest::test_folder_move_jump_directory_unique_name() {
  const QString& TO = QDir(mTestPath).absoluteFilePath("b");
  QVERIFY(QDir(mTestPath).exists("a"));
  QVERIFY(QDir(mTestPath).exists("a/a1"));
  QVERIFY(QDir(mTestPath).exists("a/a1.txt"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(not QDir(TO).exists("path/to/a moved"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(mTestPath, "a", TO, "path/to/a moved");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(not QDir(mTestPath).exists("a"));
  QVERIFY(not QDir(mTestPath).exists("a/a1"));
  QVERIFY(not QDir(mTestPath).exists("a/a1.txt"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(QDir(TO).exists("path/to/a moved"));
  QVERIFY(QDir(TO).exists("path/to/a moved/a1"));
  QVERIFY(QDir(TO).exists("path/to/a moved/a1.txt"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(mTestPath).exists("a"));
  QVERIFY(QDir(mTestPath).exists("a/a1"));
  QVERIFY(QDir(mTestPath).exists("a/a1.txt"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(not QDir(TO).exists("path/to/a moved"));
}

void FileOperationTest::test_rename_relative_file() {
  // a, b, a.txt, b.txt
  // a{a1{a2{a3,a3.txt}, a2.txt}, a1.txt}
  // b{b1{b2, b2.txt}, b1.txt}
  const QString& TO = QDir(mTestPath).absoluteFilePath("b");
  QVERIFY(QDir(mTestPath).exists("a/a1.txt"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(!QDir(TO).exists("path/to/a moved"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(mTestPath, "a/a1.txt", TO, "a/a1.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(!QDir(mTestPath).exists("a/a1.txt"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(QDir(TO).exists("a/a1.txt"));
  QVERIFY(!aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(mTestPath).exists("a/a1.txt"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(!QDir(TO).exists("a/a1.txt"));
}

void FileOperationTest::test_folder_move_jump_directory_conflict_name() {
  const QString& TO = QDir(mTestPath).absoluteFilePath("b");
  QVERIFY(QDir(mTestPath).exists("a"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(QDir(TO).exists("b1/b2"));  // conflict here;
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(mTestPath, "a", TO, "b1/b2");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QVERIFY(ret != ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(mTestPath).exists("a"));
  QVERIFY(QDir(mTestPath).exists("b"));
  QVERIFY(QDir(TO).exists("b1/b2"));
}
void FileOperationTest::test_touch_a_json_file_in_direct_path() {
  QVERIFY(not QDir(mTestPath).exists("a new json file.json"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(mTestPath, "a new json file.json");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(mTestPath).exists("a new json file.json"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(not QDir(mTestPath).exists("a new json file.json"));
}
void FileOperationTest::test_touch_a_json_file_in_relative_path() {
  QVERIFY(not QDir(mTestPath).exists("path/to/a new json file.json"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(mTestPath, "path/to/a new json file.json");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(mTestPath).exists("path/to/a new json file.json"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(not QDir(mTestPath).exists("path/to/a new json file.json"));
}
void FileOperationTest::test_touch_an_existed_txt_file_in_direct_path() {
  QVERIFY(QDir(mTestPath).exists("a.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(mTestPath, "a.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(mTestPath).exists("a.txt"));
}
void FileOperationTest::test_touch_an_existed_txt_file_in_relative_path() {
  QVERIFY(QDir(mTestPath).exists("a/a1.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(mTestPath, "a/a1.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(mTestPath).exists("a/a1.txt"));
}
void FileOperationTest::test_touch_a_folder_in_direct_path() {
  QVERIFY(not QDir(mTestPath).exists("a new folder"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(mTestPath, "a new folder");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(mTestPath).exists("a new folder"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(not QDir(mTestPath).exists("a new folder"));
}
void FileOperationTest::test_touch_a_folder_in_relative_path() {
  QVERIFY(not QDir(mTestPath).exists("path/to/a new folder"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(mTestPath, "path/to/a new folder");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(mTestPath).exists("path/to/a new folder"));
  QVERIFY(not aBatch.isEmpty());

  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(not QDir(mTestPath).exists("path/to/a new folder"));
}
void FileOperationTest::test_touch_an_existed_folder_in_direct_path() {
  QVERIFY(QDir(mTestPath).exists("a"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(mTestPath, "a");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(mTestPath).exists("a"));
}
void FileOperationTest::test_touch_an_existed_folder_in_relative_path() {
  QVERIFY(QDir(mTestPath).exists("a/a1"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(mTestPath, "a/a1");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(mTestPath).exists("a/a1"));
}

void FileOperationTest::test_link_a_file() {
  QVERIFY2(QDir(mTestPath).exists("a.txt"), "Precondition not required.");

  FileOperatorType::RETURN_TYPE retEle = FileOperation::link(mTestPath, "a.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);

  QVERIFY(QDir(mTestPath).exists("a.txt"));
  QVERIFY(QDir(SystemPath::STARRED_PATH).exists("a.txt.lnk"));
  QVERIFY(not aBatch.isEmpty());

  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;
  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(mTestPath).exists("a.txt"));
  QVERIFY(not QDir(SystemPath::STARRED_PATH).exists("a.txt.lnk"));
}

void FileOperationTest::test_link_a_relative_file() {
  QVERIFY2(QDir(mTestPath).exists("a/a1.txt"), "Precondition not required.");

  FileOperatorType::RETURN_TYPE retEle = FileOperation::link(mTestPath, "a/a1.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);

  QVERIFY(QDir(mTestPath).exists("a/a1.txt"));
  QVERIFY(QDir(SystemPath::STARRED_PATH).exists("a/a1.txt.lnk"));
  QVERIFY(not aBatch.isEmpty());

  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;
  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(mTestPath).exists("a/a1.txt"));
  QVERIFY(not QDir(SystemPath::STARRED_PATH).exists("a/a1.txt.lnk"));
}

void FileOperationTest::test_rename_a_txt_To_A_TXT_Not_Exists() {
  const QString lowerCaseName = "a.txt";
  QVERIFY2(QDir(mTestPath).entryList(QDir::Filter::AllEntries).contains(lowerCaseName), "Environment should met first");

  const QString upperCaseName = "A.TXT";
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(mTestPath, lowerCaseName, mTestPath, upperCaseName);
  QCOMPARE(retEle.ret, ErrorCode::OK);
  QVERIFY2(QDir(mTestPath).entryList(QDir::Filter::AllEntries).contains(upperCaseName), "a.txt should be renamed to A.TXT");
}

void FileOperationTest::test_rename_b_txt_To_A_TXT_Already_Exists() {
  QVERIFY2(QDir(mTestPath).entryList(QDir::Filter::AllEntries).contains("b.txt"), "Environment should met first");
  QVERIFY2(QDir(mTestPath).entryList(QDir::Filter::AllEntries).contains("a.txt"), "Environment should met first");
  // rename b.txt -> A.TXT while {a.txt} already exist in destination
  const QString onlyCaseDifferName = "A.TXT";

  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(mTestPath, "b.txt", mTestPath, onlyCaseDifferName);
  QVERIFY2(retEle.ret != ErrorCode::OK, "should reject this rename and override operation");
}

void FileOperationTest::test_executer_return_if_any_cmd_failed() {
  QString inexistFolder("inexistFolder");
  QString existFile("b.txt");
  QVERIFY(!QDir(mTestPath).exists(inexistFolder));
  QVERIFY(QDir(mTestPath).exists(existFile));

  BATCH_COMMAND_LIST_TYPE aBatch, srcCommand;
  aBatch.append(ACMD{FILE_OPERATOR_E::RENAME,  //
                     {mTestPath, inexistFolder, mTestPath, "NewName inexistFolder"}});
  aBatch.append(ACMD{FILE_OPERATOR_E::RENAME,  //
                     {mTestPath, existFile, mTestPath, "NewName b.txt"}});

  FileOperatorType::RETURN_TYPE retEle = FileOperation::executer(aBatch, srcCommand);
  QCOMPARE(retEle.ret, ErrorCode::SRC_INEXIST);
  QVERIFY(!QDir(mTestPath).exists("NewName inexistFolder"));
  QVERIFY(!QDir(mTestPath).exists("NewName b.txt"));
  QVERIFY(QDir(mTestPath).exists(existFile));
}

#include "FileOperationTest.moc"
FileOperationTest g_FileOperationTest;
