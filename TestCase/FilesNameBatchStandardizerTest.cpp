#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "Tools/FilesNameBatchStandardizer.h"
#include "PublicTool.h"

const QString TEST_SRC_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_FilesNameBatchStandardizer/DONT_CHANGE");
const QString TEST_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_FilesNameBatchStandardizer/COPY_REMOVABLE");

class FilesNameBatchStandardizerTest : public QObject
{
    Q_OBJECT
public:
    FilesNameBatchStandardizer frr;
private slots:
    void init(){
        if (QDir(TEST_DIR).exists()){
            QDir(TEST_DIR).removeRecursively();
        }
        auto ret = PublicTool::copyDirectoryFiles(TEST_SRC_DIR, TEST_DIR);
        assert(ret); // should copied ok
    }
    void cleanup(){
        if (QDir(TEST_DIR).exists()){
            QDir(TEST_DIR).removeRecursively();
        }
    }
    void test_files_batch_rename(){
        QVERIFY2(QDir(TEST_DIR).exists("A - .txt"), "Precondition not required.");
        QVERIFY2(QDir(TEST_DIR).exists("---C.txt"), "Precondition not required.");
        QVERIFY2(QDir(TEST_DIR).exists("A       B/C [A/C (A.txt"), "Precondition not required.");

        frr = FilesNameBatchStandardizer();
        frr(TEST_DIR);

        QVERIFY(QDir(TEST_DIR).exists("A.txt"));
        QVERIFY(QDir(TEST_DIR).exists("- C.txt"));
        QVERIFY(QDir(TEST_DIR).exists("A B/C - A/C - A.txt"));
    }
    void test_files_folders_rename(){
        QVERIFY2(QDir(TEST_DIR).exists("- .C"), "Precondition not required.");
        QVERIFY2(QDir(TEST_DIR).exists("A       B"), "Precondition not required.");
        QVERIFY2(QDir(TEST_DIR).exists("Movie – C"), "Precondition not required.");
        QVERIFY2(QDir(TEST_DIR).exists("A       B/C [A"), "Precondition not required.");

        frr = FilesNameBatchStandardizer();
        frr(TEST_DIR);

        QVERIFY(QDir(TEST_DIR).exists(".C"));
        QVERIFY(QDir(TEST_DIR).exists("A B"));
        QVERIFY(QDir(TEST_DIR).exists("Movie - C"));
        QVERIFY(QDir(TEST_DIR).exists("A B/C - A"));
    }
};

//QTEST_MAIN(FilesNameBatchStandardizerTest)
#include "FilesNameBatchStandardizerTest.moc"
