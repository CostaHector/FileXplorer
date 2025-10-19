#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "SceneMixed.h"

class ScenesMixedTest : public PlainTestSuite {
  Q_OBJECT
public:
  ScenesMixedTest() : PlainTestSuite{} {}
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
    const QStringList imgsSorted{"X-Man - The Last Stand.jpg",    //
                                 "X-Man - The Last Stand.png",    //
                                 "X-Man - The Last Stand 1.png",  //
                                 "X-Man - The Last Stand 33.png"};
    QCOMPARE(sMixed.m_img2Name["X-Man - The Last Stand"], imgsSorted);
  }

  void test_video_without_part_number() {
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

  void test_video_poster_images_sceenshot_images() {
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

  void test_folder2_contains_json_no_need_combined() {
    // folder 2 start with folder 1
    // both folder 1 and folder 2 have a json file, no need combine
    QStringList files;
    files << "Captain America - Henry Carvill, Chris Evans.json"
          << "Captain America - Henry Carvill, Chris Evans.mp4"
          << "Captain America - Henry Carvill, Chris Evans.jpg"
          << "Captain America - Henry Carvill, Chris Evans 1.jpg"
          << "Captain America - Henry Carvill, Chris Evans 2.jpg"
          << "Captain America - Henry Carvill, Chris Evans Part 2.json"
          << "Captain America - Henry Carvill, Chris Evans Part 2.mp4";
    ScenesMixed sMixed;
    const auto& batches = sMixed(files);
    QCOMPARE(batches.size(), 2);  // batch count = 2
    QCOMPARE(batches["Captain America - Henry Carvill, Chris Evans"].size(), 5);
    QCOMPARE(batches["Captain America - Henry Carvill, Chris Evans Part 2"].size(), 2);
    QCOMPARE(sMixed.m_json2Name.size(), 2);
    QCOMPARE(sMixed.m_vid2Name.size(), 2);
    QCOMPARE(sMixed.m_img2Name.size(), 1);
  }

  void test_folder2_contains_no_json_combined_2_front_ok() {
    // folder 2 start with folder 1
    // folder 1, folder 2 share a json file
    QStringList files;
    files << "Captain America - Henry Carvill, Chris Evans.json"
          << "Captain America - Henry Carvill, Chris Evans.mp4"
          << "Captain America - Henry Carvill, Chris Evans.jpg"
          << "Captain America - Henry Carvill, Chris Evans 1.jpg"
          << "Captain America - Henry Carvill, Chris Evans 2.jpg"
          << "Captain America - Henry Carvill, Chris Evans Part 2.mp4";
    ScenesMixed sMixed;
    const auto& batches = sMixed(files);
    QCOMPARE(batches.size(), 1);  // batch count = 2
    QCOMPARE(batches["Captain America - Henry Carvill, Chris Evans"].size(), 6);
    QCOMPARE(sMixed.m_json2Name.size(), 1);
    QCOMPARE(sMixed.m_vid2Name.size(), 1);
    QCOMPARE(sMixed.m_img2Name.size(), 1);

    QVERIFY(sMixed.m_json2Name.contains("Captain America - Henry Carvill, Chris Evans"));
    QVERIFY(sMixed.m_vid2Name.contains("Captain America - Henry Carvill, Chris Evans"));
    QVERIFY(sMixed.m_img2Name.contains("Captain America - Henry Carvill, Chris Evans"));
  }

  void test_videos_and_folder_mixed() {
    QStringList vidAndFolders;
    vidAndFolders << "a078d0708b9ed65258070434c23b14cd66b34256"
                  << "a078d0708b9ed65258070434c23b14cd66b34276"
                  << "Name.mp4"
                  << "Heated 2022";
    ScenesMixed sMixed;
    const auto& folder2Items = sMixed(vidAndFolders);
    QCOMPARE(folder2Items.size(), 4);
  }

  void test_folder_without_json_combined_to_front_ok() {
    QStringList aMovie;
    aMovie << "MovieName - Micheal, Jensen Ankles.mp4"
           << "MovieName - Micheal, Jensen Ankles.avi"
           << "MovieName - Micheal, Jensen Ankles.mkv"
           << "MovieName - Micheal, Jensen Ankles FHD.mp4"
           << "MovieName - Micheal, Jensen Ankles 1080p.mp4"
           << "MovieName - Micheal, Jensen Ankles - HD.mp4";
    ScenesMixed sMixed;
    const auto& folder2Items = sMixed(aMovie);
    QCOMPARE(folder2Items.size(), 1);
    QVERIFY(folder2Items.contains("MovieName - Micheal, Jensen Ankles"));
    QCOMPARE(folder2Items["MovieName - Micheal, Jensen Ankles"].size(), 6);
  }

  void test_imgName_with_index_chop_index() {
    QStringList imgs;
    imgs << "Name 1.png"                                  // Name
         << "Name - 1.png"                                // Name
         << "Name 1 - 1.png"                              // Name 1
         << "Name - 1 - 1.png"                            // Name - 1
         << "Fox - Sporty - Chris Evans, Henry.png"       // Fox - Sporty - Chris Evans, Henry
         << "Fox - Sporty - Chris Evans, Henry - 1.png"   // Fox - Sporty - Chris Evans, Henry
         << "Fox - Sporty - Chris Evans, Henry - 2.png";  // Fox - Sporty - Chris Evans, Henry
    ScenesMixed sMixed;
    const auto& folder2Items = sMixed(imgs);
    QCOMPARE(folder2Items.size(), 4);
    const QMap<QString, QStringList>& expectsFolder2Items //
        {
         {"Name", {"Name 1.png", "Name - 1.png"}},
         {"Name 1", {"Name 1 - 1.png"}},
         {"Name - 1", {"Name - 1 - 1.png"}},
         {"Fox - Sporty - Chris Evans, Henry",
          {"Fox - Sporty - Chris Evans, Henry.png",
           "Fox - Sporty - Chris Evans, Henry - 1.png",
           "Fox - Sporty - Chris Evans, Henry - 2.png"}},
         };
    QCOMPARE(folder2Items, expectsFolder2Items);
  }

  void test_special_scenario_group_test() {
    // folder also in stringlist
    const QStringList items {
        "H",                          // situation 1 file with a json need group
        "H.C.jpg",                    //
        "H.C.json",                   //
        "Michael",                    // situation 2 file without json need group
        "Michael Fassbender.jpg",     //
        "Michael Fassbender.mp4",     //
#ifdef _WIN32
        "C.R",                        // situation 3 file without json and baseName endswith dot need group
#else
        "C.R.",                       //
#endif
        "C.R..jpg",
    };
    ScenesMixed sMixed;
    const QMap<QString, QStringList>& folder2Items = sMixed(items);
    const QMap<QString, QStringList>& expectsFolder2Items {
        {"H", {"H"}},
        {"H.C", {"H.C.json", "H.C.jpg"}}, // image append behind json, behind videos
        {"Michael", {"Michael"}},
        {"Michael Fassbender", {"Michael Fassbender.mp4", "Michael Fassbender.jpg"}},
#ifdef _WIN32
        {"C", {"C.R"}},
        {"C.R.", {"C.R..jpg"}},
#else
        {"C.R.", {"C.R.", "C.R..jpg"}},
#endif
    };
    QCOMPARE(folder2Items.size(), expectsFolder2Items.size());
    QCOMPARE(folder2Items, expectsFolder2Items);
  }
};

#include "ScenesMixedTest.moc"
REGISTER_TEST(ScenesMixedTest, false)
