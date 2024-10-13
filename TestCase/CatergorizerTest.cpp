#include <QCoreApplication>
#include <QtTest>

#include "PublicTool.h"
#include "pub/BeginToExposePrivateMember.h"
#include "Tools/Categorizer.h"
#include "Tools/ExtractPileItemsOutFolder.h"
#include "pub/EndToExposePrivateMember.h"

const QString TEST_SRC_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_Classfier/DONT_CHANGE");
const QString TEST_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_Classfier/COPY_REMOVABLE");

class CatergorizerTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void init() {
    if (QDir(TEST_DIR).exists()) {
      QDir(TEST_DIR).removeRecursively();
    }
    auto ret = PublicTool::copyDirectoryFiles(TEST_SRC_DIR, TEST_DIR);
    assert(ret);  // should copied ok
  }
  void cleanup() {
    if (QDir(TEST_DIR).exists()) {
      QDir(TEST_DIR).removeRecursively();
    }
  }

  void initTestCase() { qDebug("Start CatergorizerTest..."); }
  //  initTestCase_data();//将被调用以创建全局测试数据表。
  void cleanupTestCase() { qDebug("End CatergorizerTest..."); }

  void test_imgsVidsIsolatedExistedFolder();

  void test_name2CoreName_unchange();
  void test_vidNameWithIndex_chop_index();
  void test_imgNameWithIndex_chop_index();
};

void CatergorizerTest::test_name2CoreName_unchange() {
  QStringList dependentMixedFilesFolders;
  dependentMixedFilesFolders << "a078d0708b9ed65258070434c23b14cd66b34256"
                             << "a078d0708b9ed65258070434c23b14cd66b34276"
                             << "Name.mp4"
                             << "Heated 2022";
  Categorizer classfier;
  const auto& folder2Items = classfier.ClassifyItemIntoPiles(dependentMixedFilesFolders);
  QCOMPARE(folder2Items.size(), 4);
}

void CatergorizerTest::test_vidNameWithIndex_chop_index() {
  QStringList aMovie;
  aMovie << "MovieName - Micheal, Jensen Ankles.mp4"
         << "MovieName - Micheal, Jensen Ankles.avi"
         << "MovieName - Micheal, Jensen Ankles.mkv"
         << "MovieName - Micheal, Jensen Ankles FHD.mp4"
         << "MovieName - Micheal, Jensen Ankles 1080p.mp4"
         << "MovieName - Micheal, Jensen Ankles - HD.mp4";
  Categorizer classfier;
  const auto& folder2Items = classfier.ClassifyItemIntoPiles(aMovie);
  QCOMPARE(folder2Items.size(), 4);
  QVERIFY(folder2Items.contains("MovieName - Micheal, Jensen Ankles"));
  QVERIFY(folder2Items.contains("MovieName - Micheal, Jensen Ankles FHD"));
  QVERIFY(folder2Items.contains("MovieName - Micheal, Jensen Ankles 1080p"));
  QVERIFY(folder2Items.contains("MovieName - Micheal, Jensen Ankles - HD"));
}

void CatergorizerTest::test_imgNameWithIndex_chop_index() {
  QStringList imgs;
  imgs << "Name 1.png"
       << "Name - 1.png"                        // Name
       << "Name 1 - 1.png"                      // Name 1
       << "Name - 1 - 1.png"                    // Name - 1
       << "LE - Sporty - Malik, King - 1.png";  // LE - Sporty - Malik, King
  Categorizer classfier;
  const auto& folder2Items = classfier.ClassifyItemIntoPiles(imgs);
  QCOMPARE(folder2Items.size(), 4);
  QVERIFY(folder2Items.contains("Name"));
  QVERIFY(folder2Items.contains("Name 1"));
  QVERIFY(folder2Items.contains("Name - 1"));
  QVERIFY(folder2Items.contains("LE - Sporty - Malik, King"));
}

void CatergorizerTest::test_imgsVidsIsolatedExistedFolder() {
  QStringList theFirstItems =
      QDir(TEST_DIR, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot).entryList();

  // packer
  QMap<QString, QStringList> folder2Items;
  folder2Items["isolated file"] << "isolated file.json";                                                                  // isolated not rearrange
  folder2Items["isolated folder"] << "isolated folder";                                                                   // folder not rearrange
  folder2Items["Falcon - Heated"] << "Falcon - Heated.mp4"                                                                // create a folder and move
                                  << "Falcon - Heated.avi";                                                               // move
  folder2Items["Factory - Movie Name - Malik Daddy, Rafael Daddy"] << "Factory - Movie Name - Malik Daddy, Rafael Daddy"  // folder not rearrange
                                                                   << "Factory - Movie Name - Malik Daddy, Rafael Daddy 1.jpg"  // move
                                                                   << "Factory - Movie Name - Malik Daddy, Rafael Daddy.json";  // move
  Categorizer packer;
  int filesRearrangedCnt = packer(TEST_DIR, folder2Items);
  QCOMPARE(packer.m_cmds.size(), 5);  // mkpath 1 + move 4 files
  QCOMPARE(filesRearrangedCnt, 4);
  QVERIFY(packer.StartToRearrange());

  QDir dir(TEST_DIR, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
  QStringList itemsActual = dir.entryList();
  itemsActual.sort();
  QCOMPARE(itemsActual.size(), 4);

  QStringList expectItems{"isolated folder", "isolated file.json", "Falcon - Heated", "Factory - Movie Name - Malik Daddy, Rafael Daddy"};
  expectItems.sort();
  QCOMPARE(expectItems.size(), 4);
  QCOMPARE(itemsActual, expectItems);

  // unpacker
  ExtractPileItemsOutFolder unpacker;
  int upackedFoldersCnt = unpacker(TEST_DIR);
  QCOMPARE(upackedFoldersCnt, 2);
  QCOMPARE(unpacker.m_cmds.size(), 6);  // recyle 2 path + move 4 files
  QVERIFY(unpacker.StartToRearrange());

  QStringList theLastItems =
      QDir(TEST_DIR, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot).entryList();
  theLastItems << "Factory - Movie Name - Malik Daddy, Rafael Daddy";  // this folder will be removed at correct purpose
  theFirstItems.sort();
  theLastItems.sort();
  QCOMPARE(theFirstItems, theLastItems);
}

//QTEST_MAIN(CatergorizerTest)
#include "CatergorizerTest.moc"
