#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "TDir.h"
#include "BeginToExposePrivateMember.h"
#include "ItemsPacker.h"
#include "ItemsUnpacker.h"
#include "EndToExposePrivateMember.h"
#include "SceneMixed.h"
#include "SyncModifiyFileSystem.h"

class ItemsPackerTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir tDir;
  QString tPath{tDir.path()};
private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());
  }

  /* create a folder tDir contains
[file] "H.C..jpg"
[file] "H.C..json"
   */
  void test_group_folder_endwith_dot() {
    QVERIFY(tDir.ClearAll());
    const QList<FsNodeEntry> baseNameWithDotNodes // already sort name ascending
        {
         FsNodeEntry{"H.C..jpg", false, ""},
         FsNodeEntry{"H.C..json", false, ""},
         };
    QCOMPARE(tDir.createEntries(baseNameWithDotNodes), baseNameWithDotNodes.size());

    ScenesMixed sMixed;
    const QMap<QString, QStringList>& actualGrps = sMixed(tPath);
    const QMap<QString, QStringList> expectGrps{
        {"H.C.", {"H.C..json", "H.C..jpg"}} // ImgsSortNameLengthFirst used
    };
    QCOMPARE(actualGrps, expectGrps);

    ItemsPacker packer;
    int filesRearrangedCnt = packer(tPath, expectGrps);
    QCOMPARE(packer.m_cmds.size(), 3); // 3 = 1 create folder + 2 move file cmds
    QCOMPARE(filesRearrangedCnt, 2);

    QVERIFY(packer.StartToRearrange());

    const QSet<QString> expectSnapShots{
#ifdef _WIN32
        "H.C", // file-system thought that trailing dot can be chopped
        "H.C/H.C..json",
        "H.C/H.C..jpg",
#else
        "H.C.",
        "H.C./H.C..json",
        "H.C./H.C..jpg",
#endif
    };
    QCOMPARE(tDir.Snapshot(), expectSnapShots);
    // unpacker
    ItemsUnpacker unpacker;
    int upackedFoldersCnt = unpacker(tPath);
    QCOMPARE(upackedFoldersCnt, 1);
    QCOMPARE(unpacker.m_cmds.size(), 3);  // 3 = move 2 files, recycle 1 path
    QVERIFY(unpacker.StartToRearrange());
    auto actualNodes{tDir.getEntries()};
    std::sort(actualNodes.begin(), actualNodes.end());
    QCOMPARE(actualNodes, baseNameWithDotNodes);
  }

  /* create a folder mDir contains
[folder] Forbes - Movie Name - Chris Evans, John Reese
[folder] isolated folder
[file] Forbes - Movie Name - Chris Evans, John Reese 1.jpg
[file] Forbes - Movie Name - Chris Evans, John Reese.json
[file] Marvel - Heated.avi
[file] Marvel - Heated.mp4
[file] isolated file.json
   */
  void test_imgs_vids_isolated_existed_folder_pack_and_unpack_ok() {
    SyncModifiyFileSystem::m_syncOperationSw = false;

    QVERIFY(tDir.ClearAll());
    const QList<FsNodeEntry> envNodeEntries {
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese", true, {}},         //
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese 1.jpg", false, {}},  //
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese.json", false, {}},   //
        FsNodeEntry{"Marvel - Heated.avi", false, {}},                                  //
        FsNodeEntry{"Marvel - Heated.mp4", false, {}},                                  //
        FsNodeEntry{"isolated folder", true, {}},                                       //
        FsNodeEntry{"isolated file.json", false, {}},                                   //
    };
    const QList<FsNodeEntry> expectsPackerEntries{
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
    const QList<FsNodeEntry> expectsUnpackerEntries{
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese 1.jpg", false, {}},  //
        FsNodeEntry{"Forbes - Movie Name - Chris Evans, John Reese.json", false, {}},   //
        FsNodeEntry{"Marvel - Heated.avi", false, {}},                                  //
        FsNodeEntry{"Marvel - Heated.mp4", false, {}},                                  //
        FsNodeEntry{"isolated file.json", false, {}},                                   //
        FsNodeEntry{"isolated folder", true, {}},                                       //
    };

    QVERIFY(tDir.IsValid());
    QCOMPARE(tDir.createEntries(envNodeEntries), envNodeEntries.size());

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
    const QMap<QString, QStringList>& actualGrps = sMixed(tPath);
    QCOMPARE(expectGrps, actualGrps);

    ItemsPacker packer;
    int filesRearrangedCnt = packer(tPath, expectGrps);
    // mkpath "Marvel - Heated": 1
    // move files [jpg, json, avi, mp4]: 4
    QCOMPARE(packer.m_cmds.size(), 5);
    QCOMPARE(filesRearrangedCnt, 4);
    QVERIFY(packer.StartToRearrange());
    QCOMPARE(tDir.getEntries(), expectsPackerEntries);

    // unpacker
    ItemsUnpacker unpacker;
    int upackedFoldersCnt = unpacker(tPath);
    QCOMPARE(upackedFoldersCnt, 2);
    QCOMPARE(unpacker.m_cmds.size(), 6);  // recyle 2 path + move 4 files
    QVERIFY(unpacker.StartToRearrange());
    QCOMPARE(tDir.getEntries(), expectsUnpackerEntries);
  }

  void test_path_as_parameter() {
    QVERIFY(tDir.ClearAll());
    const QList<FsNodeEntry> envNodeEntries {
        {"A quick fox jump into", true, {}},       //
        {"A quick fox jump into.jpg", false, {}},  //
    };
    QCOMPARE(tDir.createEntries(envNodeEntries), 2);
    ItemsPacker ipacker;
    ipacker(tPath);
    QCOMPARE(ipacker.m_cmds.size(), 1); // only 1 command
  }
};

#include "ItemsPackerTest.moc"
REGISTER_TEST(ItemsPackerTest, false)
