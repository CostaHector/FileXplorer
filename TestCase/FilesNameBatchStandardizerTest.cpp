#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "Tools/FilesNameBatchStandardizer.h"
#include "pub/FileSystemTestSuite.h"

class FilesNameBatchStandardizerTest : public FileSystemTestSuite {
  Q_OBJECT

 public:
  FilesNameBatchStandardizerTest() : FileSystemTestSuite{"TestEnv_FilesNameBatchStandardizer", true, false} {  //
  }
  FilesNameBatchStandardizer frr;

 private slots:
  void init() {
    // - .C, A       B, Movie – C, A - .txt, ---C.txt
    m_rootHelper << FileSystemNode{"- .C"} << FileSystemNode{"A       B"} << FileSystemNode{"Movie – C"} << FileSystemNode{"A - .txt", false, ""} << FileSystemNode{"---C.txt", false, ""};
    // - .C
    // A       B{C [A{C (A.txt}}
    // Movie – C
    (m_rootHelper.GetSubHelper("A       B") << FileSystemNode{"C [A"}).GetSubHelper("C [A") << FileSystemNode{"C (A.txt"};
  }

  void test_files_batch_rename() {
    QVERIFY2(QDir(mTestPath).exists("A - .txt"), "Precondition not required.");
    QVERIFY2(QDir(mTestPath).exists("---C.txt"), "Precondition not required.");
    QVERIFY2(QDir(mTestPath).exists("A       B/C [A/C (A.txt"), "Precondition not required.");

    frr = FilesNameBatchStandardizer();
    frr(mTestPath);

    QVERIFY(QDir(mTestPath).exists("A.txt"));
    QVERIFY(QDir(mTestPath).exists("- C.txt"));
    QVERIFY(QDir(mTestPath).exists("A B/C - A/C - A.txt"));
  }
  void test_files_folders_rename() {
    QVERIFY2(QDir(mTestPath).exists("- .C"), "Precondition not required.");
    QVERIFY2(QDir(mTestPath).exists("A       B"), "Precondition not required.");
    QVERIFY2(QDir(mTestPath).exists("Movie – C"), "Precondition not required.");
    QVERIFY2(QDir(mTestPath).exists("A       B/C [A"), "Precondition not required.");

    frr = FilesNameBatchStandardizer();
    frr(mTestPath);

    QVERIFY(QDir(mTestPath).exists(".C"));
    QVERIFY(QDir(mTestPath).exists("A B"));
    QVERIFY(QDir(mTestPath).exists("Movie - C"));
    QVERIFY(QDir(mTestPath).exists("A B/C - A"));
  }
};

#include "FilesNameBatchStandardizerTest.moc"
FilesNameBatchStandardizerTest g_FilesNameBatchStandardizerTest;
