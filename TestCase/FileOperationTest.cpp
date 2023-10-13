#include <QtTest>
#include <QCoreApplication>

// add necessary includes here


class FileOperationTest : public QObject
{
    Q_OBJECT

public:
    FileOperationTest();
    ~FileOperationTest();


    static bool copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist=false) {
        QDir sourceDir(fromDir);
        QDir targetDir(toDir);
        if(!targetDir.exists()){    /* if directory don't exists, build it */
            if(!targetDir.mkdir(targetDir.absolutePath()))
                return false;
        }

        QFileInfoList fileInfoList = sourceDir.entryInfoList();
        for(const QFileInfo& fileInfo:fileInfoList){
            if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
                continue;

            if(fileInfo.isDir()){    /* if it is directory, copy recursively*/
                if(copyDirectoryFiles(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()), coverFileIfExist)){
                    continue;
                }
                return false;
            }
            /* if coverFileIfExist == true, remove old file first */

            if (targetDir.exists(fileInfo.fileName())){
                if(coverFileIfExist){
                    targetDir.remove(fileInfo.fileName());
                    qDebug("%s/%s is covered by file under [%s]", targetDir.absolutePath().toStdString().c_str(), fileInfo.fileName().toStdString().c_str(), fromDir.toStdString().c_str());
                }else{
                    qDebug("%s/[%s] was kept", targetDir.absolutePath().toStdString().c_str(), fileInfo.fileName().toStdString().c_str());
                }
            }
            // files copy
            if(!QFile::copy(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))){
                return false;
            }
        }
        return true;
    }

private slots:
    void initTestCase();
    void cleanupTestCase();
    void toUpper();
    void capitalizer();
    void testSplitDirName();

    void test_file_remove();

};


const QString TEST_SRC_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_FileOperation/DONT_CHANGE");
const QString TEST_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_FileOperation/COPY_REMOVABLE");


FileOperationTest::FileOperationTest()
{
    if (QDir(TEST_DIR).exists()){
        QDir(TEST_DIR).removeRecursively();
    }
    auto ret = copyDirectoryFiles(TEST_SRC_DIR, TEST_DIR);
    assert(ret); // should copied ok
}

FileOperationTest::~FileOperationTest()
{
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

void FileOperationTest::toUpper()
{
    QString s("abc123ABC!@#");
    QCOMPARE(s.toUpper(), QString("ABC123ABC!@#"));
}

void FileOperationTest::capitalizer(){
    QCOMPARE(1, 1);
}

#include <QFileInfo>
#include "FileOperation/FileOperation.h"

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
    FileOperation::RETURN_TYPE retPr = FileOperation::rmfile(TEST_DIR, "a.txt");
    QCOMPARE(retPr.first, ErrorCode::OK);
    QVERIFY(not QDir(TEST_DIR).exists("a.txt"));
    QVERIFY(retPr.second.isEmpty());
}

QTEST_MAIN(FileOperationTest)

#include "FileOperationTest.moc"
