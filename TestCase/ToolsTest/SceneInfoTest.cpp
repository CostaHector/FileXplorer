#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"

#include "BeginToExposePrivateMember.h"
#include "SceneInfo.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"
#include <QSaveFile>

class SceneInfoTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
 private slots:
  void initTestCase() {  //
    QVERIFY(tDir.IsValid());
  }

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

  void exception_test_ok() {
    QVERIFY(tDir.ClearAll());
    using namespace SceneHelper;
    SceneInfoList marvelRootPathScenes{
        SceneInfo{"",                                                                                    //
                  "Chris\n \r\nEvans",                                                                   //
                  {"Chris Evans.jpg"},                                                                   //
                  "",                                                                                    //
                  120 * 1024 * 1024,                                                                     //
                  97,                                                                                    //
                  ""},                                                                                   //
        SceneInfo{"",                                                                                    //
                  "Michael Fassbender",                                                                  //
                  {"Michael Fassbender 0.jpg", "Michael Fassbender 1.jpg", "Michael Fassbender 2.jpg"},  //
                  {"Michael Fassbender.mp4"},                                                            //
                  290 * 1024 * 1024,                                                                     //
                  99,                                                                                    //
                  ""},                                                                                   //
    };
    QVERIFY(marvelRootPathScenes.size() > 1);

    const QString scnAbsFilePath{tDir.itemPath("exception_protect.scn")};

    {
      // 0. 测试空文件
      QSaveFile saveFi(scnAbsFilePath);
      QVERIFY(saveFi.open(QIODevice::WriteOnly));
      QVERIFY(saveFi.commit());
      SceneInfoList result = ParseAScnFile(scnAbsFilePath, "/");
      QVERIFY(result.isEmpty());
    }

    {
      // 1. 文件魔数错误
      decltype(SceneInfo::MAGIC_NUMBER) INVALID_MAGIC_NUMBER{9956323};

      QSaveFile saveFi(scnAbsFilePath);
      QVERIFY(saveFi.open(QIODevice::WriteOnly));
      QDataStream iStream{&saveFi};
      iStream.setVersion(QDataStream::Qt_5_15);
      iStream << INVALID_MAGIC_NUMBER;        // invalid "LMSC" magic
      iStream << SceneInfo::CURRENT_VERSION;  //
      iStream << (SceneInfo::ELEMENT_COUNT_TYPE)(marvelRootPathScenes.size());
      for (const auto& scene : marvelRootPathScenes) {
        iStream << scene;
      }
      QVERIFY(saveFi.commit());

      SceneInfoList result = ParseAScnFile(scnAbsFilePath, "/");
      QVERIFY(result.isEmpty());
    }

    {
      // 2. 版本过低;
      decltype(SceneInfo::CURRENT_VERSION) INVALID_VERSION{0};

      QSaveFile saveFi(scnAbsFilePath);
      QVERIFY(saveFi.open(QIODevice::WriteOnly));
      QDataStream iStream{&saveFi};
      iStream.setVersion(QDataStream::Qt_5_15);
      iStream << SceneInfo::MAGIC_NUMBER;
      iStream << INVALID_VERSION;  // invalid version < SceneInfo::CURRENT_VERSION
      iStream << (SceneInfo::ELEMENT_COUNT_TYPE)(marvelRootPathScenes.size());
      for (const auto& scene : marvelRootPathScenes) {
        iStream << scene;
      }
      QVERIFY(saveFi.commit());

      SceneInfoList result = ParseAScnFile(scnAbsFilePath, "/");
      QVERIFY(result.isEmpty());
    }

    {
      // 3.1 内容(填写的元素数量少于实际数量); 读取数量<=std::min(填写值, 实际值);
      constexpr SceneInfo::ELEMENT_COUNT_TYPE ELEMENT_COUNT_1 = 1;

      QSaveFile saveFi(scnAbsFilePath);
      QVERIFY(saveFi.open(QIODevice::WriteOnly));
      QDataStream iStream{&saveFi};
      iStream.setVersion(QDataStream::Qt_5_15);

      iStream << SceneInfo::MAGIC_NUMBER;
      iStream << SceneInfo::CURRENT_VERSION;  // invalid version
      iStream << ELEMENT_COUNT_1;
      for (const auto& scene : marvelRootPathScenes) {
        iStream << scene;
      }
      QVERIFY(saveFi.commit());

      SceneInfoList result = ParseAScnFile(scnAbsFilePath, "/");
      QCOMPARE(result.size(), 1);
    }

    {
      // 3.2 内容(填写的元素数量大于实际数量); 读取数量<=std::min(填写值, 实际值);
      constexpr SceneInfo::ELEMENT_COUNT_TYPE ELEMENT_COUNT_10 = 10;

      QSaveFile saveFi(scnAbsFilePath);
      QVERIFY(saveFi.open(QIODevice::WriteOnly));
      QDataStream iStream{&saveFi};
      iStream.setVersion(QDataStream::Qt_5_15);

      iStream << SceneInfo::MAGIC_NUMBER;
      iStream << SceneInfo::CURRENT_VERSION;  // invalid version
      iStream << ELEMENT_COUNT_10;
      for (const auto& scene : marvelRootPathScenes) {
        iStream << scene;
      }
      QVERIFY(saveFi.commit());

      SceneInfoList result = ParseAScnFile(scnAbsFilePath, "/");
      QCOMPARE(result.size(), marvelRootPathScenes.size());
    }

    {
      // 4. 内容(结构体不匹配)
      QSaveFile saveFi(scnAbsFilePath);
      QVERIFY(saveFi.open(QIODevice::WriteOnly));
      QDataStream iStream{&saveFi};
      iStream.setVersion(QDataStream::Qt_5_15);

      iStream << SceneInfo::MAGIC_NUMBER;
      iStream << SceneInfo::CURRENT_VERSION;                       // invalid version
      iStream << (SceneInfo::ELEMENT_COUNT_TYPE)(1);               // only one element
      iStream << QString{"AbcRel"} << QString{"Name"} << int(99);  // rel, name, rate not the struct we need
      QVERIFY(saveFi.commit());

      SceneInfoList result = ParseAScnFile(scnAbsFilePath, "/");
      QVERIFY(result.isEmpty());
    }

    {
      // 5. 读取方式错误使用文本则0个元素, 使用正确的二进制方式则2个元素
      QSaveFile saveFi(scnAbsFilePath);
      QVERIFY(saveFi.open(QIODevice::WriteOnly));
      QDataStream iStream{&saveFi};
      iStream.setVersion(QDataStream::Qt_5_15);

      iStream << SceneInfo::MAGIC_NUMBER;
      iStream << SceneInfo::CURRENT_VERSION;                                    // invalid version
      iStream << (SceneInfo::ELEMENT_COUNT_TYPE)(marvelRootPathScenes.size());  // only one element
      for (const auto& scene : marvelRootPathScenes) {
        iStream << scene;
      }
      QVERIFY(saveFi.commit());

      SceneInfoList result = ParseAScnFile(scnAbsFilePath, "/");
      QCOMPARE(result.size(), 2);
      QCOMPARE(result[0].rel2scn, "/");
      QCOMPARE(result[1].rel2scn, "/");
      result[0].rel2scn = "";
      result[1].rel2scn = "";
      std::sort(result.begin(), result.end(), [](const SceneInfo& lhs, const SceneInfo& rhs) -> bool {  //
        return lhs.lessThanName(rhs);
      });
      QCOMPARE(result, marvelRootPathScenes);
    }
  }

  void SaveAndRead_ok() {
    QVERIFY(tDir.ClearAll());
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

    QString rootScnAbsFilePath = tDir.itemPath(tDir.baseName() + ".scn");
    QString forbesScnAbsFilePath = tDir.itemPath("Forbes/Forbes.scn");

    QVERIFY(SaveScenesListToBinaryFile(rootScnAbsFilePath, marvelRootPathScenes));
    QVERIFY(!SaveScenesListToBinaryFile(forbesScnAbsFilePath, forbesXMenPathScenes));  // folder not exists
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
  // todo: add exception for rewrite rate value
};

#include "SceneInfoTest.moc"
REGISTER_TEST(SceneInfoTest, false)
