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
    // xx scene 1:{xx scene 1; xx scene 1.mp4; xx scene 1 2.jpg};
    // yy 1{yy 1; yy 1.png};
    const QString& SUB_TEST_DIR = QDir(TEST_DIR).absoluteFilePath("VidsEndBySceneIndexAndExistedFolder");
    categorizer(SUB_TEST_DIR);
    const auto& fL = QDir(SUB_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(fL.size(), 3);  // 1 file{yy 1.png} and 2 folder{xx scene 1, yy 1};

    QVERIFY(fL.contains("yy 1.png"));
    QVERIFY(fL.contains("xx scene 1"));
    QVERIFY(fL.contains("yy 1"));
  }
  void test_JsonImgVidNormalScene() {
    const QString& SUB_TEST_DIR = QDir(TEST_DIR).absoluteFilePath("JsonImgVidNormalScene");
    categorizer(SUB_TEST_DIR);

    const auto& fL = QDir(SUB_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(fL.size(), 1);  // only one folder;
    QVERIFY(fL.contains("xx"));
  }

  void test_GrabbedImageVidsScene(){
    // "xx.mp4" "xx.jpeg" "xx 00001.jpg" "xx 99999.jpeg"
    const QString& SUB_TEST_DIR = QDir(TEST_DIR).absoluteFilePath("GrabbedImageVidScene");
    categorizer(SUB_TEST_DIR);

    const auto& fL = QDir(SUB_TEST_DIR).entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(fL.size(), 1);  // only one folder;
    QVERIFY(fL.contains("xx"));
  }
};

QTEST_MAIN(CatergorizerTest)

#include "CatergorizerTest.moc"
