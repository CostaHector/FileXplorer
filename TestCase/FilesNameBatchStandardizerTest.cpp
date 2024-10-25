#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "Tools/FilesNameBatchStandardizer.h"
#include "pub/FileSystemRelatedTest.h"

class FilesNameBatchStandardizerTest : public FileSystemRelatedTest {
  Q_OBJECT

 public:
  FilesNameBatchStandardizerTest() : FileSystemRelatedTest{"TestEnv_FilesNameBatchStandardizer"} {}
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
    QVERIFY2(QDir(ROOT_DIR).exists("A - .txt"), "Precondition not required.");
    QVERIFY2(QDir(ROOT_DIR).exists("---C.txt"), "Precondition not required.");
    QVERIFY2(QDir(ROOT_DIR).exists("A       B/C [A/C (A.txt"), "Precondition not required.");

    frr = FilesNameBatchStandardizer();
    frr(ROOT_DIR);

    QVERIFY(QDir(ROOT_DIR).exists("A.txt"));
    QVERIFY(QDir(ROOT_DIR).exists("- C.txt"));
    QVERIFY(QDir(ROOT_DIR).exists("A B/C - A/C - A.txt"));
  }
  void test_files_folders_rename() {
    QVERIFY2(QDir(ROOT_DIR).exists("- .C"), "Precondition not required.");
    QVERIFY2(QDir(ROOT_DIR).exists("A       B"), "Precondition not required.");
    QVERIFY2(QDir(ROOT_DIR).exists("Movie – C"), "Precondition not required.");
    QVERIFY2(QDir(ROOT_DIR).exists("A       B/C [A"), "Precondition not required.");

    frr = FilesNameBatchStandardizer();
    frr(ROOT_DIR);

    QVERIFY(QDir(ROOT_DIR).exists(".C"));
    QVERIFY(QDir(ROOT_DIR).exists("A B"));
    QVERIFY(QDir(ROOT_DIR).exists("Movie - C"));
    QVERIFY(QDir(ROOT_DIR).exists("A B/C - A"));
  }
};

//QTEST_MAIN(FilesNameBatchStandardizerTest)
#include "FilesNameBatchStandardizerTest.moc"
