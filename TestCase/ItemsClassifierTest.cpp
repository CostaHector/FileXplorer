#include <QCoreApplication>
#include <QtTest>
#include "pub/FileSystemTestSuite.h"

#include "Tools/Classify/SceneMixed.h"
#include "pub/BeginToExposePrivateMember.h"
#include "Tools/Classify/ItemsClassifier.h"
#include "Tools/ExtractPileItemsOutFolder.h"
#include "pub/EndToExposePrivateMember.h"

class ItemsClassifierTest : public FileSystemTestSuite {
  Q_OBJECT
 public:
  ItemsClassifierTest() : FileSystemTestSuite{"TestEnv_Classfier", true, false} {}
 private slots:
  void test_imgsVidsIsolatedExistedFolder_pack_and_unpack_ok() {
    /* create a folder contains
[folder] Factory - Movie Name - Malik Sir, Rafael Sir
[folder] isolated folder
[file] Factory - Movie Name - Malik Sir, Rafael Sir 1.jpg
[file] Factory - Movie Name - Malik Sir, Rafael Sir.json
[file] Marvel - Heated.avi
[file] Marvel - Heated.mp4
[file] isolated file.json
     */
    m_rootHelper << FileSystemNode{"Factory - Movie Name - Malik Sir, Rafael Sir"}                   // empty folder
                 << FileSystemNode{"isolated folder"}                                                // no operation on this folder
                 << FileSystemNode{"Factory - Movie Name - Malik Sir, Rafael Sir 1.jpg", false, ""}  //
                 << FileSystemNode{"Factory - Movie Name - Malik Sir, Rafael Sir.json", false, ""}   //
                 << FileSystemNode{"Marvel - Heated.avi", false, ""}                                 //
                 << FileSystemNode{"Marvel - Heated.mp4", false, ""}                                 //
                 << FileSystemNode{"isolated file.json", false, ""};                                 // no operation on this file

    QStringList theFirstItems = QDir{mTestPath, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot}.entryList();
    theFirstItems.sort();

    // packer
    QMap<QString, QStringList> expectFolder2Items;
    expectFolder2Items["isolated file"] << "isolated file.json";        // isolated not rearrange
    expectFolder2Items["Marvel - Heated"]                               //
        << "Marvel - Heated.avi"                                        // move
        << "Marvel - Heated.mp4";                                       // create a folder and move
    expectFolder2Items["Factory - Movie Name - Malik Sir, Rafael Sir"]  //
        << "Factory - Movie Name - Malik Sir, Rafael Sir.json"          // move
        << "Factory - Movie Name - Malik Sir, Rafael Sir 1.jpg";        // move
    ScenesMixed sMixed;
    const auto& actualexpectFolder2Items = sMixed(mTestPath);
    QCOMPARE(expectFolder2Items, actualexpectFolder2Items);

    ItemsClassifier packer;
    int filesRearrangedCnt = packer(mTestPath, expectFolder2Items);
    // mkpath "Marvel - Heated": 1
    // move files [jpg, json, avi, mp4]: 4
    QCOMPARE(packer.m_cmds.size(), 5);
    QCOMPARE(filesRearrangedCnt, 4);
    QVERIFY(packer.StartToRearrange());

    QStringList itemsActual = QDir{mTestPath, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot}.entryList();
    itemsActual.sort();
    QCOMPARE(itemsActual.size(), 4);

    QStringList expectItems{"isolated folder", "isolated file.json", "Marvel - Heated", "Factory - Movie Name - Malik Sir, Rafael Sir"};
    expectItems.sort();
    QCOMPARE(expectItems.size(), 4);
    QCOMPARE(itemsActual, expectItems);

    // unpacker
    ExtractPileItemsOutFolder unpacker;
    int upackedFoldersCnt = unpacker(mTestPath);
    QCOMPARE(upackedFoldersCnt, 2);
    QCOMPARE(unpacker.m_cmds.size(), 6);  // recyle 2 path + move 4 files
    QVERIFY(unpacker.StartToRearrange());

    QStringList theLastItems = QDir(mTestPath, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot).entryList();
    // folder "Factory - Movie Name - Malik Sir, Rafael Sir" after unpack will be removed
    theLastItems << "Factory - Movie Name - Malik Sir, Rafael Sir";
    theLastItems.sort();
    // before items and after items only differ in this folder
    QCOMPARE(theFirstItems, theLastItems);
  }
};

#include "ItemsClassifierTest.moc"
ItemsClassifierTest g_ItemsClassifierTest;
