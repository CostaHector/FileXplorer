#include <QCoreApplication>
#include <QtTest>

#include "Tools/ExtractPileItemsOutFolder.h"

class ExtractPileItemsOutFolderTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}

  void init() {}
  void cleanup() {}

  void test_cannotExtract_ok() {
    QStringList noStrictAPileFiles;
    noStrictAPileFiles << "random name.mp4";
    noStrictAPileFiles << "another random name.mp4";
    QVERIFY(!ExtractPileItemsOutFolder::CanExtractOut(noStrictAPileFiles));
  }

  void test_canExtract_ok() {
    QStringList aPileFiles;
    aPileFiles << "name.mp4"
               << "name.mkv"
               << "name.ts"
               << "name.avi"
               << "name.wmv"
               << "name.json"
               << "name.jpg"
               << "name 1.jpeg"
               << "name 2.webp"
               << "name 10.png"
               << "name - 10.png";
    QVERIFY(ExtractPileItemsOutFolder::CanExtractOut(aPileFiles));
  }

  void test_foldersCntNeedExtract_ok() {
    QMap<QString, QStringList> folder2PileItems;
    folder2PileItems["name"] << "name.mp4"
                             << "name.mkv"
                             << "name.ts"
                             << "name.avi"
                             << "name.wmv"
                             << "name.json"
                             << "name.jpg"
                             << "name 1.jpeg"
                             << "name 2.webp"
                             << "name 10.png"
                             << "name - 10.png";
    folder2PileItems["not a pile"] << "random name.mp4"
                                   << "another random name.mp4";
    ExtractPileItemsOutFolder epiof;
    QCOMPARE(epiof("not existed out directory", folder2PileItems), 1);
  }

  void test_MixedScenes() {
    QStringList files;
    files << "name.mp4"
          << "name.mkv"
          << "name.json"
          << "name.jpg"
          << "name 1.jpeg"
          << "name 2.webp"
          << "name 10.png"
          << "name - 10.png";
    files << "another name.mp4"
          << "another name.wmv"
          << "another name.json"
          << "another name.jpg"
          << "another name 1.jpeg"
          << "another name 2.webp"
          << "another name 10.png"
          << "another name - 10.png";

    ScenesMixed sMixed;
    QCOMPARE(sMixed(files), 2);
    QCOMPARE(sMixed.m_img2Name["name"].size(), 5);
    QCOMPARE(sMixed.m_img2Name["name"][0], "name.jpg");
    QCOMPARE(sMixed.m_vid2Name["name"].size(), 2);
    QCOMPARE(sMixed.m_json2Name["name"], "name.json");

    QCOMPARE(sMixed.m_img2Name["another name"].size(), 5);
    QCOMPARE(sMixed.m_img2Name["another name"][0], "another name.jpg");
    QCOMPARE(sMixed.m_vid2Name["another name"].size(), 2);
    QCOMPARE(sMixed.m_json2Name["another name"], "another name.json");
  }

  void test_MixScenesEndWithNumber() {
    QStringList files;
    files << "name98.mp4"
          << "name98.jpg"
          << "name98 1.jpg"
          << "name98.json";
    ScenesMixed sMixed;
    QCOMPARE(sMixed(files), 1); // 1 json file, 1
    QCOMPARE(sMixed.m_img2Name["name98"].size(), 2);
    QCOMPARE(sMixed.m_img2Name["name98"][0], "name98.jpg");
    QCOMPARE(sMixed.m_img2Name["name98"][1], "name98 1.jpg");
  }
};

QTEST_MAIN(ExtractPileItemsOutFolderTest)
#include "ExtractPileItemsOutFolderTest.moc"
