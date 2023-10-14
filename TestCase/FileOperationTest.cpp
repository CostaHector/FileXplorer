#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "FileOperation/FileOperation.h"

#include <QFileInfo>
#include "PublicTool.h"

class FileOperationTest : public QObject
{
    Q_OBJECT

public:


private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testSplitDirName();
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
};


const QString TEST_SRC_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_FileOperation/DONT_CHANGE");
const QString TEST_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_FileOperation/COPY_REMOVABLE");


void FileOperationTest::init(){
    if (QDir(TEST_DIR).exists()){
        QDir(TEST_DIR).removeRecursively();
    }
    auto ret = PublicTool::copyDirectoryFiles(TEST_SRC_DIR, TEST_DIR);
    assert(ret); // should copied ok
}
void FileOperationTest::cleanup(){
    if (QDir(TEST_DIR).exists()){
        QDir(TEST_DIR).removeRecursively();
    }
}

void FileOperationTest::initTestCase()
{

}

void FileOperationTest::cleanupTestCase()
{

}


void FileOperationTest::testSplitDirName()
{
    const QPair<QString, QString>& filePath = FileOperation::SplitDirName("C:/home/to/file.txt");
    QCOMPARE(filePath.first, "C:/home/to");
    QCOMPARE(filePath.second, "file.txt");

    const QPair<QString, QString>& fileInRoot = FileOperation::SplitDirName("C:/file.txt");
    QCOMPARE(fileInRoot.first, "C:/");
    QCOMPARE(fileInRoot.second, "file.txt");

    const QPair<QString, QString>& filePathLinux = FileOperation::SplitDirName("/home/to/file.txt");
    QCOMPARE(filePathLinux.first, "/home/to");
    QCOMPARE(filePathLinux.second, "file.txt");

    const QPair<QString, QString>& filePathInRootLinux = FileOperation::SplitDirName("/home");
    QCOMPARE(filePathInRootLinux.first, "/");
    QCOMPARE(filePathInRootLinux.second, "home");

}

void FileOperationTest::test_file_remove()
{
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
    FileOperation::RETURN_TYPE retEle = FileOperation::rmfile(TEST_DIR, "a.txt");
    QCOMPARE(retEle.first, ErrorCode::OK);
    QVERIFY(not QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(retEle.second.isEmpty());
}

void FileOperationTest::test_folder_remove() {
    QVERIFY(QDir(TEST_DIR).exists("a"));
    QVERIFY(QDir(TEST_DIR).exists("a/a1"));
    QVERIFY(QDir(TEST_DIR).exists("a/a1.txt"));
    FileOperation::RETURN_TYPE retEle = FileOperation::rmdir(TEST_DIR, "a");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(not QDir(TEST_DIR).exists("a/a1.txt"));
    QVERIFY(not QDir(TEST_DIR).exists("a/a1"));
    QVERIFY(not QDir(TEST_DIR).exists("a"));
    QVERIFY(aBatch.isEmpty()); // remove cannot be recover
}

void FileOperationTest::test_file_to_trashbin()
{
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));

    FileOperation::RETURN_TYPE retEle = FileOperation::moveToTrash(TEST_DIR, "a.txt");

    auto ret = retEle.first;
    auto aBatch = retEle.second;

    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(not QDir(TEST_DIR).exists("a.txt"));

    QVERIFY(not aBatch.isEmpty());

    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());

    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;
    QVERIFY(recoverRet);
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
}

void FileOperationTest::test_file_copy(){
    QString existFile("a.txt");
    QVERIFY(QDir(TEST_DIR).exists(existFile));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(not QDir(TEST_DIR).exists(QString("b/%1").arg(existFile)));

    FileOperation::RETURN_TYPE retEle = FileOperation::cpfile(TEST_DIR, existFile, QString("%1/b").arg(TEST_DIR));
    auto ret = retEle.first;
    auto aBatch = retEle.second;


    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(QDir(TEST_DIR).exists(QString("b/%1").arg(existFile)));

    QVERIFY(not aBatch.isEmpty());

    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());

    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;

    QVERIFY(recoverRet);
    QVERIFY(QDir(TEST_DIR).exists(existFile));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(not QDir(TEST_DIR).exists(QString("b/%1").arg(existFile)));
}


void FileOperationTest::test_inexist_file_copy(){
    QString inexistFileName("an inexist file blablablabla.txt");
    QVERIFY(not QDir(TEST_DIR).exists(inexistFileName));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(not QDir(TEST_DIR).exists(QString("b/%1").arg(inexistFileName)));
    FileOperation::RETURN_TYPE retEle = FileOperation::cpfile(TEST_DIR, inexistFileName, QString("%1/b").arg(TEST_DIR));
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QVERIFY(ret != ErrorCode::OK);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(not QDir(TEST_DIR).exists(inexistFileName));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(not QDir(TEST_DIR).exists(QString("b/%1").arg(inexistFileName)));
}

void FileOperationTest::test_folder_copy_including_its_articles(){
    QString existFile("a");
    QString subDir("a/a1");
    QString subFile("a/a1.txt");
    QVERIFY(QDir(TEST_DIR).exists(existFile));
    QVERIFY(QDir(TEST_DIR).exists(subDir));
    QVERIFY(QDir(TEST_DIR).exists(subFile));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(not QDir(TEST_DIR).exists(QString("b/%1").arg(existFile)));
    QVERIFY(not QDir(TEST_DIR).exists(QString("b/%1").arg(subDir)));
    QVERIFY(not QDir(TEST_DIR).exists(QString("b/%1").arg(subFile)));
    FileOperation::RETURN_TYPE retEle = FileOperation::cpdir(TEST_DIR, existFile, QString("%1/b").arg(TEST_DIR));

    auto ret = retEle.first;
    auto aBatch = retEle.second;

    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(QDir(TEST_DIR).exists(existFile));
    QVERIFY(QDir(TEST_DIR).exists(subDir));
    QVERIFY(QDir(TEST_DIR).exists(subFile));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(QDir(TEST_DIR).exists(QString("b/%1").arg(subDir)));
    QVERIFY(QDir(TEST_DIR).exists(QString("b/%1").arg(existFile)));
    QVERIFY(QDir(TEST_DIR).exists(QString("b/%1").arg(subFile)));
    QVERIFY(not aBatch.isEmpty());
    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());
    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;

    QVERIFY(recoverRet);
    QVERIFY(QDir(TEST_DIR).exists(existFile));
    QVERIFY(QDir(TEST_DIR).exists(subDir));
    QVERIFY(QDir(TEST_DIR).exists(subFile));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(not QDir(TEST_DIR).exists(QString("b/%1").arg(existFile)));
    QVERIFY(not QDir(TEST_DIR).exists(QString("b/%1").arg(subDir)));
    QVERIFY(not QDir(TEST_DIR).exists(QString("b/%1").arg(subFile)));
}
void FileOperationTest::test_inexist_folder_copy_including_its_articles(){
    QString inexistFolder("an inexist folder blablablabla");
    QVERIFY(not QDir(TEST_DIR).exists(inexistFolder));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(not QDir(TEST_DIR).exists(QString("b/%1").arg(inexistFolder)));
    FileOperation::RETURN_TYPE retEle = FileOperation::cpdir(TEST_DIR, inexistFolder, QString("%1/b").arg(TEST_DIR));

    auto ret = retEle.first;
    auto aBatch = retEle.second;


    QVERIFY(ret != ErrorCode::OK);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(not QDir(TEST_DIR).exists(inexistFolder));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(not QDir(TEST_DIR).exists(QString("b/%1").arg(inexistFolder)));
}
void FileOperationTest::test_file_move_same_directory_unique_name(){
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(not QDir(TEST_DIR).exists("a moved.txt"));
    FileOperation::RETURN_TYPE retEle = FileOperation::rename(TEST_DIR, "a.txt", TEST_DIR, "a moved.txt");
    auto ret = retEle.first;
    auto aBatch = retEle.second;

    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(not QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(QDir(TEST_DIR).exists("a moved.txt"));
    QVERIFY(not aBatch.isEmpty());
    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());
    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;

    QVERIFY(recoverRet);
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(not QDir(TEST_DIR).exists("a moved.txt"));
}
void FileOperationTest::test_file_move_same_directory_conflict_name(){
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(QDir(TEST_DIR).exists("b.txt"));
    FileOperation::RETURN_TYPE retEle = FileOperation::rename(TEST_DIR, "a.txt", TEST_DIR, "b.txt");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QVERIFY(ret != ErrorCode::OK);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(QDir(TEST_DIR).exists("b.txt"));
}
void FileOperationTest::test_folder_move_same_directory_unique_name(){
    QVERIFY(QDir(TEST_DIR).exists("a"));
    QVERIFY(not QDir(TEST_DIR).exists("a moved"));
    FileOperation::RETURN_TYPE retEle = FileOperation::rename(TEST_DIR, "a", TEST_DIR, "a moved");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(not QDir(TEST_DIR).exists("a"));
    QVERIFY(QDir(TEST_DIR).exists("a moved"));
    QVERIFY(not aBatch.isEmpty());
    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());
    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;

    QVERIFY(recoverRet);
    QVERIFY(QDir(TEST_DIR).exists("a"));
    QVERIFY(not QDir(TEST_DIR).exists("a moved"));
}
void FileOperationTest::test_folder_move_same_directory_conflict_name(){
    QVERIFY(QDir(TEST_DIR).exists("a"));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    FileOperation::RETURN_TYPE retEle = FileOperation::rename(TEST_DIR, "a", TEST_DIR, "b");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QVERIFY(ret != ErrorCode::OK);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(QDir(TEST_DIR).exists("a"));
    QVERIFY(QDir(TEST_DIR).exists("b"));
}
void FileOperationTest::test_file_move_jump_directory_unique_name(){
    const QString& TO = QDir(TEST_DIR).absoluteFilePath("b");
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(not QDir(TO).exists("path/to/a moved.txt"));
    FileOperation::RETURN_TYPE retEle = FileOperation::rename(TEST_DIR, "a.txt", TO, "path/to/a moved.txt");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(not QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(QDir(TO).exists("path/to/a moved.txt"));
    QVERIFY(not aBatch.isEmpty());
    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());
    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;

    QVERIFY(recoverRet);
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(not QDir(TO).exists("path/to/a moved.txt"));
}
void FileOperationTest::test_file_move_jump_directory_conflict_name(){
    const QString& TO = QDir(TEST_DIR).absoluteFilePath("b");
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(QDir(TO).exists("b1/b2.txt")); // conflict here;
    FileOperation::RETURN_TYPE retEle = FileOperation::rename(TEST_DIR, "a.txt", TO, "b1/b2.txt");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QVERIFY(ret != ErrorCode::OK);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(QDir(TO).exists("b1/b2.txt"));
}
void FileOperationTest::test_folder_move_jump_directory_unique_name(){
    const QString& TO = QDir(TEST_DIR).absoluteFilePath("b");
    QVERIFY(QDir(TEST_DIR).exists("a"));
    QVERIFY(QDir(TEST_DIR).exists("a/a1"));
    QVERIFY(QDir(TEST_DIR).exists("a/a1.txt"));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(not QDir(TO).exists("path/to/a moved"));
    FileOperation::RETURN_TYPE retEle = FileOperation::rename(TEST_DIR, "a", TO, "path/to/a moved");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(not QDir(TEST_DIR).exists("a"));
    QVERIFY(not QDir(TEST_DIR).exists("a/a1"));
    QVERIFY(not QDir(TEST_DIR).exists("a/a1.txt"));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(QDir(TO).exists("path/to/a moved"));
    QVERIFY(QDir(TO).exists("path/to/a moved/a1"));
    QVERIFY(QDir(TO).exists("path/to/a moved/a1.txt"));
    QVERIFY(not aBatch.isEmpty());
    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());
    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;

    QVERIFY(recoverRet);
    QVERIFY(QDir(TEST_DIR).exists("a"));
    QVERIFY(QDir(TEST_DIR).exists("a/a1"));
    QVERIFY(QDir(TEST_DIR).exists("a/a1.txt"));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(not QDir(TO).exists("path/to/a moved"));
}
void FileOperationTest::test_folder_move_jump_directory_conflict_name(){
    const QString& TO = QDir(TEST_DIR).absoluteFilePath("b");
    QVERIFY(QDir(TEST_DIR).exists("a"));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(QDir(TO).exists("b1/b2"));  // conflict here;
    FileOperation::RETURN_TYPE retEle = FileOperation::rename(TEST_DIR, "a", TO, "b1/b2");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QVERIFY(ret != ErrorCode::OK);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(QDir(TEST_DIR).exists("a"));
    QVERIFY(QDir(TEST_DIR).exists("b"));
    QVERIFY(QDir(TO).exists("b1/b2"));
}
void FileOperationTest::test_touch_a_json_file_in_direct_path(){
    QVERIFY(not QDir(TEST_DIR).exists("a new json file.json"));
    FileOperation::RETURN_TYPE retEle = FileOperation::touch(TEST_DIR, "a new json file.json");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(QDir(TEST_DIR).exists("a new json file.json"));
    QVERIFY(not aBatch.isEmpty());
    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());
    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;

    QVERIFY(recoverRet);
    QVERIFY(not QDir(TEST_DIR).exists("a new json file.json"));
}
void FileOperationTest::test_touch_a_json_file_in_relative_path(){
    QVERIFY(not QDir(TEST_DIR).exists("path/to/a new json file.json"));
    FileOperation::RETURN_TYPE retEle = FileOperation::touch(TEST_DIR, "path/to/a new json file.json");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(QDir(TEST_DIR).exists("path/to/a new json file.json"));
    QVERIFY(not aBatch.isEmpty());
    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());
    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;

    QVERIFY(recoverRet);
    QVERIFY(not QDir(TEST_DIR).exists("path/to/a new json file.json"));
}
void FileOperationTest::test_touch_an_existed_txt_file_in_direct_path(){
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
    FileOperation::RETURN_TYPE retEle = FileOperation::touch(TEST_DIR, "a.txt");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
}
void FileOperationTest::test_touch_an_existed_txt_file_in_relative_path(){
    QVERIFY(QDir(TEST_DIR).exists("a/a1.txt"));
    FileOperation::RETURN_TYPE retEle = FileOperation::touch(TEST_DIR, "a/a1.txt");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(QDir(TEST_DIR).exists("a/a1.txt"));
}
void FileOperationTest::test_touch_a_folder_in_direct_path(){
    QVERIFY(not QDir(TEST_DIR).exists("a new folder"));
    FileOperation::RETURN_TYPE retEle = FileOperation::touch(TEST_DIR, "a new folder");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(QDir(TEST_DIR).exists("a new folder"));
    QVERIFY(not aBatch.isEmpty());
    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());
    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;

    QVERIFY(recoverRet);
    QVERIFY(not QDir(TEST_DIR).exists("a new folder"));
}
void FileOperationTest::test_touch_a_folder_in_relative_path(){
    QVERIFY(not QDir(TEST_DIR).exists("path/to/a new folder"));
    FileOperation::RETURN_TYPE retEle = FileOperation::touch(TEST_DIR, "path/to/a new folder");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(QDir(TEST_DIR).exists("path/to/a new folder"));
    QVERIFY(not aBatch.isEmpty());
    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());
    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;

    QVERIFY(recoverRet);
    QVERIFY(not QDir(TEST_DIR).exists("path/to/a new folder"));
}
void FileOperationTest::test_touch_an_existed_folder_in_direct_path(){
    QVERIFY(QDir(TEST_DIR).exists("a"));
    FileOperation::RETURN_TYPE retEle = FileOperation::touch(TEST_DIR, "a");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(QDir(TEST_DIR).exists("a"));
}
void FileOperationTest::test_touch_an_existed_folder_in_relative_path(){
    QVERIFY(QDir(TEST_DIR).exists("a/a1"));
    FileOperation::RETURN_TYPE retEle = FileOperation::touch(TEST_DIR, "a/a1");
    auto ret = retEle.first;
    auto aBatch = retEle.second;
    QCOMPARE(ret, ErrorCode::OK);
    QVERIFY(aBatch.isEmpty());
    QVERIFY(QDir(TEST_DIR).exists("a/a1"));
}

void FileOperationTest::test_link_a_file(){
    QVERIFY2(QDir(TEST_DIR).exists("a.txt"), "Precondition not required.");

    FileOperation::RETURN_TYPE retEle = FileOperation::link(TEST_DIR, "a.txt");
    auto ret = retEle.first;
    auto aBatch = retEle.second;

    QCOMPARE(ret, ErrorCode::OK);

    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(QDir(SystemPath::starredPath).exists("a.txt.lnk"));
    QVERIFY(not aBatch.isEmpty());

    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());
    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;
    QVERIFY(recoverRet);
    QVERIFY(QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(not QDir(SystemPath::starredPath).exists("a.txt.lnk"));
}

void FileOperationTest::test_link_a_relative_file(){
    QVERIFY2(QDir(TEST_DIR).exists("a/a1.txt"), "Precondition not required.");

    FileOperation::RETURN_TYPE retEle = FileOperation::link(TEST_DIR, "a/a1.txt");
    auto ret = retEle.first;
    auto aBatch = retEle.second;

    QCOMPARE(ret, ErrorCode::OK);

    QVERIFY(QDir(TEST_DIR).exists("a/a1.txt"));
    QVERIFY(QDir(SystemPath::starredPath).exists("a/a1.txt.lnk"));
    QVERIFY(not aBatch.isEmpty());

    QList<QStringList> srcCommand;
    const auto& reversedaBatch = decltype(srcCommand)(aBatch.crbegin(), aBatch.crend());
    const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(reversedaBatch, srcCommand);
    const auto recoverRet = exeRetEle.first;
    QVERIFY(recoverRet);
    QVERIFY(QDir(TEST_DIR).exists("a/a1.txt"));
    QVERIFY(not QDir(SystemPath::starredPath).exists("a/a1.txt.lnk"));
}

QTEST_MAIN(FileOperationTest)

#include "FileOperationTest.moc"
