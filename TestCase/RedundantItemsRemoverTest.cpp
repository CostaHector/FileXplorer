#include <QCoreApplication>
#include <QtTest>

#include "TestCase/pub/BeginToExposePrivateMember.h"
#include "Tools/RedundantFolderRemove.h"
#include "TestCase/pub/EndToExposePrivateMember.h"
#include "pub/FileSystemTestSuite.h"

class RedundantItemsRemoverTest : public FileSystemTestSuite {
  Q_OBJECT
 public:
  RedundantItemsRemoverTest() : FileSystemTestSuite{"TestEnv_RedundantItemRemove", false, false} {}
 private slots:
  void cleanupTestCase() {  // after the last test function was executed.
    FileSystemHelper(mTestPath).EraseFileSystemTree(true);
  }

  void initTestCase() {
    /*
EmptyFolder
    // DeleteRandomEmptyFolder1
    // DeleteRandomEmptyFolder2
    // KeepNonEmptyFolder
    //  - randomFile.txt
RedundantKeyword
    // Delete Movie Falcon{delete.txt}
    // Keep Marvel imgs{.jpg total: 11 file(s)}
    // Keep Fox Movie{keep.mp4}
RedundantParentFolder
    // useful parent folder
    // - random folder
    // - 1.txt
    // - 2.txt
    // useless parent folder
    // - useless parent folder.txt
     */
    m_rootHelper << FileSystemNode{"EmptyFolder"}             //
                 << FileSystemNode{"RedundantKeyword"}        //
                 << FileSystemNode{"RedundantParentFolder"};  //

    m_rootHelper.GetSubHelper("EmptyFolder")           //
        << FileSystemNode{"DeleteRandomEmptyFolder1"}  //
        << FileSystemNode{"DeleteRandomEmptyFolder2"}  //
        << FileSystemNode{"KeepNonEmptyFolder"};
    m_rootHelper.GetSubHelper("EmptyFolder").GetSubHelper("KeepNonEmptyFolder")  //
        << FileSystemNode{"randomFile.txt", false, ""};

    m_rootHelper.GetSubHelper("RedundantKeyword")  //
        << FileSystemNode{"Delete Movie Falcon"}   //
        << FileSystemNode{"Keep Marvel imgs"}      //
        << FileSystemNode{"Keep Fox Movie"};
    m_rootHelper.GetSubHelper("RedundantKeyword").GetSubHelper("Delete Movie Falcon")  //
        << FileSystemNode{"delete.txt", false, ""};
    auto imgs = m_rootHelper.GetSubHelper("RedundantKeyword").GetSubHelper("Keep Marvel imgs");
    for (int imgCnt = 1; imgCnt < 12; ++imgCnt) {  // create 11 image(s)
      imgs << FileSystemNode{QString("keep - Copy (%1).jpg").arg(imgCnt)};
    }
    m_rootHelper.GetSubHelper("RedundantKeyword").GetSubHelper("Keep Fox Movie")  //
        << FileSystemNode{"keep.mp4", false, ""};

    m_rootHelper.GetSubHelper("RedundantParentFolder")  //
        << FileSystemNode{"useful parent folder"}       //
        << FileSystemNode{"useless parent folder"};
    m_rootHelper.GetSubHelper("RedundantParentFolder").GetSubHelper("useful parent folder")  //
        << FileSystemNode{"random folder"}                                                   //
        << FileSystemNode{"1.txt", false, ""}                                                //
        << FileSystemNode{"2.txt", false, ""};
    m_rootHelper.GetSubHelper("RedundantParentFolder").GetSubHelper("useless parent folder")  //
        << FileSystemNode{"useless parent folder.txt", false, ""};
  }

  void test_recycleEmptyFolder() {
    // DeleteRandomEmptyFolder1
    // DeleteRandomEmptyFolder2
    // KeepNonEmptyFolder
    //  - randomFile.txt
    const QString EMPTY_TEST_DIR = mTestPath + "/EmptyFolder";
    QCOMPARE(QDir(EMPTY_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).size(), 3);
    EmptyFolderRemove efr;
    const int recycleCmdCnt = efr(EMPTY_TEST_DIR);
    QCOMPARE(recycleCmdCnt, 2);
    QCOMPARE(efr.m_cmds.size(), 2);
    QCOMPARE(efr.m_cmds[0].op, FileOperatorType::MOVETOTRASH);
    QCOMPARE(efr.m_cmds[1].op, FileOperatorType::MOVETOTRASH);
  }

  void test_redundantParentFolder() {
    // useful parent folder
    // - random folder
    // - 1.txt
    // - 2.txt
    // useless parent folder
    // - useless parent folder.txt
    const QString REDUN_PARENT_TEST_DIR = mTestPath + "/RedundantParentFolder";
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
    // Keep Marvel imgs{.jpg total: 11}
    // Keep Fox Movie{keep.mp4}
    const QString KEYWORD_TEST_DIR = mTestPath + "/RedundantKeyword";
    QCOMPARE(QDir(KEYWORD_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).size(), 3);
    int recycleCmdCnt = -1;

    RedundantItemsRemoverByKeyword rirbkFalcon{"Falcon"};
    recycleCmdCnt = rirbkFalcon(KEYWORD_TEST_DIR);
    QCOMPARE(recycleCmdCnt, 1);
    QCOMPARE(rirbkFalcon.m_cmds.size(), 1);

    RedundantItemsRemoverByKeyword rirbkRaging{"Fox"};
    recycleCmdCnt = rirbkRaging(KEYWORD_TEST_DIR);
    QCOMPARE(recycleCmdCnt, 0);
    QVERIFY(rirbkRaging.m_cmds.isEmpty());

    RedundantItemsRemoverByKeyword rirbkKristen{"Marvel"};
    recycleCmdCnt = rirbkKristen(KEYWORD_TEST_DIR);
    QCOMPARE(recycleCmdCnt, 0);
    QVERIFY(rirbkKristen.m_cmds.isEmpty());
  }
};

#include "RedundantItemsRemoverTest.moc"
RedundantItemsRemoverTest g_RedundantItemsRemoverTest;
