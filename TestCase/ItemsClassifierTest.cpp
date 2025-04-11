#include <QCoreApplication>
#include <QtTest>

#include "pub/BeginToExposePrivateMember.h"
#include "Tools/Classify/ItemsClassifier.h"
#include "Tools/ExtractPileItemsOutFolder.h"
#include "pub/EndToExposePrivateMember.h"
#include "pub/FileSystemRelatedTest.h"

class ItemsClassifierTest : public FileSystemRelatedTest {
  Q_OBJECT
 public:
  ItemsClassifierTest() : FileSystemRelatedTest{"TestEnv_Classfier"} {}
 private slots:
  void test_imgsVidsIsolatedExistedFolder() {
    /*
Factory - Movie Name - Malik Daddy, Rafael Daddy
isolated folder
Factory - Movie Name - Malik Daddy, Rafael Daddy 1.jpg
Factory - Movie Name - Malik Daddy, Rafael Daddy.json
Falcon - Heated.avi
Falcon - Heated.mp4
isolated file.json
     */
    m_rootHelper << FileSystemNode{"Factory - Movie Name - Malik Daddy, Rafael Daddy"}                   //
                 << FileSystemNode{"isolated folder"}                                                    //
                 << FileSystemNode{"Factory - Movie Name - Malik Daddy, Rafael Daddy 1.jpg", false, ""}  //
                 << FileSystemNode{"Factory - Movie Name - Malik Daddy, Rafael Daddy.json", false, ""}   //
                 << FileSystemNode{"Falcon - Heated.avi", false, ""}                                     //
                 << FileSystemNode{"Falcon - Heated.mp4", false, ""}                                     //
                 << FileSystemNode{"isolated file.json", false, ""};

    QStringList theFirstItems = QDir(ROOT_DIR, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot).entryList();

    // packer
    QMap<QString, QStringList> folder2Items;
    folder2Items["isolated file"] << "isolated file.json";                                                                        // isolated not rearrange
    folder2Items["isolated folder"] << "isolated folder";                                                                         // folder not rearrange
    folder2Items["Falcon - Heated"] << "Falcon - Heated.mp4"                                                                      // create a folder and move
                                    << "Falcon - Heated.avi";                                                                     // move
    folder2Items["Factory - Movie Name - Malik Daddy, Rafael Daddy"] << "Factory - Movie Name - Malik Daddy, Rafael Daddy"        // folder not rearrange
                                                                     << "Factory - Movie Name - Malik Daddy, Rafael Daddy 1.jpg"  // move
                                                                     << "Factory - Movie Name - Malik Daddy, Rafael Daddy.json";  // move
    ItemsClassifier packer;
    int filesRearrangedCnt = packer(ROOT_DIR, folder2Items);
    QCOMPARE(packer.m_cmds.size(), 5);  // mkpath 1 + move 4 files
    QCOMPARE(filesRearrangedCnt, 4);
    QVERIFY(packer.StartToRearrange());

    QDir dir(ROOT_DIR, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    QStringList itemsActual = dir.entryList();
    itemsActual.sort();
    QCOMPARE(itemsActual.size(), 4);

    QStringList expectItems{"isolated folder", "isolated file.json", "Falcon - Heated", "Factory - Movie Name - Malik Daddy, Rafael Daddy"};
    expectItems.sort();
    QCOMPARE(expectItems.size(), 4);
    QCOMPARE(itemsActual, expectItems);

    // unpacker
    ExtractPileItemsOutFolder unpacker;
    int upackedFoldersCnt = unpacker(ROOT_DIR);
    QCOMPARE(upackedFoldersCnt, 2);
    QCOMPARE(unpacker.m_cmds.size(), 6);  // recyle 2 path + move 4 files
    QVERIFY(unpacker.StartToRearrange());

    QStringList theLastItems = QDir(ROOT_DIR, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot).entryList();
    // this empty folder will be removed after unpack
    // before items and after items only differ in this folder
    theLastItems << "Factory - Movie Name - Malik Daddy, Rafael Daddy";
    theFirstItems.sort();
    theLastItems.sort();
    QCOMPARE(theFirstItems, theLastItems);
  }
};

//QTEST_MAIN(ItemsClassifierTest);
#include "ItemsClassifierTest.moc"
