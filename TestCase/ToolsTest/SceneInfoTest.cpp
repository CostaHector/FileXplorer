#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"

#include "BeginToExposePrivateMember.h"
#include "SceneInfo.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

class SceneInfoTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void basic_memeber_test() {
    const SceneInfo lhs{"", "Britain", {"img1", "img2"}, "vid", 100, 99, "2000"};
    const SceneInfo lhs_pretend_deep_copy{lhs};
    QCOMPARE(lhs, lhs_pretend_deep_copy);

    const SceneInfo rhs{"/", "Britain", {"img1", "img2"}, "vid", 100, 99, "2000"};
    QVERIFY(lhs < rhs);  // rel2scn differ

    const SceneInfo rhs2{"", "America", {"img1", "img2"}, "vid", 100, 99, "2000"};
    QVERIFY(!(lhs < rhs2));  // rel2scn equal, but name differ

    QByteArray buffer;

    QDataStream wstream{&buffer, QIODevice::WriteOnly};
    wstream << lhs;
    QCOMPARE(wstream.status(), QDataStream::Status::Ok);
    QVERIFY(buffer.size() > 0);

    QDataStream rstream{&buffer, QIODevice::ReadOnly};
    SceneInfo recoverFromStream;
    rstream >> recoverFromStream;
    QCOMPARE(rstream.status(), QDataStream::Status::Ok);
    QCOMPARE(lhs, recoverFromStream);
  }

  void scene_sort_function_ok() {
    SceneInfo si1{"/", "The U.S.", {}, {}, 100 * 1024, 98, "1980"};
    SceneInfo si2{"/", "France", {}, {}, 200 * 1024, 95, "1960"};
    SceneInfo si3{"/Asia", "Singapore", {}, {}, 150 * 1024, 96, "2000"};

    SceneInfoList siList{si1, si2, si3};

    // name: France(2) < Singapore(3) < The U.S.(1)
    std::sort(siList.begin(), siList.end(), [](const SceneInfo& lhs, const SceneInfo& rhs) -> bool {  //
      return lhs.lessThanName(rhs);
    });
    QCOMPARE(siList, (SceneInfoList{si2, si3, si1}));

    // / and France(2) < / and The U.S.(1) < /Asia and Singapore(3)

    std::sort(siList.begin(), siList.end(), [](const SceneInfo& lhs, const SceneInfo& rhs) -> bool {
      auto pComparator = SceneInfo::getCompareFunc(SceneSortOrderHelper::SortDimE::MOVIE_PATH);
      return (lhs.*pComparator)(rhs);
    });
    QCOMPARE(siList, (SceneInfoList{si2, si1, si3}));

    // 100k(1) < 150k(3) < 200k(2)
    std::sort(siList.begin(), siList.end(), [](const SceneInfo& lhs, const SceneInfo& rhs) -> bool {
      auto pComparator = SceneInfo::getCompareFunc(SceneSortOrderHelper::SortDimE::MOVIE_SIZE);
      return (lhs.*pComparator)(rhs);
    });
    QCOMPARE(siList, (SceneInfoList{si1, si3, si2}));

    // 95(2) < 96(3) < 98(1)
    std::sort(siList.begin(), siList.end(), [](const SceneInfo& lhs, const SceneInfo& rhs) -> bool {
      auto pComparator = SceneInfo::getCompareFunc(SceneSortOrderHelper::SortDimE::RATE);
      return (lhs.*pComparator)(rhs);
    });
    QCOMPARE(siList, (SceneInfoList{si2, si3, si1}));

    // 1960(2) < 1980(1) < 2000(3)
    std::sort(siList.begin(), siList.end(), [](const SceneInfo& lhs, const SceneInfo& rhs) -> bool {
      auto pComparator = SceneInfo::getCompareFunc(SceneSortOrderHelper::SortDimE::UPLOADED_TIME);
      return (lhs.*pComparator)(rhs);
    });
    QCOMPARE(siList, (SceneInfoList{si2, si1, si3}));
  }

  void SaveAndRead_ok() {
    using namespace SceneHelper;

    SceneInfoList marvelRootPathScenes{
        SceneInfo{"",                                            //
                  "Chris Evans",                                 //
                  {"Chris Evans.jpg"},                           //
                  "",                                            //
                  120 * 1024 * 1024,                             //
                  97,                                            //
                  ""},                                           //
        SceneInfo{"",                                            //
                  "Henry Cavill",                                //
                  {"Henry Cavill 1.jpg", "Henry Cavill 2.jpg"},  //
                  {"Henry Cavill Biology.mp4"},                  //
                  140 * 1024 * 1024,                             //
                  99,                                            //
                  ""},                                           //
    };
    SceneInfoList forbesXMenPathScenes{
        SceneInfo{"",                 //
                  "Jane Grey",        //
                  {"Jane Grey.jpg"},  //
                  "Jane Grey.mp4",    //
                  100 * 1024 * 1024,  //
                  75,                 //
                  ""},                //
    };

    TDir tDir;
    QVERIFY(tDir.IsValid());

    QString rootScnAbsFilePath = tDir.itemPath(tDir.baseName() + ".scn");
    QString forbesScnAbsFilePath = tDir.itemPath("Forbes/Forbes.scn");

    QVERIFY(SaveScenesListToBinaryFile(rootScnAbsFilePath, marvelRootPathScenes));
    QVERIFY(!SaveScenesListToBinaryFile(forbesScnAbsFilePath, forbesXMenPathScenes)); // folder not exists
    QVERIFY(tDir.mkdir("Forbes"));
    QVERIFY(SaveScenesListToBinaryFile(forbesScnAbsFilePath, forbesXMenPathScenes));

    SceneInfoList scenesInForbes = GetScnsLstFromPath(tDir.itemPath("Forbes"));
    QCOMPARE(scenesInForbes.size(), 1);
    QCOMPARE(scenesInForbes.front().rel2scn, "/");
    scenesInForbes.front().rel2scn = "";
    QCOMPARE(scenesInForbes, forbesXMenPathScenes);

    SceneInfoList allScenesInRoot = GetScnsLstFromPath(tDir.path());
    QCOMPARE(allScenesInRoot.size(), 2 + 1);
    std::sort(allScenesInRoot.begin(), allScenesInRoot.end(), [](const SceneInfo& lhs, const SceneInfo& rhs) -> bool {  //
      return lhs.lessThanName(rhs);
    });
    QCOMPARE(allScenesInRoot[0].rel2scn, "/");
    QCOMPARE(allScenesInRoot[1].rel2scn, "/");
    QCOMPARE(allScenesInRoot[2].rel2scn, "/Forbes/");
    allScenesInRoot[0].rel2scn = "";
    allScenesInRoot[1].rel2scn = "";
    allScenesInRoot[2].rel2scn = "";
    SceneInfoList expectAllScenes;
    expectAllScenes += marvelRootPathScenes;
    expectAllScenes += forbesXMenPathScenes;
    QCOMPARE(allScenesInRoot, expectAllScenes);
  }
};

#include "SceneInfoTest.moc"
REGISTER_TEST(SceneInfoTest, false)
