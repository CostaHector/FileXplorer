#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "Tools/RedundantFolderRemove.h"

const QString EMPTY_TEST_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_RedundantItemRemove/EmptyFolder");
const QString KEYWORD_TEST_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_RedundantItemRemove/RedundantKeyword");

class RedundantItemsRemoverTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}

  void init() {
    QVERIFY2(QDir(EMPTY_TEST_DIR).exists(), "precondition. path should exist");

    QVERIFY2(QDir(KEYWORD_TEST_DIR).exists(), "precondition. path should exist");
  }
  void cleanup() {}

  void test_recycleEmptyFolder() {
    // DeleteRandomEmptyFolder1, DeleteRandomEmptyFolder2, KeepNonEmptyFolder
    QCOMPARE(QDir(EMPTY_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).size(), 3);
    EmptyFolderRemove efr;
    int recycleCmdCnt = efr(EMPTY_TEST_DIR);
    QCOMPARE(recycleCmdCnt, 2);
  }
  void test_recycleRedundantItemByKeyWord() {
    // Delete Movie Falcon, Keep KristenBjorn imgs, Keep RagingStallion Movie
    QCOMPARE(QDir(KEYWORD_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).size(), 3);
    int recycleCmdCnt = -1;

    RedundantItemsRemoverByKeyword rirbkFalcon{"Falcon"};
    recycleCmdCnt = rirbkFalcon(KEYWORD_TEST_DIR);
    QCOMPARE(recycleCmdCnt, 1);

    RedundantItemsRemoverByKeyword rirbkRaging{"RagingStallion"};
    recycleCmdCnt = rirbkRaging(KEYWORD_TEST_DIR);
    QCOMPARE(recycleCmdCnt, 0);

    RedundantItemsRemoverByKeyword rirbkKristen{"KristenBjorn"};
    recycleCmdCnt = rirbkKristen(KEYWORD_TEST_DIR);
    QCOMPARE(recycleCmdCnt, 0);
  }


};

QTEST_MAIN(RedundantItemsRemoverTest)
#include "RedundantItemsRemoverTest.moc"
