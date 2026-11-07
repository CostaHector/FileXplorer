#include <QCoreApplication>
#include <QtTest>

#include "BeginToExposePrivateMember.h"
#include "RedundantRmv.h"
#include "EndToExposePrivateMember.h"
#include "PlainTestSuite.h"

#include "TDir.h"
#include "UndoRedo.h"

class RedundantRmvTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
private slots:
  void initTestCase() {
    /*
EmptyFolder
    // DeleteRandomEmptyFolder1
    //  - SubDeleteRandomEmptyFolder1
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
    QList<FsNodeEntry> nodes
        {
         // EmptyFolder
         {"EmptyFolder/DeleteRandomEmptyFolder1/SubDeleteRandomEmptyFolder1", true, ""},
         {"EmptyFolder/DeleteRandomEmptyFolder2", true, ""},
         {"EmptyFolder/KeepNonEmptyFolder/randomFile.txt", false, ""},
         // RedundantParentFolder
         {"RedundantParentFolder/useful parent folder/random folder", true, ""},
         {"RedundantParentFolder/useful parent folder/1.txt", false, ""},
         {"RedundantParentFolder/useful parent folder/2.txt", false, ""},
         {"RedundantParentFolder/useless parent folder/useless parent folder.txt", false, ""},
         // RedundantKeyword
         {"RedundantKeyword/Delete Movie Falcon/delete.txt", false, ""},
         {"RedundantKeyword/Keep Marvel imgs/keep - Copy (1).jpg", false, ""},
         {"RedundantKeyword/Keep Marvel imgs/keep - Copy (2).jpg", false, ""},
         {"RedundantKeyword/Keep Marvel imgs/keep - Copy (3).jpg", false, ""},
         {"RedundantKeyword/Keep Marvel imgs/keep - Copy (4).jpg", false, ""},
         {"RedundantKeyword/Keep Marvel imgs/keep - Copy (5).jpg", false, ""},
         {"RedundantKeyword/Keep Marvel imgs/keep - Copy (6).jpg", false, ""},
         {"RedundantKeyword/Keep Marvel imgs/keep - Copy (7).jpg", false, ""},
         {"RedundantKeyword/Keep Marvel imgs/keep - Copy (8).jpg", false, ""},
         {"RedundantKeyword/Keep Marvel imgs/keep - Copy (9).jpg", false, ""},
         {"RedundantKeyword/Keep Marvel imgs/keep - Copy (10).jpg", false, ""},
         {"RedundantKeyword/Keep Marvel imgs/keep - Copy (11).jpg", false, ""},
         {"RedundantKeyword/Keep Fox Movie/keep.mp4", false, ""},
         };
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());
  }

  void recycleEmptyFolder_ok() {
    // DeleteRandomEmptyFolder1                 --- recycle 1
    //  - SubDeleteRandomEmptyFolder1           --- recycle 0
    // DeleteRandomEmptyFolder2                 --- recycle 2
    // KeepNonEmptyFolder
    //  - randomFile.txt
    const QString workPath = mTDir.itemPath("EmptyFolder");

    EmptyFolderRmv efr;
    const int recycleCmdCnt = efr.Find(workPath);
    QCOMPARE(recycleCmdCnt, 3);
    QCOMPARE(efr.m_workPath, workPath);

    const QStringList expectsCmds {
        "DeleteRandomEmptyFolder1/SubDeleteRandomEmptyFolder1",
        "DeleteRandomEmptyFolder1",
        "DeleteRandomEmptyFolder2",
    };
    QCOMPARE(efr.m_mixedCmds.size(), 0);
    QCOMPARE(efr.m_relNames, expectsCmds);
  }

  void redundantParentFolder_ok() {
    // useful parent folder
    // - random folder
    // - 1.txt
    // - 2.txt
    // useless parent folder         需要删除
    // - useless parent folder.txt   需要上移
    QString workPath = mTDir.itemPath("RedundantParentFolder");
    QSet<QString> initialItems = mTDir.SnapshotAtPath(workPath);
    QVERIFY(initialItems.contains("useless parent folder/useless parent folder.txt"));

    ZeroOrOneItemFolderProc rpfr;
    const int cmdCnt = rpfr.Find(workPath);
    QCOMPARE(rpfr.m_workPath, workPath);
    QCOMPARE(cmdCnt, 2);
    QCOMPARE(rpfr.m_mixedCmds.size(), 2); // no confirm dialog show. no need mock
    QCOMPARE(rpfr.m_relNames.size(), 0);
    QVERIFY(rpfr.Exec());

    QSet<QString> afterRemoveItems = mTDir.SnapshotAtPath(workPath);
    QVERIFY(afterRemoveItems.contains("useless parent folder.txt"));
    QVERIFY(!afterRemoveItems.contains("useless parent folder"));


    QCOMPARE(UndoRedo::on_Undo(), true);
    QSet<QString> recoverItems = mTDir.SnapshotAtPath(workPath);
    QCOMPARE(recoverItems, initialItems);
  }

  void recycleRedundantItemByKeyWord_ok() {
    // {"RedundantKeyword/Delete Movie Falcon/delete.txt", false, ""},
    // {"RedundantKeyword/Keep Marvel imgs/keep - Copy (1).jpg", false, ""},
    // {"RedundantKeyword/Keep Marvel imgs/keep - Copy (2).jpg", false, ""},
    // {"RedundantKeyword/Keep Marvel imgs/keep - Copy (3).jpg", false, ""},
    // {"RedundantKeyword/Keep Marvel imgs/keep - Copy (4).jpg", false, ""},
    // {"RedundantKeyword/Keep Marvel imgs/keep - Copy (5).jpg", false, ""},
    // {"RedundantKeyword/Keep Marvel imgs/keep - Copy (6).jpg", false, ""},
    // {"RedundantKeyword/Keep Marvel imgs/keep - Copy (7).jpg", false, ""},
    // {"RedundantKeyword/Keep Marvel imgs/keep - Copy (8).jpg", false, ""},
    // {"RedundantKeyword/Keep Marvel imgs/keep - Copy (9).jpg", false, ""},
    // {"RedundantKeyword/Keep Marvel imgs/keep - Copy (10).jpg", false, ""},
    // {"RedundantKeyword/Keep Marvel imgs/keep - Copy (11).jpg", false, ""},
    // {"RedundantKeyword/Keep Fox Movie/keep.mp4", false, ""},

    const QString workPath = mTDir.itemPath("RedundantKeyword");
    int recycleCmdCnt = -1;

    FolderNameContainKeyRmv rirbkFalcon{"Falcon"};
    recycleCmdCnt = rirbkFalcon.Find(workPath);
    QCOMPARE(recycleCmdCnt, 1);
    QCOMPARE(rirbkFalcon.m_relNames.size(), 1);
    QCOMPARE(rirbkFalcon.m_mixedCmds.size(), 0);
    QCOMPARE(rirbkFalcon.m_workPath, workPath);
    QCOMPARE(rirbkFalcon.m_relNames, (QStringList{"Delete Movie Falcon"}));
    // Don't Exec. Otherwise Undo needed to avoid in system trashbin

    FolderNameContainKeyRmv rirbkFox{"Fox"};
    QCOMPARE(rirbkFox.Find(workPath), 0);
    QVERIFY(rirbkFox.m_relNames.isEmpty());
    QVERIFY(rirbkFox.m_mixedCmds.isEmpty());
    QVERIFY(rirbkFox.Exec()); // skip

    FolderNameContainKeyRmv rirbkMarvel{"Marvel"};
    QCOMPARE(rirbkMarvel.Find(workPath), 0);
    QVERIFY(rirbkMarvel.m_relNames.isEmpty());
    QVERIFY(rirbkMarvel.m_mixedCmds.isEmpty());
    QVERIFY(rirbkMarvel.Exec()); // skip
  }
};

#include "RedundantRmvTest.moc"
REGISTER_TEST(RedundantRmvTest, false)
