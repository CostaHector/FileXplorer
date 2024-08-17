#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "PublicTool.h"
#include "Tools/Categorizer.h"

const QString TEST_SRC_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_Classfier/DONT_CHANGE");
const QString TEST_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_Classfier/COPY_REMOVABLE");

class CatergorizerTest : public QObject {
  Q_OBJECT
 public:
  Categorizer categorizer;
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

  void initTestCase() {  // 将在执行第一个测试函数之前调用。
    qDebug("Start CatergorizerTest...");
  }
  //  initTestCase_data();//将被调用以创建全局测试数据表。
  void cleanupTestCase() {  // 将在执行最后一个测试函数后调用。
    qDebug("End CatergorizerTest...");
  }

  void test_a1Vid1ImgException() {
    // OneVid;
    const QString& SUB_TEST_DIR = QDir(TEST_DIR).absoluteFilePath("1Vid1ImgException");
    categorizer(SUB_TEST_DIR);
    const auto& fL = QDir(SUB_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(fL.size(), 2);
    QVERIFY(fL.contains("Test A.mp4"));
    QVERIFY(fL.contains("Test B.jpg"));
  }
  void test_3Imgs1Json() {
    // Test - 1984.jpg, Test - 1984.json
    // Test - 1.jpg, Test - 2.jpg
    const QString& SUB_TEST_DIR = QDir(TEST_DIR).absoluteFilePath("3Imgs1Json");
    categorizer(SUB_TEST_DIR);
    const auto& fL = QDir(SUB_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(fL.size(), 2);  // two folders: [Test, Test - 1984];
    QVERIFY(fL.contains("Test"));
    QVERIFY(fL.contains("Test - 1984"));

    const auto& FolderTestfL = QDir(SUB_TEST_DIR + "/Test").entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(FolderTestfL.size(), 2);
    const auto& FolderTest1984fL = QDir(SUB_TEST_DIR + "/Test - 1984").entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(FolderTest1984fL.size(), 2);
  }
  void test_VidsEndBySceneIndexAndExistedFolder() {
    // Falcon - Heated:{scene/sc./sc/part/pt./pt/p; - 2x\d - 2x\d};
    // Factory - Movie Name - Malik Daddy, Rafael Daddy:{2x\d};
    const QString& SUB_TEST_DIR = QDir(TEST_DIR).absoluteFilePath("VidsEndBySceneIndexAndExistedFolder");
    categorizer(SUB_TEST_DIR);
    const auto& fL = QDir(SUB_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(fL.size(), 2);  // 1 file{yy 1.png} and 2 folder{xx scene 1, yy 1};

    QVERIFY(fL.contains("Factory - Movie Name - Malik Daddy, Rafael Daddy"));
    QVERIFY(fL.contains("Falcon - Heated"));
  }

  void testrandomVidNames() {
    QCOMPARE(Categorizer::VidCoreName("a078d0708b9ed65258070434c23b14cd66b34256.mp4"), "a078d0708b9ed65258070434c23b14cd66b34256");
    QCOMPARE(Categorizer::VidCoreName("Name.mp4"), "Name");
  }

  void testrandomImgNames() {
    QCOMPARE(Categorizer::ImgCoreName("a078d0708b9ed65258070434c23b14cd66b34256.jpg"), "a078d0708b9ed65258070434c23b14cd66b34256");
    QCOMPARE(Categorizer::ImgCoreName("Heated 2022.webp"), "Heated 2022");
  }

  void testonlyIndexVidNames() {
    QCOMPARE(Categorizer::VidCoreName("MovieName - Micheal, Jensen Ankles 1.mp4"), "MovieName - Micheal, Jensen Ankles");
    QCOMPARE(Categorizer::VidCoreName("MovieName - Micheal, Jensen Ankles 19.mp4"), "MovieName - Micheal, Jensen Ankles");
    QCOMPARE(Categorizer::VidCoreName("MovieName - Micheal, Jensen Ankles - 1.mp4"), "MovieName - Micheal, Jensen Ankles");
    QCOMPARE(Categorizer::VidCoreName("MovieName - Micheal, Jensen Ankles - 19.mp4"), "MovieName - Micheal, Jensen Ankles");
  }

  void testvidNameSceneAndItsAbbrTest() {
    QCOMPARE(Categorizer::VidCoreName("Name Scene 1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name Sc. 1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name Sc.1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name Sc 1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name Sc1.mp4"), "Name");

    QCOMPARE(Categorizer::VidCoreName("Name scene 1.mp4"), "Name");
  }

  void testvidNamePartAndItsAbbrTest() {
    QCOMPARE(Categorizer::VidCoreName("Name Part 1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name Pt. 1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name Pt.1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name Pt 1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name Pt1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name P1.mp4"), "Name");

    QCOMPARE(Categorizer::VidCoreName("Name part 1.mp4"), "Name");
  }

  void testvidNameHypenTest() {
    QCOMPARE(Categorizer::VidCoreName("Name - Scene 1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name - Sc. 1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name - Sc 1.mp4"), "Name");

    QCOMPARE(Categorizer::VidCoreName("Name - Part 1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name - Pt. 1.mp4"), "Name");
    QCOMPARE(Categorizer::VidCoreName("Name - Pt 1.mp4"), "Name");

    QCOMPARE(Categorizer::VidCoreName("Blurred Line - Kris Evans, Fassbinder - Scene 1.mp4"), "Blurred Line - Kris Evans, Fassbinder");
    QCOMPARE(Categorizer::VidCoreName("Blurred Line - Kris Evans, Fassbinder - Sc. 1.mp4"), "Blurred Line - Kris Evans, Fassbinder");
    QCOMPARE(Categorizer::VidCoreName("Blurred Line - Kris Evans, Fassbinder - Sc 1.mp4"), "Blurred Line - Kris Evans, Fassbinder");
  }

  void testscenePartImgName() {
    QCOMPARE(Categorizer::ImgCoreName("Name Scene 1.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name Sc 1.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name SC. 1.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name SC.1.png"), "Name");

    QCOMPARE(Categorizer::ImgCoreName("Name Scene 1 - 1.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name Sc 1 - 2.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name SC. 1 - 3.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name SC.1 - 3.png"), "Name");

    QCOMPARE(Categorizer::ImgCoreName("Name Part 1.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name Pt 1.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name PT. 1.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name PT.1.png"), "Name");

    QCOMPARE(Categorizer::ImgCoreName("Name Part 1 - 4.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name Pt 1 - 5.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name PT. 1 - 6.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name PT.1 - 6.png"), "Name");
  }

  void testbasicImgNameNoIndex() {
    QCOMPARE(Categorizer::ImgCoreName("Name 1.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name - 1.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("LE - Sporty - Malik, King - 1.png"), "LE - Sporty - Malik, King");
  }

  void testbasicImgNameWithIndex() {
    QCOMPARE(Categorizer::ImgCoreName("Name 1.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name 1 - 1.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name - 1.png"), "Name");
    QCOMPARE(Categorizer::ImgCoreName("Name - 1 - 1.png"), "Name");
  }

  void test_JsonImgVidNormalScene() {
    const QString& SUB_TEST_DIR = QDir(TEST_DIR).absoluteFilePath("JsonImgVidNormalScene");
    categorizer(SUB_TEST_DIR);

    const auto& fL = QDir(SUB_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(fL.size(), 1);  // only one folder;
    QVERIFY(fL.contains("xx"));
  }

  void test_GrabbedImageVidsScene() {
    // "xx.mp4" "xx.jpeg" "xx 00001.jpg" "xx 99999.jpeg"
    const QString& SUB_TEST_DIR = QDir(TEST_DIR).absoluteFilePath("GrabbedImageVidScene");
    categorizer(SUB_TEST_DIR);

    const auto& fL = QDir(SUB_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(fL.size(), 1);  // only one folder;
    QVERIFY(fL.contains("xx"));
  }
};

//QTEST_MAIN(CatergorizerTest)
//#include "CatergorizerTest.moc"
