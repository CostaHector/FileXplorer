#include <QCoreApplication>
#include <QtTest>

#include "Tools/Classify/SceneMixed.h"

class ScenesMixedTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void test_basicFileNames() {
    QStringList files;
    files << "X-Man - The Last Stand.mp4"
          << "X-Man - The Last Stand.json"
          << "X-Man - The Last Stand.jpg"
          << "X-Man - The Last Stand.png"
          << "X-Man - The Last Stand 33.png"
          << "X-Man - The Last Stand 1.png";
    ScenesMixed sMixed;
    const auto& batches = sMixed(files);
    QCOMPARE(batches.size(), 1);  // batch count = 1
    QCOMPARE(batches["X-Man - The Last Stand"].size(), 6);
    QCOMPARE(sMixed.m_json2Name.size(), 1);
    QCOMPARE(sMixed.m_vid2Name.size(), 1);
    QCOMPARE(sMixed.m_img2Name.size(), 1);

    QVERIFY(sMixed.m_json2Name.contains("X-Man - The Last Stand"));
    QVERIFY(sMixed.m_vid2Name.contains("X-Man - The Last Stand"));
    QVERIFY(sMixed.m_img2Name.contains("X-Man - The Last Stand"));

    // in ascii table ' ' = 0x20, ' ' = 0x2E
    // but here we want length shorter rather than ascii less
    QStringList imgsSorted{"X-Man - The Last Stand.jpg", "X-Man - The Last Stand.png", "X-Man - The Last Stand 1.png", "X-Man - The Last Stand 33.png"};
    QCOMPARE(sMixed.m_img2Name["X-Man - The Last Stand"], imgsSorted);
  }

  void test_VideoWithoutPartNumber() {
    QStringList files;
    files << "Tarzan.mp4"
          << "Tarzan.mkv"
          << "Tarzan.json"
          << "Tarzan.jpg"
          << "Tarzan 1.jpeg"
          << "Tarzan 2.webp"
          << "Tarzan 10.png"
          << "Tarzan - 10.png";
    files << "another Tarzan.mp4"
          << "another Tarzan.wmv"
          << "another Tarzan.json"
          << "another Tarzan.jpg"
          << "another Tarzan 1.jpeg"
          << "another Tarzan 2.webp"
          << "another Tarzan 10.png"
          << "another Tarzan - 10.png";

    ScenesMixed sMixed;
    const auto& batches = sMixed(files);
    QCOMPARE(batches.size(), 2);  // batch count = 2
    QCOMPARE(batches["Tarzan"].size(), 8);
    QCOMPARE(batches["another Tarzan"].size(), 8);
    QCOMPARE(sMixed.m_img2Name["Tarzan"].size(), 5);
    QCOMPARE(sMixed.m_img2Name["Tarzan"][0], "Tarzan.jpg");
    QCOMPARE(sMixed.m_vid2Name["Tarzan"].size(), 2);
    QCOMPARE(sMixed.m_json2Name["Tarzan"], "Tarzan.json");

    QCOMPARE(sMixed.m_img2Name["another Tarzan"].size(), 5);
    QCOMPARE(sMixed.m_img2Name["another Tarzan"][0], "another Tarzan.jpg");
    QCOMPARE(sMixed.m_vid2Name["another Tarzan"].size(), 2);
    QCOMPARE(sMixed.m_json2Name["another Tarzan"], "another Tarzan.json");
  }

  void test_VidWithPartNumber() {
    QStringList files;
    files << "Superman - The Inked Meat Part 1.mp4"
          << "Superman - The Inked Meat Part 1.json"
          << "Superman - The Inked Meat Part 1.jpg"
          << "Superman - The Inked Meat Part 1 0.jpg"
          << "Superman - The Inked Meat Part 1 33.jpg";
    files << "Superman - The Inked Meat Part 2.mp4"
          << "Superman - The Inked Meat Part 2.json"
          << "Superman - The Inked Meat Part 2.jpg"
          << "Superman - The Inked Meat Part 2 - 0.jpg"
          << "Superman - The Inked Meat Part 2 - 33.jpg";
    ScenesMixed sMixed;
    const auto& batches = sMixed(files);
    QCOMPARE(batches.size(), 2);  // batch count = 2
    QCOMPARE(batches["Superman - The Inked Meat Part 1"].size(), 5);
    QCOMPARE(batches["Superman - The Inked Meat Part 2"].size(), 5);
    QCOMPARE(sMixed.m_json2Name.size(), 2);
    QCOMPARE(sMixed.m_vid2Name.size(), 2);

    QCOMPARE(sMixed.m_img2Name.size(), 2);
    QVERIFY(sMixed.m_img2Name.contains("Superman - The Inked Meat Part 1"));
    QVERIFY(sMixed.m_img2Name.contains("Superman - The Inked Meat Part 2"));
    QStringList part1SortedLst{"Superman - The Inked Meat Part 1.jpg", "Superman - The Inked Meat Part 1 0.jpg", "Superman - The Inked Meat Part 1 33.jpg"};
    QStringList part2SortedLst{"Superman - The Inked Meat Part 2.jpg", "Superman - The Inked Meat Part 2 - 0.jpg", "Superman - The Inked Meat Part 2 - 33.jpg"};
    QCOMPARE(sMixed.m_img2Name["Superman - The Inked Meat Part 1"], part1SortedLst);
    QCOMPARE(sMixed.m_img2Name["Superman - The Inked Meat Part 2"], part2SortedLst);
  }

  void test_1VidWithoutPartNumberOtherWith() {
    // part 1 and part 2 share a json file
    QStringList files;
    files << "Captain America - The Inked Meat.mp4"
          << "Captain America - The Inked Meat.json"
          << "Captain America - The Inked Meat.jpg"
          << "Captain America - The Inked Meat 0.jpg"
          << "Captain America - The Inked Meat 33.jpg"
          << "Captain America - The Inked Meat Part 2.mp4"
          << "Captain America - The Inked Meat Part 2 0.jpg"
          << "Captain America - The Inked Meat Part 2 33.jpg";
    ScenesMixed sMixed;
    const auto& batches = sMixed(files);
    QCOMPARE(batches.size(), 2);  // batch count = 2
    QCOMPARE(batches["Captain America - The Inked Meat"].size(), 5);
    QCOMPARE(batches["Captain America - The Inked Meat Part 2"].size(), 3);
    QCOMPARE(sMixed.m_json2Name.size(), 1);
    QCOMPARE(sMixed.m_vid2Name.size(), 2);
    QCOMPARE(sMixed.m_img2Name.size(), 2);

    QVERIFY(sMixed.m_json2Name.contains("Captain America - The Inked Meat"));
    QVERIFY(sMixed.m_vid2Name.contains("Captain America - The Inked Meat"));
    QVERIFY(sMixed.m_vid2Name.contains("Captain America - The Inked Meat Part 2"));
    QVERIFY(sMixed.m_img2Name.contains("Captain America - The Inked Meat"));
    QVERIFY(sMixed.m_img2Name.contains("Captain America - The Inked Meat Part 2"));

    QVERIFY(sMixed.m_vid2Name.contains("Captain America - The Inked Meat Part 2"));
    QVERIFY(sMixed.m_img2Name.contains("Captain America - The Inked Meat Part 2"));
  }

  void test_VidAndFoldersMixed() {
    QStringList vidAndFolders;
    vidAndFolders << "a078d0708b9ed65258070434c23b14cd66b34256"
                  << "a078d0708b9ed65258070434c23b14cd66b34276"
                  << "Name.mp4"
                  << "Heated 2022";
    ScenesMixed sMixed;
    const auto& folder2Items = sMixed(vidAndFolders);
    QCOMPARE(folder2Items.size(), 4);
  }

  void test_vidNameWithIndex_chop_index() {
    QStringList aMovie;
    aMovie << "MovieName - Micheal, Jensen Ankles.mp4"
           << "MovieName - Micheal, Jensen Ankles.avi"
           << "MovieName - Micheal, Jensen Ankles.mkv"
           << "MovieName - Micheal, Jensen Ankles FHD.mp4"
           << "MovieName - Micheal, Jensen Ankles 1080p.mp4"
           << "MovieName - Micheal, Jensen Ankles - HD.mp4";
    ScenesMixed sMixed;
    const auto& folder2Items = sMixed(aMovie);
    QCOMPARE(folder2Items.size(), 4);
    QVERIFY(folder2Items.contains("MovieName - Micheal, Jensen Ankles"));
    QVERIFY(folder2Items.contains("MovieName - Micheal, Jensen Ankles FHD"));
    QVERIFY(folder2Items.contains("MovieName - Micheal, Jensen Ankles 1080p"));
    QVERIFY(folder2Items.contains("MovieName - Micheal, Jensen Ankles - HD"));
  }

  void test_imgNameWithIndex_chop_index() {
    QStringList imgs;
    imgs << "Name 1.png"                          // Name
         << "Name - 1.png"                        // Name
         << "Name 1 - 1.png"                      // Name 1
         << "Name - 1 - 1.png"                    // Name - 1
         << "LE - Sporty - Malik, King.png"       // LE - Sporty - Malik, King
         << "LE - Sporty - Malik, King - 1.png"   // LE - Sporty - Malik, King
         << "LE - Sporty - Malik, King - 2.png";  // LE - Sporty - Malik, King
    ScenesMixed sMixed;
    const auto& folder2Items = sMixed(imgs);
    QCOMPARE(folder2Items.size(), 4);

    QVERIFY(folder2Items.contains("Name"));
    const QStringList nameImg{"Name 1.png", "Name - 1.png"};
    QCOMPARE(folder2Items["Name"], nameImg);

    const QStringList nameSpace1Img{"Name 1 - 1.png"};
    QVERIFY(folder2Items.contains("Name 1"));
    QCOMPARE(folder2Items["Name 1"], nameSpace1Img);

    const QStringList nameHypen1Img{"Name - 1 - 1.png"};
    QVERIFY(folder2Items.contains("Name - 1"));
    QCOMPARE(folder2Items["Name - 1"], nameHypen1Img);

    QVERIFY(folder2Items.contains("LE - Sporty - Malik, King"));
    QCOMPARE(folder2Items["LE - Sporty - Malik, King"].size(), 3);
  }
};

//QTEST_MAIN(ScenesMixedTest)
#include "ScenesMixedTest.moc"
