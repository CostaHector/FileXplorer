#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "pub/FileSystemRelatedTest.h"
#include "FileOperation/FileOperation.h"
#include "FileOperation/FileOperatorPub.h"

using namespace FileOperatorType;

class FileOperationTest : public FileSystemRelatedTest {
  Q_OBJECT

 public:
  FileOperationTest() : FileSystemRelatedTest{"TestEnv_FileOperation"} {}
 private slots:
  void initTestCase();
  void cleanupTestCase();

  void init();
  void test_file_remove();
  void test_folder_remove();

  void test_file_to_trashbin();

  void test_file_copy();

  void test_inexist_file_copy();
  void test_folder_copy_including_its_articles();
  void test_inexist_folder_copy_including_its_articles();
  void test_file_move_same_directory_unique_name();
  void test_file_move_same_directory_conflict_name();
  void test_folder_move_same_directory_unique_name();
  void test_folder_move_same_directory_conflict_name();
  void test_file_move_jump_directory_unique_name();
  void test_file_move_jump_directory_conflict_name();
  void test_folder_move_jump_directory_unique_name();
  void test_folder_move_jump_directory_conflict_name();
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

  void test_FileSystemHelper();
};

void FileOperationTest::init() {
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

void FileOperationTest::initTestCase() {}

void FileOperationTest::cleanupTestCase() {}

void FileOperationTest::test_FileSystemHelper() {
  cleanup();
  m_rootHelper << FileSystemNode{"directory", true, ""} << FileSystemNode{"mdfile.md", false, "waaaa"}
               << FileSystemNode{"readme.txt", false, "jahhhhhhhhhhhhhhhhaaaworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworld"};
  QCOMPARE(m_rootHelper.GetFileContent("mdfile.md"), "waaaa");
  const FileSystemNode expectNode{"directory", true, ""};
  QCOMPARE(m_rootHelper.GetNode("directory", false), expectNode);

  FileSystemHelper subHelper = m_rootHelper.GetSubHelper("directory");
  subHelper << FileSystemNode{".gitignore", false, "somecontents"};
  QStringList lvl1Items = QDir(ROOT_DIR, "", QDir::SortFlag::Name, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryList();
  QStringList expectItems;
  expectItems << "directory"
              << "mdfile.md"
              << "readme.txt";
  QCOMPARE(lvl1Items, expectItems);
  QCOMPARE(subHelper.GetFileContent(".gitignore"), "somecontents");

  QVERIFY(m_rootHelper.EraseFileSystemTree());
  QVERIFY(QDir(ROOT_DIR).isEmpty(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot));
}

void FileOperationTest::test_file_remove() {
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rmfile(ROOT_DIR, "a.txt");
  QCOMPARE(retEle.ret, ErrorCode::OK);

  QVERIFY(not QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(retEle.cmds.isEmpty());
}

void FileOperationTest::test_folder_remove() {
  QVERIFY(QDir(ROOT_DIR).exists("a"));
  QVERIFY(QDir(ROOT_DIR).exists("a/a1"));
  QVERIFY(QDir(ROOT_DIR).exists("a/a1.txt"));

  FileOperatorType::RETURN_TYPE retEle = FileOperation::rmdir(ROOT_DIR, "a");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(!QDir(ROOT_DIR).exists("a/a1.txt"));
  QVERIFY(!QDir(ROOT_DIR).exists("a/a1"));
  QVERIFY(!QDir(ROOT_DIR).exists("a"));
  QVERIFY(aBatch.isEmpty());  // remove cannot be recover
}

void FileOperationTest::test_file_to_trashbin() {
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::moveToTrash(ROOT_DIR, "a.txt");

  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(not QDir(ROOT_DIR).exists("a.txt"));

  QVERIFY(not aBatch.isEmpty());

  BATCH_COMMAND_LIST_TYPE srcCommand;
  BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;
  QVERIFY(recoverRet == 0);
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
}

void FileOperationTest::test_file_copy() {
  QString existFile("a.txt");
  QVERIFY(QDir(ROOT_DIR).exists(existFile));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(!QDir(ROOT_DIR).exists(QString("b/%1").arg(existFile)));

  FileOperatorType::RETURN_TYPE retEle = FileOperation::cpfile(ROOT_DIR, existFile, QString("%1/b").arg(ROOT_DIR));
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(ROOT_DIR).exists(QString("b/%1").arg(existFile)));

  QVERIFY(not aBatch.isEmpty());

  BATCH_COMMAND_LIST_TYPE srcCommand;
  BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};

  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(ROOT_DIR).exists(existFile));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(not QDir(ROOT_DIR).exists(QString("b/%1").arg(existFile)));
}

void FileOperationTest::test_inexist_file_copy() {
  QString inexistFileName("an inexist file blablablabla.txt");
  QVERIFY(!QDir(ROOT_DIR).exists(inexistFileName));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(!QDir(ROOT_DIR).exists(QString("b/%1").arg(inexistFileName)));

  FileOperatorType::RETURN_TYPE retEle = FileOperation::cpfile(ROOT_DIR, inexistFileName, QString("%1/b").arg(ROOT_DIR));
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QVERIFY(ret != ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(!QDir(ROOT_DIR).exists(inexistFileName));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(!QDir(ROOT_DIR).exists(QString("b/%1").arg(inexistFileName)));
}

void FileOperationTest::test_folder_copy_including_its_articles() {
  QString existFolder("a");
  QString subDir("a/a1");
  QString subFile("a/a1.txt");

  QVERIFY(QDir(ROOT_DIR).exists(existFolder));
  QVERIFY(QDir(ROOT_DIR).exists(subDir));
  QVERIFY(QDir(ROOT_DIR).exists(subFile));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(!QDir(ROOT_DIR).exists(QString("b/%1").arg(existFolder)));
  QVERIFY(!QDir(ROOT_DIR).exists(QString("b/%1").arg(subDir)));
  QVERIFY(!QDir(ROOT_DIR).exists(QString("b/%1").arg(subFile)));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::cpdir(ROOT_DIR, existFolder, QString("%1/b").arg(ROOT_DIR));

  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(ROOT_DIR).exists(existFolder));
  QVERIFY(QDir(ROOT_DIR).exists(subDir));
  QVERIFY(QDir(ROOT_DIR).exists(subFile));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(QDir(ROOT_DIR).exists(QString("b/%1").arg(subDir)));
  QVERIFY(QDir(ROOT_DIR).exists(QString("b/%1").arg(existFolder)));
  QVERIFY(QDir(ROOT_DIR).exists(QString("b/%1").arg(subFile)));
  QVERIFY(!aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(ROOT_DIR).exists(existFolder));
  QVERIFY(QDir(ROOT_DIR).exists(subDir));
  QVERIFY(QDir(ROOT_DIR).exists(subFile));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(!QDir(ROOT_DIR).exists(QString("b/%1").arg(existFolder)));
  QVERIFY(!QDir(ROOT_DIR).exists(QString("b/%1").arg(subDir)));
  QVERIFY(!QDir(ROOT_DIR).exists(QString("b/%1").arg(subFile)));
}

void FileOperationTest::test_inexist_folder_copy_including_its_articles() {
  QString inexistFolder("an inexist folder blablablabla");
  QVERIFY(not QDir(ROOT_DIR).exists(inexistFolder));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(not QDir(ROOT_DIR).exists(QString("b/%1").arg(inexistFolder)));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::cpdir(ROOT_DIR, inexistFolder, QString("%1/b").arg(ROOT_DIR));

  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QVERIFY(ret != ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(!QDir(ROOT_DIR).exists(inexistFolder));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(not QDir(ROOT_DIR).exists(QString("b/%1").arg(inexistFolder)));
}

void FileOperationTest::test_file_move_same_directory_unique_name() {
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(not QDir(ROOT_DIR).exists("a moved.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(ROOT_DIR, "a.txt", ROOT_DIR, "a moved.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(not QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(QDir(ROOT_DIR).exists("a moved.txt"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(not QDir(ROOT_DIR).exists("a moved.txt"));
}
void FileOperationTest::test_file_move_same_directory_conflict_name() {
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(QDir(ROOT_DIR).exists("b.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(ROOT_DIR, "a.txt", ROOT_DIR, "b.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QVERIFY(ret != ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(QDir(ROOT_DIR).exists("b.txt"));
}

void FileOperationTest::test_folder_move_same_directory_unique_name() {
  QVERIFY(QDir(ROOT_DIR).exists("a"));
  QVERIFY(not QDir(ROOT_DIR).exists("a moved"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(ROOT_DIR, "a", ROOT_DIR, "a moved");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(not QDir(ROOT_DIR).exists("a"));
  QVERIFY(QDir(ROOT_DIR).exists("a moved"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(ROOT_DIR).exists("a"));
  QVERIFY(not QDir(ROOT_DIR).exists("a moved"));
}
void FileOperationTest::test_folder_move_same_directory_conflict_name() {
  QVERIFY(QDir(ROOT_DIR).exists("a"));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(ROOT_DIR, "a", ROOT_DIR, "b");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QVERIFY(ret != ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(ROOT_DIR).exists("a"));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
}
void FileOperationTest::test_file_move_jump_directory_unique_name() {
  const QString& TO = QDir(ROOT_DIR).absoluteFilePath("b");
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(not QDir(TO).exists("path/to/a moved.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(ROOT_DIR, "a.txt", TO, "path/to/a moved.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(not QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(QDir(TO).exists("path/to/a moved.txt"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(not QDir(TO).exists("path/to/a moved.txt"));
}
void FileOperationTest::test_file_move_jump_directory_conflict_name() {
  const QString& TO = QDir(ROOT_DIR).absoluteFilePath("b");
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(QDir(TO).exists("b1/b2.txt"));  // conflict here;
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(ROOT_DIR, "a.txt", TO, "b1/b2.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QVERIFY(ret != ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(QDir(TO).exists("b1/b2.txt"));
}
void FileOperationTest::test_folder_move_jump_directory_unique_name() {
  const QString& TO = QDir(ROOT_DIR).absoluteFilePath("b");
  QVERIFY(QDir(ROOT_DIR).exists("a"));
  QVERIFY(QDir(ROOT_DIR).exists("a/a1"));
  QVERIFY(QDir(ROOT_DIR).exists("a/a1.txt"));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(not QDir(TO).exists("path/to/a moved"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(ROOT_DIR, "a", TO, "path/to/a moved");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(not QDir(ROOT_DIR).exists("a"));
  QVERIFY(not QDir(ROOT_DIR).exists("a/a1"));
  QVERIFY(not QDir(ROOT_DIR).exists("a/a1.txt"));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(QDir(TO).exists("path/to/a moved"));
  QVERIFY(QDir(TO).exists("path/to/a moved/a1"));
  QVERIFY(QDir(TO).exists("path/to/a moved/a1.txt"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(ROOT_DIR).exists("a"));
  QVERIFY(QDir(ROOT_DIR).exists("a/a1"));
  QVERIFY(QDir(ROOT_DIR).exists("a/a1.txt"));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(not QDir(TO).exists("path/to/a moved"));
}
void FileOperationTest::test_folder_move_jump_directory_conflict_name() {
  const QString& TO = QDir(ROOT_DIR).absoluteFilePath("b");
  QVERIFY(QDir(ROOT_DIR).exists("a"));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(QDir(TO).exists("b1/b2"));  // conflict here;
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(ROOT_DIR, "a", TO, "b1/b2");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QVERIFY(ret != ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(ROOT_DIR).exists("a"));
  QVERIFY(QDir(ROOT_DIR).exists("b"));
  QVERIFY(QDir(TO).exists("b1/b2"));
}
void FileOperationTest::test_touch_a_json_file_in_direct_path() {
  QVERIFY(not QDir(ROOT_DIR).exists("a new json file.json"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(ROOT_DIR, "a new json file.json");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(ROOT_DIR).exists("a new json file.json"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(not QDir(ROOT_DIR).exists("a new json file.json"));
}
void FileOperationTest::test_touch_a_json_file_in_relative_path() {
  QVERIFY(not QDir(ROOT_DIR).exists("path/to/a new json file.json"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(ROOT_DIR, "path/to/a new json file.json");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(ROOT_DIR).exists("path/to/a new json file.json"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(not QDir(ROOT_DIR).exists("path/to/a new json file.json"));
}
void FileOperationTest::test_touch_an_existed_txt_file_in_direct_path() {
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(ROOT_DIR, "a.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
}
void FileOperationTest::test_touch_an_existed_txt_file_in_relative_path() {
  QVERIFY(QDir(ROOT_DIR).exists("a/a1.txt"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(ROOT_DIR, "a/a1.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(ROOT_DIR).exists("a/a1.txt"));
}
void FileOperationTest::test_touch_a_folder_in_direct_path() {
  QVERIFY(not QDir(ROOT_DIR).exists("a new folder"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(ROOT_DIR, "a new folder");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(ROOT_DIR).exists("a new folder"));
  QVERIFY(not aBatch.isEmpty());
  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(not QDir(ROOT_DIR).exists("a new folder"));
}
void FileOperationTest::test_touch_a_folder_in_relative_path() {
  QVERIFY(not QDir(ROOT_DIR).exists("path/to/a new folder"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(ROOT_DIR, "path/to/a new folder");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(QDir(ROOT_DIR).exists("path/to/a new folder"));
  QVERIFY(not aBatch.isEmpty());

  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;

  QCOMPARE(recoverRet, 0);
  QVERIFY(not QDir(ROOT_DIR).exists("path/to/a new folder"));
}
void FileOperationTest::test_touch_an_existed_folder_in_direct_path() {
  QVERIFY(QDir(ROOT_DIR).exists("a"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(ROOT_DIR, "a");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(ROOT_DIR).exists("a"));
}
void FileOperationTest::test_touch_an_existed_folder_in_relative_path() {
  QVERIFY(QDir(ROOT_DIR).exists("a/a1"));
  FileOperatorType::RETURN_TYPE retEle = FileOperation::touch(ROOT_DIR, "a/a1");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;
  QCOMPARE(ret, ErrorCode::OK);
  QVERIFY(aBatch.isEmpty());
  QVERIFY(QDir(ROOT_DIR).exists("a/a1"));
}

void FileOperationTest::test_link_a_file() {
  QVERIFY2(QDir(ROOT_DIR).exists("a.txt"), "Precondition not required.");

  FileOperatorType::RETURN_TYPE retEle = FileOperation::link(ROOT_DIR, "a.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);

  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(QDir(SystemPath::starredPath).exists("a.txt.lnk"));
  QVERIFY(not aBatch.isEmpty());

  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;
  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(ROOT_DIR).exists("a.txt"));
  QVERIFY(not QDir(SystemPath::starredPath).exists("a.txt.lnk"));
}

void FileOperationTest::test_link_a_relative_file() {
  QVERIFY2(QDir(ROOT_DIR).exists("a/a1.txt"), "Precondition not required.");

  FileOperatorType::RETURN_TYPE retEle = FileOperation::link(ROOT_DIR, "a/a1.txt");
  auto ret = retEle.ret;
  auto aBatch = retEle.cmds;

  QCOMPARE(ret, ErrorCode::OK);

  QVERIFY(QDir(ROOT_DIR).exists("a/a1.txt"));
  QVERIFY(QDir(SystemPath::starredPath).exists("a/a1.txt.lnk"));
  QVERIFY(not aBatch.isEmpty());

  BATCH_COMMAND_LIST_TYPE srcCommand;
  const BATCH_COMMAND_LIST_TYPE reversedaBatch{aBatch.crbegin(), aBatch.crend()};
  const auto& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
  const auto recoverRet = exeRetEle.ret;
  QCOMPARE(recoverRet, 0);
  QVERIFY(QDir(ROOT_DIR).exists("a/a1.txt"));
  QVERIFY(not QDir(SystemPath::starredPath).exists("a/a1.txt.lnk"));
}

void FileOperationTest::test_rename_a_txt_To_A_TXT_Not_Exists() {
  const QString lowerCaseName = "a.txt";
  QVERIFY2(QDir(ROOT_DIR).entryList(QDir::Filter::AllEntries).contains(lowerCaseName), "Environment should met first");

  const QString upperCaseName = "A.TXT";
  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(ROOT_DIR, lowerCaseName, ROOT_DIR, upperCaseName);
  QCOMPARE(retEle.ret, ErrorCode::OK);
  QVERIFY2(QDir(ROOT_DIR).entryList(QDir::Filter::AllEntries).contains(upperCaseName), "a.txt should be renamed to A.TXT");
}

void FileOperationTest::test_rename_b_txt_To_A_TXT_Already_Exists() {
  QVERIFY2(QDir(ROOT_DIR).entryList(QDir::Filter::AllEntries).contains("b.txt"), "Environment should met first");
  QVERIFY2(QDir(ROOT_DIR).entryList(QDir::Filter::AllEntries).contains("a.txt"), "Environment should met first");
  // rename b.txt -> A.TXT while {a.txt} already exist in destination
  const QString onlyCaseDifferName = "A.TXT";

  FileOperatorType::RETURN_TYPE retEle = FileOperation::rename(ROOT_DIR, "b.txt", ROOT_DIR, onlyCaseDifferName);
  QVERIFY2(retEle.ret != ErrorCode::OK, "should reject this rename and override operation");
}

//QTEST_MAIN(FileOperationTest)
#include "FileOperationTest.moc"
