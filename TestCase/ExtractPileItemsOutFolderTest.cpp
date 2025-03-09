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
};

//QTEST_MAIN(ExtractPileItemsOutFolderTest)
#include "ExtractPileItemsOutFolderTest.moc"
