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
    const QStringList files{
        // 1 vid + 1 json + 5 imgs
        "X-Man - The Last Stand.mp4",     //
        "X-Man - The Last Stand.json",    //
        "X-Man - The Last Stand.jpg",     //
        "X-Man - The Last Stand.png",     //
        "X-Man - The Last Stand 33.png",  //
        "X-Man - The Last Stand 1.png",   //
        "X-Man - The Last Stand - 2.png", //
    };

    ScenesMixed sMixed;
    const ScenesMixed::GROUP_MAP_TYPE& grps = sMixed(files);
    QCOMPARE(grps.size(), 1); // batch count = 1
    QCOMPARE(grps["X-Man - The Last Stand"].size(), 1 + 1 + 5);
    QCOMPARE(sMixed.m_json2Name.size(), 1);
    QCOMPARE(sMixed.m_vid2Name.size(), 1);
    QCOMPARE(sMixed.m_img2Name.size(), 1);

    QCOMPARE(sMixed.m_json2Name["X-Man - The Last Stand"], "X-Man - The Last Stand.json");
    QCOMPARE(sMixed.m_vid2Name["X-Man - The Last Stand"].size(), 1);
    // in ascii table ' ' = 0x20, ' ' = 0x2E
    // but here we want length shorter rather than ascii less
    const QStringList imgsSorted{
        "X-Man - The Last Stand.jpg",   //
        "X-Man - The Last Stand.png",   //
        "X-Man - The Last Stand 1.png", //
        "X-Man - The Last Stand 33.png",
        "X-Man - The Last Stand - 2.png", //
    };
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
    const ScenesMixed::GROUP_MAP_TYPE& grps = sMixed(files);
    QCOMPARE(grps.size(), 2); // batch count = 2
    QCOMPARE(grps["Tarzan"].size(), 8);
    QCOMPARE(grps["another Tarzan"].size(), 8);
    QCOMPARE(sMixed.m_img2Name["Tarzan"].size(), 5);
    QCOMPARE(sMixed.m_img2Name["Tarzan"][0], "Tarzan.jpg");
    QCOMPARE(sMixed.m_vid2Name["Tarzan"].size(), 2);
    QCOMPARE(sMixed.m_json2Name["Tarzan"], "Tarzan.json");

    QCOMPARE(sMixed.m_img2Name["another Tarzan"].size(), 5);
    QCOMPARE(sMixed.m_img2Name["another Tarzan"][0], "another Tarzan.jpg");
    QCOMPARE(sMixed.m_vid2Name["another Tarzan"].size(), 2);
    QCOMPARE(sMixed.m_json2Name["another Tarzan"], "another Tarzan.json");
  }

  void test_video_with_part_and_images() {
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
    const ScenesMixed::GROUP_MAP_TYPE& grps = sMixed(files);
    QCOMPARE(grps.size(), 2); // batch count = 2
    const QStringList expectGrpNames{"Superman - The Inked Meat Part 1", "Superman - The Inked Meat Part 2"};
    QCOMPARE(grps.keys(), expectGrpNames);
    QCOMPARE(grps["Superman - The Inked Meat Part 1"].size(), 5);
    QCOMPARE(grps["Superman - The Inked Meat Part 2"].size(), 5);

    QCOMPARE(sMixed.m_json2Name.keys(), expectGrpNames);
    QCOMPARE(sMixed.m_vid2Name.keys(), expectGrpNames);
    QCOMPARE(sMixed.m_img2Name.keys(), expectGrpNames);

    const QStringList expectPart1SortedLst{"Superman - The Inked Meat Part 1.jpg",
                                           "Superman - The Inked Meat Part 1 0.jpg",
                                           "Superman - The Inked Meat Part 1 33.jpg"};
    const QStringList expectPart2SortedLst{"Superman - The Inked Meat Part 2.jpg",
                                           "Superman - The Inked Meat Part 2 - 0.jpg",
                                           "Superman - The Inked Meat Part 2 - 33.jpg"};
    QCOMPARE(sMixed.m_img2Name["Superman - The Inked Meat Part 1"], expectPart1SortedLst);
    QCOMPARE(sMixed.m_img2Name["Superman - The Inked Meat Part 2"], expectPart2SortedLst);
  }

  void test_grp2_contains_json_no_need_combined_to_grp() {
    // grp2 name start with grp1 name, and only differ with Part2
    // grp2 contains json. no combine need
    QStringList files;
    files << "Captain America - Henry Carvill, Chris Evans.mp4"
          << "Captain America - Henry Carvill, Chris Evans.jpg"
          << "Captain America - Henry Carvill, Chris Evans 1.jpg"
          << "Captain America - Henry Carvill, Chris Evans 2.jpg"
          << "Captain America - Henry Carvill, Chris Evans Part 2.json"
          << "Captain America - Henry Carvill, Chris Evans Part 2.mp4";
    ScenesMixed sMixed;
    const ScenesMixed::GROUP_MAP_TYPE& grps = sMixed(files);
    QCOMPARE(grps.size(), 2); // batch count = 2
    const QStringList expectGrpNames{"Captain America - Henry Carvill, Chris Evans", "Captain America - Henry Carvill, Chris Evans Part 2"};
    QCOMPARE(grps.keys(), expectGrpNames);
    QCOMPARE(grps["Captain America - Henry Carvill, Chris Evans"].size(), 4);
    QCOMPARE(grps["Captain America - Henry Carvill, Chris Evans Part 2"].size(), 2);

    QCOMPARE(sMixed.m_json2Name.keys(), (QStringList{"Captain America - Henry Carvill, Chris Evans Part 2"}));
    QCOMPARE(sMixed.m_vid2Name.keys(), expectGrpNames);
    QCOMPARE(sMixed.m_img2Name.size(), 1);
  }

  void grp2_contains_no_json_combined_to_grp_ok_differ_ignored() {
    // grp2 startwith grp1
    // grp2 contains no json
    // grp2.mid(grp1.size()) = "part" or "sc" or "hd"
    QStringList files;
    files << "Captain America - Henry Carvill, Chris Evans.json"
          << "Captain America - Henry Carvill, Chris Evans.mp4"
          << "Captain America - Henry Carvill, Chris Evans.jpg"
          << "Captain America - Henry Carvill, Chris Evans 1.jpg"
          << "Captain America - Henry Carvill, Chris Evans 2.jpg"
          << "Captain America - Henry Carvill, Chris Evans Part 2.mp4"
          << "Captain America - Henry Carvill, Chris Evans Pt 2.mp4"
          << "Captain America - Henry Carvill, Chris Evans Scene 3.mkv"
          << "Captain America - Henry Carvill, Chris Evans Sc 3.mkv"
          << "Captain America - Henry Carvill, Chris Evans HD.avi"
          << "Captain America - Henry Carvill, Chris Evans HD 1.jpg";
    ScenesMixed sMixed;
    const ScenesMixed::GROUP_MAP_TYPE& grps = sMixed(files);
    QCOMPARE(grps.size(), 1); // batch count = 1
    QCOMPARE(grps["Captain America - Henry Carvill, Chris Evans"].size(), 11);
    const QStringList expectGrpsNames{"Captain America - Henry Carvill, Chris Evans"};
    QCOMPARE(sMixed.m_json2Name.keys(), expectGrpsNames);
    QCOMPARE(sMixed.m_vid2Name.keys(), expectGrpsNames);
    QCOMPARE(sMixed.m_img2Name.keys(), expectGrpsNames);

    QCOMPARE(sMixed.m_json2Name["Captain America - Henry Carvill, Chris Evans"], "Captain America - Henry Carvill, Chris Evans.json");
    QCOMPARE(sMixed.m_vid2Name["Captain America - Henry Carvill, Chris Evans"].size(), 6);
    QCOMPARE(sMixed.m_img2Name["Captain America - Henry Carvill, Chris Evans"].size(), 4);
  }

  void folder_mixed_no_need_combine_at_all() {
    const QStringList vidAndFolders{
        "a078d0708b9ed65258070434c23b14cd66b34256",
        "a078d0708b9ed65258070434c23b14cd66b34276",
        "Name.mp4",
        "Heated 2022",
        "lam 3.jpg",
        "beyond.json",
    };
    ScenesMixed sMixed;
    const ScenesMixed::GROUP_MAP_TYPE& folder2Items = sMixed(vidAndFolders);
    QCOMPARE(folder2Items.size(), 6);
  }

  void test_imgName_with_index_chop_index() {
    QStringList imgs;
    imgs << "Name 1.png"                                 // Name
         << "Name - 1.png"                               // Name
         << "Name 1 - 1.png"                             // Name 1
         << "Name - 1 - 1.png"                           // Name - 1
         << "Fox - Sporty - Chris Evans, Henry.png"      // Fox - Sporty - Chris Evans, Henry
         << "Fox - Sporty - Chris Evans, Henry - 1.png"  // Fox - Sporty - Chris Evans, Henry
         << "Fox - Sporty - Chris Evans, Henry - 2.png"; // Fox - Sporty - Chris Evans, Henry
    ScenesMixed sMixed;
    const auto& folder2Items = sMixed(imgs);
    QCOMPARE(folder2Items.size(), 4);
    const ScenesMixed::GROUP_MAP_TYPE& expectsFolder2Items //
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
    const QStringList items{
        "H",                      // situation 1 file with a json need group
        "H.C.jpg",                //
        "H.C.json",               //
        "Michael",                // situation 2 file without json need group
        "Michael Fassbender.jpg", //
        "Michael Fassbender.mp4", //
#ifdef _WIN32
        "C.R", // situation 3 file without json and baseName endswith dot need group
#else
        "C.R.", //
#endif
        "C.R..jpg",
    };
    ScenesMixed sMixed;
    const ScenesMixed::GROUP_MAP_TYPE& folder2Items = sMixed(items);
    const ScenesMixed::GROUP_MAP_TYPE& expectsFolder2Items{
        {"H", {"H"}},
        {"H.C", {"H.C.json", "H.C.jpg"}},  // image append behind json, behind videos
        {"Michael", {"Michael"}},
        {"Michael Fassbender", {"Michael Fassbender.mp4", "Michael Fassbender.jpg"}},
#ifdef _WIN32
        {"C",
         { "C.R" }},
        {"C.R.",
         { "C.R..jpg" }},
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
