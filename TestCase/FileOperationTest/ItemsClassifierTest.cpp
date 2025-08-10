#include <QCoreApplication>
#include <QtTest>
#include "MyTestSuite.h"
#include "TDir.h"
#include "BeginToExposePrivateMember.h"
#include "ItemsClassifier.h"
#include "ExtractPileItemsOutFolder.h"
#include "EndToExposePrivateMember.h"
#include "SceneMixed.h"
#include "SyncModifiyFileSystem.h"

class ItemsClassifierTest : public MyTestSuite {
  Q_OBJECT
 public:
  ItemsClassifierTest() : MyTestSuite{false}, mTestPath{mDir.path()} {}
  TDir mDir;
  const QString mTestPath;
  QList<FsNodeEntry> gNodeEntries, expectsPackerEntries, expectsUnpackerEntries;
 private slots:
  /* create a folder contains
[folder] Forbes - Movie Name - Chris Evans, John Reese
[folder] isolated folder
[file] Forbes - Movie Name - Chris Evans, John Reese 1.jpg
[file] Forbes - Movie Name - Chris Evans, John Reese.json
[file] Marvel - Heated.avi
[file] Marvel - Heated.mp4
[file] isolated file.json
   */
  void initTestCase() {
    SyncModifiyFileSystem::m_syncOperationSw = false;
    QVERIFY(mDir.IsValid());
    gNodeEntries = QList<FsNodeEntry>{
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese", true, {}},         //
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese 1.jpg", false, {}},  //
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese.json", false, {}},   //
        FsNodeEntry{"Marvel - Heated.avi", false, {}},                                  //
        FsNodeEntry{"Marvel - Heated.mp4", false, {}},                                  //
        FsNodeEntry{"isolated folder", true, {}},                                       //
        FsNodeEntry{"isolated file.json", false, {}},                                   //
    };
    QCOMPARE(mDir.createEntries(gNodeEntries), gNodeEntries.size());

    expectsPackerEntries = QList<FsNodeEntry>{
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese", true, {}},                                                       //
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese/Forbes - Movie Name - Chris Evans, John Reese 1.jpg", false, {}},  //
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese/Forbes - Movie Name - Chris Evans, John Reese.json", false, {}},   //
        FsNodeEntry{"Marvel - Heated", true, {}},                                                                                     //
        FsNodeEntry{"Marvel - Heated/Marvel - Heated.avi", false, {}},                                                                //
        FsNodeEntry{"Marvel - Heated/Marvel - Heated.mp4", false, {}},                                                                //
        FsNodeEntry{"isolated file.json", false, {}},                                                                                 //
        FsNodeEntry{"isolated folder", true, {}},                                                                                     //
    };

    // After unpack, empty folder "Forbes - Movie Name - Chris Evans, John Reese" will be removed
    expectsUnpackerEntries = QList<FsNodeEntry>{
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese 1.jpg", false, {}},  //
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese.json", false, {}},   //
        FsNodeEntry{"Marvel - Heated.avi", false, {}},                                  //
        FsNodeEntry{"Marvel - Heated.mp4", false, {}},                                  //
        FsNodeEntry{"isolated file.json", false, {}},                                   //
        FsNodeEntry{"isolated folder", true, {}},                                       //
    };
  }

  void test_imgs_vids_isolated_existed_folder_pack_and_unpack_ok() {
    // packer
    const QMap<QString, QStringList> expectGrps{
        {"isolated file", {"isolated file.json"}},                            // isolated not rearrange
        {"Marvel - Heated", {"Marvel - Heated.avi", "Marvel - Heated.mp4"}},  // move,create a folder and move
        {
            "Forbes - Movie Name - Chris Evans, John Reese",                                                               //
            {"Forbes - Movie Name - Chris Evans, John Reese.json", "Forbes - Movie Name - Chris Evans, John Reese 1.jpg"}  //
        },                                                                                                                 //
    };
    ScenesMixed sMixed;
    const QMap<QString, QStringList>& actualGrps = sMixed(mTestPath);
    QCOMPARE(expectGrps, actualGrps);

    ItemsClassifier packer;
    int filesRearrangedCnt = packer(mTestPath, expectGrps);
    // mkpath "Marvel - Heated": 1
    // move files [jpg, json, avi, mp4]: 4
    QCOMPARE(packer.m_cmds.size(), 5);
    QCOMPARE(filesRearrangedCnt, 4);
    QVERIFY(packer.StartToRearrange());
    QCOMPARE(mDir.getEntries(), expectsPackerEntries);

    // unpacker
    ExtractPileItemsOutFolder unpacker;
    int upackedFoldersCnt = unpacker(mTestPath);
    QCOMPARE(upackedFoldersCnt, 2);
    QCOMPARE(unpacker.m_cmds.size(), 6);  // recyle 2 path + move 4 files
    QVERIFY(unpacker.StartToRearrange());
    QCOMPARE(mDir.getEntries(), expectsUnpackerEntries);
  }
};

#include "ItemsClassifierTest.moc"
ItemsClassifierTest g_ItemsClassifierTest;
