// #include <QtTest/QtTest>
// #include "PlainTestSuite.h"
// #include "OnScopeExit.h"
// #include <QTestEventList>
// #include <QSignalSpy>

// #include "Logger.h"
// #include "MemoryKey.h"
// #include "BeginToExposePrivateMember.h"
// #include "ScenesListModel.h"
// #include "SceneSortProxyModel.h"
// #include "EndToExposePrivateMember.h"

// #include "SceneInfoManager.h"
// #include "ImageTestPrecoditionTools.h"
// #include "StringTool.h"
// #include <QDir>
// #include <QDirIterator>
// #include "TDir.h"

// #include "JsonTestPrecoditionTools.h"
// #include "JsonKey.h"
// #include "RateHelper.h"
// #include <random>

// #include <mockcpp/mokc.h>
// #include <mockcpp/GlobalMockObject.h>
// #include <mockcpp/MockObject.h>
// #include <mockcpp/MockObjectHelper.h>
// USING_MOCKCPP_NS

// using namespace ImageTestPrecoditionTools;
// using namespace SceneInfoManager;

// bool checkIndexMatch(const QAbstractListModel& srcModel, int srcR, const QSortFilterProxyModel& proModel, int proR) {
//   QVariant src = srcModel.data(srcModel.index(srcR), Qt::DisplayRole);
//   QVariant dst = proModel.data(proModel.index(proR, 0), Qt::DisplayRole);
//   if (src != dst) {
//     LOG_D("src:[%s], pro:[%s]", qPrintable(src.toString()), qPrintable(dst.toString()));
//     return false;
//   }
//   return true;
// }

// bool GetSceneInfoList(int count, SceneSortOrderHelper::SortDimE sortDim, bool isReverse, SceneInfoList& ansList) {
//   ansList.clear();
//   ansList.reserve(count);

//   std::uniform_int_distribution<int> uniformIntDist(0, count + 1);
//   std::default_random_engine e;
//   SceneInfo siBasic{"/", "Ben Affleck", {}, {"Ben Affleck.mp4"}, 100 * 1024 * 1024, 10, "2000-06-16 00:00:60"};
//   for (int i = 0; i < count; ++i) {
//     // 所有dimension都是统一的无序
//     int rndInt = uniformIntDist(e);
//     QString rnd = QString::number(rndInt);
//     siBasic.name = "name" + rnd;
//     siBasic.vidSize = rndInt * 1000;
//     siBasic.rate = rndInt * 100;
//     siBasic.uploaded = "uploaded" + rnd;
//     // 只有入参指定一个dimension有序
//     const int valueForDistinguish = isReverse ? count - i - 1 : i;
//     switch (sortDim) {
//       case SceneSortOrderHelper::SortDimE::MOVIE_PATH: {
//         siBasic.name = QString::asprintf("Name %03d", valueForDistinguish);
//         break;
//       }
//       case SceneSortOrderHelper::SortDimE::MOVIE_SIZE: {
//         siBasic.vidSize = valueForDistinguish;
//         break;
//       }
//       case SceneSortOrderHelper::SortDimE::RATE: {
//         siBasic.rate = valueForDistinguish;
//         break;
//       }
//       case SceneSortOrderHelper::SortDimE::UPLOADED_TIME: {
//         siBasic.uploaded = QString::asprintf("2010-06-16 00:00:00.%03d", valueForDistinguish);
//         break;
//       }
//       default:
//         LOG_E("unsupport: %s", SceneSortOrderHelper::c_str(sortDim));
//         return false;
//     }
//     ansList.push_back(siBasic);
//   }
//   return true;
// }

// bool isValidPixmap(const QVariant& pixmapVar) {
//   return !pixmapVar.isNull() && pixmapVar.canConvert<QPixmap>() && !pixmapVar.value<QPixmap>().isNull();
// }

// class ScenesListModelTest : public PlainTestSuite {
//   Q_OBJECT
//  public:
//   TDir mTDir;
//   QString lastFight1SvgContents = GetSvgContentTemplate().arg(SVG_FILL_COLORS[0]).arg("The Last Fight 1");
//   QString ironMenSvgContents = GetSvgContentTemplate().arg(SVG_FILL_COLORS[2]).arg("Iron Man");

//   QList<FsNodeEntry> nodeBamboo;
//   QList<FsNodeEntry> nodeGrapes;

//   const QString bambooPath = mTDir.itemPath("Bamboo");
//   const QString grapePath = mTDir.itemPath("Grapes");

//  private slots:
//   void initTestCase() {
//     Configuration().clear();

//     QVERIFY(mTDir.IsValid());
//     QVERIFY(SVG_FILL_COLORS_COUNT > 3);

//     nodeBamboo =  //
//         {
//             {"Bamboo/The Last Fight.json", false, JsonKey::ConstructJsonByteArray("The Last Fight")},             //
//             {"Bamboo/The Last Fight 1.jpg", false, lastFight1SvgContents.toUtf8()},                               //
//             {"Bamboo/The Last Fight 2.jpg", false, ""},                                                           //
//             {"Bamboo/The Last Fight.mp4", false, ""},                                                             //
//             {"Bamboo/Avengers/Iron Man.json", false, JsonKey::ConstructJsonByteArray("Iron Man")},                //
//             {"Bamboo/Avengers/Iron Man.jpg", false, ironMenSvgContents.toUtf8()},                                 //
//             {"Bamboo/Avengers/Iron Man.mp4", false, ""},                                                          //
//             {"Bamboo/Avengers/Captain America.json", false, JsonKey::ConstructJsonByteArray("Captain America")},  //
//             {"Bamboo/Avengers/Captain America.mp4", false, ""},                                                   //
//         };
//     QCOMPARE(mTDir.createEntries(nodeBamboo), nodeBamboo.size());  // 3json
//     {
//       ScnMgr scnMgr1;
//       QCOMPARE(scnMgr1(bambooPath), Counter(3, 3, 3, 2));  // 3updated, 3used,3VidUpdated,2ImgNameUpdated
//       QCOMPARE(ScnMgr::UpdateScnFiles(bambooPath), 2);     // 2 scn file writed ok
//     }

//     nodeGrapes =  //                                                                                       //
//         {
//             {"Grapes/The Last Fight.json", false, JsonKey::ConstructJsonByteArray("The Last Fight")},             //
//             {"Grapes/Michael Fassbender.json", false, JsonKey::ConstructJsonByteArray("Michael Fassbender")},     //
//             {"Grapes/Jane Grey.json", false, JsonKey::ConstructJsonByteArray("Jane Grey")},                       //
//             {"Grapes/Storm.json", false, JsonKey::ConstructJsonByteArray("Storm")},                               //
//             {"Grapes/Avengers/Iron Man.json", false, JsonKey::ConstructJsonByteArray("Iron Man")},                //
//             {"Grapes/Avengers/Captain America.json", false, JsonKey::ConstructJsonByteArray("Captain America")},  //
//             {"Grapes/Avengers/Thor.json", false, JsonKey::ConstructJsonByteArray("Thor")},                        //
//             {"Grapes/Avengers/Loki.json", false, JsonKey::ConstructJsonByteArray("Loki")},                        //
//         };
//     QCOMPARE(mTDir.createEntries(nodeGrapes), nodeGrapes.size());  // 8json. no ImgName, No VidName
//     {
//       ScnMgr scnMgr2;
//       QCOMPARE(scnMgr2(grapePath), Counter(0, 8, 0, 0));  // 3updated, 3used,3VidUpdated,2ImgNameUpdated
//       QCOMPARE(ScnMgr::UpdateScnFiles(grapePath), 2);     // 2 scn file writed ok
//     }
//   }

//   void cleanupTestCase() {  //
//   }

//   void init() { GlobalMockObject::reset(); }

//   void cleanup() { GlobalMockObject::verify(); }

//   void initalized_ok() {
//     ScenesListModel defModel{"ScenesListView"};
//     QVERIFY(defModel.mPerPageEleCnt >= 10);
//     QCOMPARE(defModel.rootPath(), "");
//     QVERIFY(defModel.data({}, Qt::DisplayRole).isNull());
//     QCOMPARE(defModel.setData({}, 999, Qt::EditRole), false);

//     QCOMPARE(defModel.GetPageIndex(), 0);
//     QCOMPARE(defModel.GetPageCnt(), 0);

//     // will not crash down
//     QCOMPARE(defModel.rowCount(), 0);
//     QCOMPARE(defModel.mCurPageStart, 0);
//     QCOMPARE(defModel.mCurPageEnd, 0);

//     QModelIndex invalidIndex;
//     int linearInd = -1;
//     QVERIFY(!defModel.isLocalIndexValid(invalidIndex, linearInd));

//     // 测试各种成员函数在空模型下的行为
//     QVERIFY(defModel.fileInfo(invalidIndex).fileName().isEmpty());
//     QCOMPARE(defModel.filePath(invalidIndex), "");
//     QCOMPARE(defModel.fileName(invalidIndex), "");
//     QCOMPARE(defModel.GetRate(invalidIndex), 0);
//     QCOMPARE(defModel.baseName(invalidIndex), "");
//     QCOMPARE(defModel.absolutePath(invalidIndex), "");
//     QCOMPARE(defModel.GetImgs(invalidIndex), (QStringList{}));
//     QCOMPARE(defModel.GetVids(invalidIndex), (QStringList{}));
//     QCOMPARE(defModel.GetJson(invalidIndex), "");
//     QCOMPARE(defModel.GetScn(invalidIndex), "");

//     // 测试分页功能
//     QVERIFY(defModel.onScenesCountsPerPageChanged(10));
//     QCOMPARE(defModel.mPerPageEleCnt, 10);
//     QCOMPARE(defModel.rowCount(), 0);
//     QCOMPARE(defModel.mCurPageStart, 0);
//     QCOMPARE(defModel.mCurPageEnd, 0);

//     QVERIFY(defModel.onPageIndexChanged(1));
//     QCOMPARE(defModel.mCurPageIndex, 1);
//     QCOMPARE(defModel.GetGlbEleCnt(), 0);

//     QCOMPARE(defModel.GetPageCnt(), 0);
//     QCOMPARE(defModel.GetGlbEleCnt(), 0);

//     QVERIFY(defModel.rel2fileNames({}).isEmpty());
//   }

//   void setRootPath_ok() {
//     SceneInfo si0{"/", "Kaka", {}, {"Kaka.mp4"}, 0, 10, "2000-06-16 00:00:60"};
//     SceneInfo si1{"/", "Cristiano Ronaldo", {}, {"Cristiano Ronaldo.mp4"}, 0, 10, "2000-06-16 00:00:60"};
//     const SceneInfoList scenesLstInPath0{si0};
//     const SceneInfoList scenesLstInPath1{si0, si1};

//     MOCKER(SceneHelper::GetScnsLstFromPath)   //
//         .expects(exactly(2))                  //
//         .with(QString{"inexist/path0"})       //
//         .will(returnValue(scenesLstInPath0))  //
//         .id("0");
//     MOCKER(SceneHelper::GetScnsLstFromPath)   //
//         .expects(exactly(1))                  //
//         .with(QString{"inexist/path1"})       //
//         .after("0")                           //
//         .will(returnValue(scenesLstInPath1))  //
//         .id("1");

//     Configuration().setValue(SceneKey::SCENES_COUNT_EACH_PAGE.name, 1000);
//     ScenesListModel slm{"ScenesListModelSetRootPath"};
//     QCOMPARE(slm.rootPath(), "");
//     QCOMPARE(slm.mPerPageEleCnt, 1000);
//     QSignalSpy pgCntSpy{&slm, &ScenesListModel::pagesCountChanged};

//     QCOMPARE(slm.setRootPath("inexist/path0"), true);  // GetScnsLstFromPath: 1, emit pagesCountChanged
//     QCOMPARE(slm.rowCount(), 1);                       // 1 item
//     QCOMPARE(pgCntSpy.count(), 1);
//     QCOMPARE(pgCntSpy.takeLast(), (QVariantList{1}));  // 1 page = floor(1/1000)
//     QCOMPARE(slm.GetPageCnt(), 1);

//     QCOMPARE(slm.setRootPath("inexist/path0"), false);  // again and not force->ignore, no emit pagesCountChanged
//     QCOMPARE(slm.rowCount(), 1);                        // 1 item
//     QCOMPARE(pgCntSpy.count(), 0);
//     slm.onScenesCountsPerPageChanged(1);
//     QCOMPARE(pgCntSpy.count(), 0);  // 1 / 1 = 1

//     QCOMPARE(slm.setRootPath("inexist/path0", true), true);  // GetScnsLstFromPath: 2, no emit pagesCountChanged
//     QCOMPARE(slm.rowCount(), 1);                             // 1 item
//     QCOMPARE(slm.GetPageCnt(), 1);                           // 1 page = floor(1/1)
//     QCOMPARE(pgCntSpy.count(), 0);

//     QCOMPARE(slm.setRootPath("inexist/path1"), true);  // GetScnsLstFromPath: 3, emit pagesCountChanged
//     QCOMPARE(slm.rowCount(), 1);                       // 1 item in page 0
//     QCOMPARE(pgCntSpy.count(), 1);
//     QCOMPARE(pgCntSpy.takeLast(), (QVariantList{2}));  // 2 page = floor(2 / 1)
//     QCOMPARE(slm.GetPageCnt(), 2);

//     slm.onScenesCountsPerPageChanged(2);  // change sceneCount per page, but page cnt unchange, emit pagesCountChanged
//     QCOMPARE(slm.rowCount(), 2);          // 2 item in page 0
//     QCOMPARE(pgCntSpy.count(), 1);
//     QCOMPARE(pgCntSpy.takeLast(), (QVariantList{1}));  // 1 page = floor(2 / 2)
//     QCOMPARE(slm.GetPageCnt(), 1);
//   }

//   void data_retrieve_ok() {
//     SceneInfo si0{"/", "Kaka", {"Kaka.png"}, {}, 0, 9, "2000-06-16 00:00:60"};
//     SceneInfo si1{"/", "Cristiano Ronaldo", {}, {"Cristiano Ronaldo.mp4"}, 0, 10, "2000-06-16 00:00:60"};
//     const SceneInfoList sceneList{si0, si1};

//     MOCKER(SceneHelper::GetScnsLstFromPath)  //
//         .expects(exactly(1))                 //
//         .with(QString{"inexist/path"})       //
//         .will(returnValue(sceneList));       //

//     Configuration().setValue(SceneKey::SCENES_COUNT_EACH_PAGE.name, 1000);
//     ScenesListModel slm{"ScenesListModelData"};
//     QCOMPARE(slm.setRootPath("inexist/path"), true);

//     QCOMPARE(slm.rowCount(), 2);
//     QCOMPARE(slm.GetPageCnt(), 1);
//     QCOMPARE(slm.GetPageIndex(), 0);

//     QCOMPARE(slm.data(slm.index(0), Qt::DisplayRole).toString(), "Kaka");               // 0 image, 0 video
//     QCOMPARE(slm.data(slm.index(1), Qt::DisplayRole).toString(), "Cristiano Ronaldo");  // 0 image, 0 video

//     QCOMPARE(slm.data(slm.index(0), Qt::DecorationRole).canConvert<QPixmap>(), true);  // 1 image, but not exist, load failed
//     QCOMPARE(slm.data(slm.index(1), Qt::DecorationRole).canConvert<QPixmap>(), true);  // 0 image, use default from res.qrc

//     QCOMPARE(slm.data(slm.index(0), Qt::BackgroundRole).isValid(), true);   // 0 video, specify solid pattern
//     QCOMPARE(slm.data(slm.index(1), Qt::BackgroundRole).isValid(), false);  // 1 video, no pattern

//     QCOMPARE(slm.data(slm.index(0), ScenesListModel::CustomRole::RatingRole).toInt(), 9);
//     QCOMPARE(slm.data(slm.index(1), ScenesListModel::CustomRole::RatingRole).toInt(), 10);

//     QCOMPARE(slm.data(slm.index(0), Qt::ForegroundRole).isValid(), false);
//     QCOMPARE(slm.data(slm.index(1), Qt::ForegroundRole).isValid(), false);

//     QCOMPARE(slm.fileInfo(slm.index(0)).fileName().isEmpty(), true);
//     QCOMPARE(slm.filePath(slm.index(0)), "");
//     QCOMPARE(slm.fileName(slm.index(0)), "");
//     QCOMPARE(slm.GetRate(slm.index(0)), 9);
//     QCOMPARE(slm.baseName(slm.index(0)), "Kaka");
//     QCOMPARE(slm.absolutePath(slm.index(0)), "inexist/path/");
//     QCOMPARE(slm.GetImgs(slm.index(0)), (QStringList{"inexist/path/Kaka.png"}));
//     QCOMPARE(slm.GetVids(slm.index(0)), (QStringList{}));
//     QCOMPARE(slm.GetJson(slm.index(0)), "inexist/path/Kaka.json");
//     QCOMPARE(slm.GetScn(slm.index(0)), "inexist/path/path.scn");

//     QCOMPARE(slm.fileInfo(slm.index(1)), QFileInfo("inexist/path/Cristiano Ronaldo.mp4"));
//     QCOMPARE(slm.filePath(slm.index(1)), "inexist/path/Cristiano Ronaldo.mp4");
//     QCOMPARE(slm.fileName(slm.index(1)), "Cristiano Ronaldo.mp4");
//     QCOMPARE(slm.GetRate(slm.index(1)), 10);
//     QCOMPARE(slm.baseName(slm.index(1)), "Cristiano Ronaldo");
//     QCOMPARE(slm.absolutePath(slm.index(1)), "inexist/path/");
//     QCOMPARE(slm.GetImgs(slm.index(1)), (QStringList{}));
//     QCOMPARE(slm.GetVids(slm.index(1)), (QStringList{"inexist/path/Cristiano Ronaldo.mp4"}));
//     QCOMPARE(slm.GetJson(slm.index(1)), "inexist/path/Cristiano Ronaldo.json");
//     QCOMPARE(slm.GetScn(slm.index(1)), "inexist/path/path.scn");
//   }

//   void setData_ok() {
//     MOCKER(SceneHelper::UpdateNameWithNewRate)  //
//         .expects(exactly(2))                    //
//         .will(returnValue(true));
//     MOCKER(RateHelper::RateMovie)  //
//         .expects(exactly(2))       //
//         .will(returnValue(true));
//     // const bool bScnUpdatedOk = SceneHelper::UpdateNameWithNewRate(scnAbsFilePath, eleBaseName, newRate);
//     // const QString jsonAbsFilePath = GetJson(index);
//     // const bool bJsonUpdatedOk = RateHelper::RateMovie(jsonAbsFilePath, newRate);

//     SceneInfo si0{"/", "Kaka", {"Kaka.png"}, {}, 0, 9, "2000-06-16 00:00:60"};
//     const SceneInfoList sceneList{si0};

//     MOCKER(SceneHelper::GetScnsLstFromPath)  //
//         .expects(exactly(1))                 //
//         .with(QString{"inexist/path"})       //
//         .will(returnValue(sceneList));       //

//     Configuration().setValue(SceneKey::SCENES_COUNT_EACH_PAGE.name, 1000);
//     ScenesListModel slm{"ScenesListModelData"};
//     QCOMPARE(slm.setRootPath("inexist/path"), true);

//     QCOMPARE(slm.rowCount(), 1);
//     QModelIndex kakaInd = slm.index(0);
//     QCOMPARE(slm.GetRate(kakaInd), 9);

//     QCOMPARE(slm.setData(kakaInd, 6, Qt::EditRole), false);                            // role unmatch
//     QCOMPARE(slm.setData({}, 6, Qt::EditRole), false);                                 // index invalid unmatch
//     QCOMPARE(slm.setData(kakaInd, 6, ScenesListModel::CustomRole::RatingRole), true);  // 1st
//     QCOMPARE(slm.setData(kakaInd, 8, ScenesListModel::CustomRole::RatingRole), true);  // 1st

//     QCOMPARE(slm.ModifySceneInfoRateValue({}, 8), false);
//     QCOMPARE(slm.ModifySceneInfoRateValue(kakaInd, 8), true);   // 2nd
//     QCOMPARE(slm.ModifySceneInfoRateValue(kakaInd, 8), true);  // rate unchange
//   }

//   void sort_given_only_one_dimension_sorted_data_ok() {
//     SceneInfoList mockMoviePathAscendingList;  // asc
//     QVERIFY(GetSceneInfoList(10, SceneSortOrderHelper::SortDimE::MOVIE_PATH, false, mockMoviePathAscendingList));
//     SceneInfoList mockMovieSizeDescendingList;  // desc
//     QVERIFY(GetSceneInfoList(10, SceneSortOrderHelper::SortDimE::MOVIE_SIZE, true, mockMovieSizeDescendingList));
//     SceneInfoList mockRateAscendingList;  // asc
//     QVERIFY(GetSceneInfoList(10, SceneSortOrderHelper::SortDimE::RATE, false, mockRateAscendingList));
//     SceneInfoList mockUploadedDescendingList;  // desc
//     QVERIFY(GetSceneInfoList(11, SceneSortOrderHelper::SortDimE::UPLOADED_TIME, true, mockUploadedDescendingList));

//     MOCKER(SceneHelper::GetScnsLstFromPath)              //
//         .expects(exactly(4))                             //
//         .will(returnValue(mockMoviePathAscendingList))   //
//         .then(returnValue(mockMovieSizeDescendingList))  //
//         .then(returnValue(mockRateAscendingList))        //
//         .then(returnValue(mockUploadedDescendingList))   //
//         ;
//     ScenesListModel slm{"ScenesListView"};
//     SceneSortProxyModel sspm;
//     sspm.setSourceModel(&slm);
//     {  // MOVIE_PATH
//       QCOMPARE(slm.setRootPath("any/inexists/path1"), true);
//       QCOMPARE(slm.rowCount(), 10);
//       // 升
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::AscendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 8, sspm, 8));
//       QVERIFY(checkIndexMatch(slm, 9, sspm, 9));
//       // 降
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::DescendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 9));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 8));
//       QVERIFY(checkIndexMatch(slm, 8, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 9, sspm, 0));

//       // 强化用例: 只更改维度, 不改变降序(aka 在Rate+降序情况下), 预期按照Rate降序
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::RATE, Qt::DescendingOrder);
//       QVERIFY(slm.GetRate(sspm.mapToSource(sspm.index(0, 0))) >= slm.GetRate(sspm.mapToSource(sspm.index(1, 0))));
//       QVERIFY(slm.GetRate(sspm.mapToSource(sspm.index(1, 0))) >= slm.GetRate(sspm.mapToSource(sspm.index(2, 0))));
//       QVERIFY(slm.GetRate(sspm.mapToSource(sspm.index(2, 0))) >= slm.GetRate(sspm.mapToSource(sspm.index(3, 0))));
//       QVERIFY(slm.GetRate(sspm.mapToSource(sspm.index(8, 0))) >= slm.GetRate(sspm.mapToSource(sspm.index(9, 0))));
//     }

//     // 测试MOVIE_SIZE排序
//     {
//       QCOMPARE(slm.setRootPath("any/inexists/path2"), true);
//       QCOMPARE(slm.rowCount(), 10);

//       // 降序排序
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::DescendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 8, sspm, 8));
//       QVERIFY(checkIndexMatch(slm, 9, sspm, 9));

//       // 升序排序
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::AscendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 9));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 8));
//       QVERIFY(checkIndexMatch(slm, 8, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 9, sspm, 0));
//     }

//     // 测试RATE排序
//     {
//       QCOMPARE(slm.setRootPath("any/inexists/path3"), true);
//       // 升序排序
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::RATE, Qt::AscendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 8, sspm, 8));
//       QVERIFY(checkIndexMatch(slm, 9, sspm, 9));

//       // 降序排序
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::RATE, Qt::DescendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 9));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 8));
//       QVERIFY(checkIndexMatch(slm, 8, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 9, sspm, 0));
//     }

//     // 测试UPLOADED_TIME排序
//     {
//       QCOMPARE(slm.setRootPath("any/inexists/path4"), true);
//       QCOMPARE(slm.rowCount(), 11);
//       // 降序排序
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::UPLOADED_TIME, Qt::DescendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 8, sspm, 8));
//       QVERIFY(checkIndexMatch(slm, 9, sspm, 9));

//       // 升序排序
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::UPLOADED_TIME, Qt::AscendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 10));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 9));
//       QVERIFY(checkIndexMatch(slm, 8, sspm, 2));
//       QVERIFY(checkIndexMatch(slm, 9, sspm, 1));
//     }
//   }

//   void sort_ok() {
//     // precondition
//     SceneInfo si0{"/", "Ben Affleck", {}, {"Ben Affleck.mp4"}, 100 * 1024 * 1024, 10, "2000-06-16 00:00:60"};
//     SceneInfo si1{"/", "Brad Pitt", {}, {"Brad Pitt.mp4"}, 99 * 1024 * 1024, 11, "2000-06-16 00:00:59"};
//     SceneInfo si2{"/", "Chris Evans", {}, {"Chris Evans.mp4"}, 98 * 1024 * 1024, 12, "2000-06-16 00:00:58"};
//     SceneInfo si3{"/", "Chris Hemsworth", {}, {"Chris Hemsworth.mp4"}, 97 * 1024 * 1024, 13, "2000-06-16 00:00:57"};
//     SceneInfo si4{"/", "Chris Pine", {}, {"Chris Pine.mp4"}, 96 * 1024 * 1024, 14, "2000-06-16 00:00:56"};
//     SceneInfo si5{"/", "Chris Pratt", {}, {"Chris Pratt.mp4"}, 95 * 1024 * 1024, 15, "2000-06-16 00:00:55"};
//     SceneInfo si6{"/", "Henry Cavill", {}, {"Henry Cavill.mp4"}, 94 * 1024 * 1024, 16, "2000-06-16 00:00:54"};
//     SceneInfo si7{"/", "Keanu Reeves", {}, {"Keanu Reeves.mp4"}, 93 * 1024 * 1024, 17, "2000-06-16 00:00:53"};
//     SceneInfo si8{"/", "Michael Fassbender", {}, {"Michael Fassbender.mp4"}, 92 * 1024 * 1024, 18, "2000-06-16 00:00:52"};
//     SceneInfo si9{"/", "Tom Cruise", {}, {"Tom Cruise.mp4"}, 91 * 1024 * 1024, 19, "2000-06-16 00:00:51"};
//     SceneInfo si10{"/Brazil/", "Kaka", {}, {"Kaka.mp4"}, 90 * 1024 * 1024, 20, "2000-06-16 00:00:50"};
//     SceneInfo si11{"/French/", "Raphael Varane", {}, {"Raphael Varane.mp4"}, 89 * 1024 * 1024, 21, "2000-06-16 00:00:49"};
//     SceneInfo si12{"/Polish/", "Robert Lewandowski", {}, {"Robert Lewandowski.mp4"}, 88 * 1024 * 1024, 22, "2000-06-16 00:00:48"};
//     SceneInfo si13{"/Portuguese/", "Cristiano Ronaldo", {}, {"Cristiano Ronaldo.mp4"}, 87 * 1024 * 1024, 23, "2000-06-16 00:00:47"};
//     SceneInfo si14{"/Spain/", "Alvaro Morata", {}, {"Alvaro Morata.mp4"}, 86 * 1024 * 1024, 24, "2000-06-16 00:00:46"};

//     SceneInfoList scenesLst{
//         si0, si1, si2, si3, si4, si5, si6, si7, si8, si9, si10, si11, si12, si13, si14  //
//     };
//     QCOMPARE(scenesLst.size(), 15);
//     MOCKER(SceneHelper::GetScnsLstFromPath).expects(exactly(1)).will(returnValue(scenesLst));

//     ScenesListModel slm{"ScenesListView"};
//     SceneSortProxyModel sspm;
//     {
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::AscendingOrder);  // nothing happend
//       sspm.setSourceModel(nullptr);                                                               // ignored
//       sspm.setSourceModel(&slm);
//       sspm.setSourceModel(&slm);  // rebind, ignored
//     }

//     {
//       QCOMPARE(sspm.sortOrder(), Qt::SortOrder::AscendingOrder);
//       QCOMPARE(sspm.m_sortDimension, SceneSortOrderHelper::DEFAULT_SCENE_SORT_ORDER);
//       QVERIFY(sspm.mComparator != nullptr);
//       QVERIFY(sspm.m_sourceModel != nullptr);
//     }

//     {  // signal pagesCountChanged ok
//       QCOMPARE(slm.mPerPageEleCnt, SceneKey::SCENES_COUNT_EACH_PAGE.v);
//       constexpr int EXPECT_PAGES_COUNT = 1;  // ceil(15 / 1024)
//       QSignalSpy pagesCntChanged(&slm, &ScenesListModel::pagesCountChanged);
//       QCOMPARE(slm.setRootPath("any/inexists/path"), true);
//       QCOMPARE(pagesCntChanged.count(), 1);

//       QVariantList countChangedToParams = pagesCntChanged.back();
//       QCOMPARE(countChangedToParams.size(), 1);
//       QCOMPARE(countChangedToParams[0].toInt(), EXPECT_PAGES_COUNT);
//       QCOMPARE(slm.GetPageCnt(), EXPECT_PAGES_COUNT);
//       QCOMPARE(slm.rowCount(), 15);
//     }

//     // Precondition: there are 15 items in source model, and MoviePath(ascending), MovieSize(Desc), Rate(Asce), Uploaded(Desc)
//     QCOMPARE(slm.rowCount(), 15);
//     QCOMPARE(sspm.sortOrder(), Qt::SortOrder::AscendingOrder);  // by default ascending in MoviePath
//     {
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - 1));

//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::DescendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 15 - 0 - 1));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 15 - 1 - 1));
//       QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - (15 - 1) - 1));
//     }

//     {  //
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::AscendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 15 - 0 - 1));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 15 - 1 - 1));
//       QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - (15 - 1) - 1));

//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::DescendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - 1));
//     }

//     {
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::RATE, Qt::AscendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - 1));

//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::RATE, Qt::DescendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 15 - 0 - 1));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 15 - 1 - 1));
//       QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - (15 - 1) - 1));
//     }

//     {  //
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::UPLOADED_TIME, Qt::AscendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 15 - 0 - 1));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 15 - 1 - 1));
//       QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - (15 - 1) - 1));

//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::UPLOADED_TIME, Qt::DescendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - 1));
//     }

//     // 2.0 设置10 cnt/page, 第1页有10个, 第2页有5个
//     QVERIFY(slm.mPerPageEleCnt > 15);
//     QCOMPARE(slm.rowCount(), 15);

//     slm.onScenesCountsPerPageChanged(10);
//     slm.onPageIndexChanged(0);
//     QCOMPARE(slm.rowCount(), 10);

//     slm.onPageIndexChanged(1);
//     QCOMPARE(slm.rowCount(), 5);

//     {
//       // 本就是MOVIE_PATH 升序
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::AscendingOrder);  // ascending
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 4, sspm, 4));

//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::DescendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 4));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 3));
//       QVERIFY(checkIndexMatch(slm, 4, sspm, 0));
//     }
//     {
//       // 本就是MOVIE_SIZE 降序
//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::AscendingOrder);  // ascending
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 4));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 3));
//       QVERIFY(checkIndexMatch(slm, 4, sspm, 0));

//       sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::DescendingOrder);
//       QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
//       QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
//       QVERIFY(checkIndexMatch(slm, 4, sspm, 4));
//     }
//   }

//   // void AfterJsonFilesNameRenamed_ok() {
//   //   ScenesListModel slm{"ScenesListView"};
//   //   // 1 预期有3行, 因为有3个json读取到了
//   //   QVERIFY(slm.setRootPath(bambooPath, false));
//   //   QCOMPARE(slm.rootPath(), mTDir.itemPath("Bamboo"));
//   //   QCOMPARE(slm.rowCount(), 3);
//   //   QCOMPARE(slm.GetPageCnt(), 1);

//   //   // data retrieve ok
//   //   // rel2Scn of the first 3 row is "/", "/Avengers/", "/Avengers/" respectively.
//   //   // we sort scenes by rel2Scn acii ascending. if rel2Scn equal, by names ascending.
//   //   QCOMPARE(slm.data(slm.index(0), Qt::DisplayRole).toString(), "The Last Fight");   // 2 images, 1 video
//   //   QCOMPARE(slm.data(slm.index(1), Qt::DisplayRole).toString(), "Captain America");  // 0 image, 1 video
//   //   QCOMPARE(slm.data(slm.index(2), Qt::DisplayRole).toString(), "Iron Man");         // 1 image, 1 video

//   //   // only 1 page, delete no row
//   //   {
//   //     auto beforeCurBegin = slm.mCurBegin;
//   //     auto beforeCurEnd = slm.mCurEnd;
//   //     auto beforeEntryList = slm.GetEntryList();
//   //     QCOMPARE(slm.AfterJsonFilesNameRenamed({}), 0);
//   //     QCOMPARE(slm.mCurBegin, beforeCurBegin);
//   //     QCOMPARE(slm.mCurEnd, beforeCurEnd);
//   //     QCOMPARE(slm.GetEntryList(), beforeEntryList);
//   //   }

//   //   // only 1 page, delete medium row
//   //   {
//   //     auto secondRowIndex = slm.index(1);
//   //     QCOMPARE(slm.AfterJsonFilesNameRenamed({secondRowIndex}), 1);
//   //     QCOMPARE(slm.mCurBegin, slm.GetEntryList().cbegin());
//   //     QCOMPARE(slm.mCurEnd, slm.mCurBegin + 2);
//   //     QCOMPARE(slm.GetEntryList().size(), 2);
//   //   }

//   //   QCOMPARE(slm.rowCount(), 2);
//   //   slm.onScenesCountsPerPageChanged(1);
//   //   QCOMPARE(slm.GetPageCnt(), 2);
//   //   QCOMPARE(slm.mCurPageIndex, 0);
//   //   QCOMPARE(slm.rowCount(), 1);

//   //   // 2 page, each page 1 element, delete element in 0th page
//   //   {
//   //     QCOMPARE(slm.AfterJsonFilesNameRenamed({slm.index(0)}), 1);
//   //     QCOMPARE(slm.GetPageCnt(), 1);
//   //     QCOMPARE(slm.mCurPageIndex, 0);
//   //     QCOMPARE(slm.rowCount(), 1);
//   //     QCOMPARE(slm.mCurBegin, slm.GetEntryList().cbegin());
//   //     QCOMPARE(slm.mCurEnd, slm.mCurBegin + 1);
//   //   }
//   // }
// };

// #include "ScenesListModelTest.moc"
// REGISTER_TEST(ScenesListModelTest, true)
