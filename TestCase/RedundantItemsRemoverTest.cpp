#include <QCoreApplication>
#include <QtTest>

#include "TestCase/pub/BeginToExposePrivateMember.h"
#include "Tools/RedundantFolderRemove.h"
#include "TestCase/pub/EndToExposePrivateMember.h"

const QString EMPTY_TEST_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_RedundantItemRemove/EmptyFolder");
const QString KEYWORD_TEST_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_RedundantItemRemove/RedundantKeyword");
const QString REDUN_PARENT_TEST_DIR =
    QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_RedundantItemRemove/RedundantParentFolder");

class RedundantItemsRemoverTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}

  void init() {
    QVERIFY2(QDir(EMPTY_TEST_DIR).exists(), "precondition. path should exist");
    QVERIFY2(QDir(KEYWORD_TEST_DIR).exists(), "precondition. path should exist");
    QVERIFY2(QDir(REDUN_PARENT_TEST_DIR).exists(), "precondition. path should exist");
  }
  void cleanup() {}

  void test_recycleEmptyFolder() {
    // DeleteRandomEmptyFolder1
    // DeleteRandomEmptyFolder2
    // KeepNonEmptyFolder
    //  - randomFile.txt
    QCOMPARE(QDir(EMPTY_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).size(), 3);
    EmptyFolderRemove efr;
    const int recycleCmdCnt = efr(EMPTY_TEST_DIR);
    QCOMPARE(recycleCmdCnt, 2);
    QCOMPARE(efr.m_cmds.size(), 2);
    QCOMPARE(efr.m_cmds[0].op, FileOperatorType::MOVETOTRASH);
    QCOMPARE(efr.m_cmds[1].op, FileOperatorType::MOVETOTRASH);
  }

  void test_redundantParentFolder() {
    // RedundantParentFolder
    //  - useful parent folder
    //    - random folder
    //    - 1.txt
    //    - 2.txt
    //  - useless parent folder
    //    - useless parent folder.txt
    QCOMPARE(QDir(REDUN_PARENT_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).size(), 2);

    RedunParentFolderRem rpfr;
    const int cmdCnt = rpfr(REDUN_PARENT_TEST_DIR);
    QCOMPARE(cmdCnt, 2);
    QCOMPARE(rpfr.m_cmds.size(), 2);
    QCOMPARE(rpfr.m_cmds[0].op, FileOperatorType::RENAME);
    QCOMPARE(rpfr.m_cmds[1].op, FileOperatorType::MOVETOTRASH);
  }

  void test_recycleRedundantItemByKeyWord() {
    // Delete Movie Falcon{delete.txt}
    // Keep KristenBjorn imgs{.jpg total: 11}
    // Keep RagingStallion Movie{keep.mp4}
    QCOMPARE(QDir(KEYWORD_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).size(), 3);
    int recycleCmdCnt = -1;

    RedundantItemsRemoverByKeyword rirbkFalcon{"Falcon"};
    recycleCmdCnt = rirbkFalcon(KEYWORD_TEST_DIR);
    QCOMPARE(recycleCmdCnt, 1);
    QCOMPARE(rirbkFalcon.m_cmds.size(), 1);

    RedundantItemsRemoverByKeyword rirbkRaging{"RagingStallion"};
    recycleCmdCnt = rirbkRaging(KEYWORD_TEST_DIR);
    QCOMPARE(recycleCmdCnt, 0);
    QVERIFY(rirbkRaging.m_cmds.isEmpty());

    RedundantItemsRemoverByKeyword rirbkKristen{"KristenBjorn"};
    recycleCmdCnt = rirbkKristen(KEYWORD_TEST_DIR);
    QCOMPARE(recycleCmdCnt, 0);
    QVERIFY(rirbkKristen.m_cmds.isEmpty());
  }
};

//QTEST_MAIN(RedundantItemsRemoverTest)
#include "RedundantItemsRemoverTest.moc"
