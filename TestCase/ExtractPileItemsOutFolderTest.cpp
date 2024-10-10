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
               << "name 4k.wmv"
               << "name 60FPS.wmv"
               << "name 1080p.wmv"
               << "name 720p.mp4"
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
                             << "name 4k.wmv"
                             << "name 60FPS.wmv"
                             << "name 1080p.wmv"
                             << "name 720p.mp4"
                             << "name.json"
                             << "name.jpg"
                             << "name 1.jpeg"
                             << "name 2.webp"
                             << "name 10.png"
                             << "name - 10.png";
    folder2PileItems["not a pile"] << "random name.mp4"
                                   << "another random name.mp4";
    ExtractPileItemsOutFolder epiof;
    QCOMPARE(epiof(folder2PileItems), 1);
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
};

//QTEST_MAIN(ExtractPileItemsOutFolderTest)
#include "ExtractPileItemsOutFolderTest.moc"
