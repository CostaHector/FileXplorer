#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "ScenesListModel.h"
#include "SceneSortProxyModel.h"
#include "EndToExposePrivateMember.h"

#include "SceneInfoManager.h"
#include "ImageTestPrecoditionTools.h"
#include "StringTool.h"
#include <QDir>
#include <QDirIterator>
#include "TDir.h"

#include "JsonTestPrecoditionTools.h"
#include "JsonKey.h"
#include "RateHelper.h"
#include <random>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

using namespace ImageTestPrecoditionTools;
using namespace SceneInfoManager;

bool checkIndexMatch(const QAbstractListModel& srcModel, int srcR, const QSortFilterProxyModel& proModel, int proR) {
  QVariant src = srcModel.data(srcModel.index(srcR), Qt::DisplayRole);
  QVariant dst = proModel.data(proModel.index(proR, 0), Qt::DisplayRole);
  if (src != dst) {
    LOG_D("src:[%s], pro:[%s]", qPrintable(src.toString()), qPrintable(dst.toString()));
    return false;
  }
  return true;
}

bool GetSceneInfoList(int count, SceneSortOrderHelper::SortDimE sortDim, bool isReverse, SceneInfoList& ansList) {
  ansList.clear();
  ansList.reserve(count);

  std::uniform_int_distribution<int> uniformIntDist(0, count + 1);
  std::default_random_engine e;
  SceneInfo siBasic{"/", "Ben Affleck", {}, {"Ben Affleck.mp4"}, 100 * 1024 * 1024, 10, "2000-06-16 00:00:60"};
  for (int i = 0; i < count; ++i) {
    // 所有dimension都是统一的无序
    int rndInt = uniformIntDist(e);
    QString rnd = QString::number(rndInt);
    siBasic.name = "name" + rnd;
    siBasic.vidSize = rndInt * 1000;
    siBasic.rate = rndInt * 100;
    siBasic.uploaded = "uploaded" + rnd;
    // 只有入参指定一个dimension有序
    const int valueForDistinguish = isReverse ? count - i - 1 : i;
    switch (sortDim) {
      case SceneSortOrderHelper::SortDimE::MOVIE_PATH: {
        siBasic.name = QString::asprintf("Name %03d", valueForDistinguish);
        break;
      }
      case SceneSortOrderHelper::SortDimE::MOVIE_SIZE: {
        siBasic.vidSize = valueForDistinguish;
        break;
      }
      case SceneSortOrderHelper::SortDimE::RATE: {
        siBasic.rate = valueForDistinguish;
        break;
      }
      case SceneSortOrderHelper::SortDimE::UPLOADED_TIME: {
        siBasic.uploaded = QString::asprintf("2010-06-16 00:00:00.%03d", valueForDistinguish);
        break;
      }
      default:
        LOG_E("unsupport: %s", SceneSortOrderHelper::c_str(sortDim));
        return false;
    }
    ansList.push_back(siBasic);
  }
  return true;
}

bool isValidPixmap(const QVariant& pixmapVar) {
  return !pixmapVar.isNull() && pixmapVar.canConvert<QPixmap>() && !pixmapVar.value<QPixmap>().isNull();
}

class ScenesListModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
  QString lastFight1SvgContents = GetSvgContentTemplate().arg(SVG_FILL_COLORS[0]).arg("The Last Fight 1");
  QString ironMenSvgContents = GetSvgContentTemplate().arg(SVG_FILL_COLORS[2]).arg("Iron Man");

  QList<FsNodeEntry> nodeBamboo;
  QList<FsNodeEntry> nodeGrapes;

  const QString bambooPath = mTDir.itemPath("Bamboo");
  const QString grapePath = mTDir.itemPath("Grapes");

 private slots:
  void initTestCase() {
    Configuration().clear();

    QVERIFY(mTDir.IsValid());
    QVERIFY(SVG_FILL_COLORS_COUNT > 3);

    nodeBamboo =  //
        {
            {"Bamboo/The Last Fight.json", false, JsonKey::ConstructJsonByteArray("The Last Fight")},             //
            {"Bamboo/The Last Fight 1.jpg", false, lastFight1SvgContents.toUtf8()},                               //
            {"Bamboo/The Last Fight 2.jpg", false, ""},                                                           //
            {"Bamboo/The Last Fight.mp4", false, ""},                                                             //
            {"Bamboo/Avengers/Iron Man.json", false, JsonKey::ConstructJsonByteArray("Iron Man")},                //
            {"Bamboo/Avengers/Iron Man.jpg", false, ironMenSvgContents.toUtf8()},                                 //
            {"Bamboo/Avengers/Iron Man.mp4", false, ""},                                                          //
            {"Bamboo/Avengers/Captain America.json", false, JsonKey::ConstructJsonByteArray("Captain America")},  //
            {"Bamboo/Avengers/Captain America.mp4", false, ""},                                                   //
        };
    QCOMPARE(mTDir.createEntries(nodeBamboo), nodeBamboo.size());  // 3json
    {
      ScnMgr scnMgr1;
      QCOMPARE(scnMgr1(bambooPath), Counter(3, 3, 3, 2));  // 3updated, 3used,3VidUpdated,2ImgNameUpdated
      QCOMPARE(ScnMgr::UpdateScnFiles(bambooPath), 2);     // 2 scn file writed ok
    }

    nodeGrapes =  //                                                                                       //
        {
            {"Grapes/The Last Fight.json", false, JsonKey::ConstructJsonByteArray("The Last Fight")},             //
            {"Grapes/Michael Fassbender.json", false, JsonKey::ConstructJsonByteArray("Michael Fassbender")},     //
            {"Grapes/Jane Grey.json", false, JsonKey::ConstructJsonByteArray("Jane Grey")},                       //
            {"Grapes/Storm.json", false, JsonKey::ConstructJsonByteArray("Storm")},                               //
            {"Grapes/Avengers/Iron Man.json", false, JsonKey::ConstructJsonByteArray("Iron Man")},                //
            {"Grapes/Avengers/Captain America.json", false, JsonKey::ConstructJsonByteArray("Captain America")},  //
            {"Grapes/Avengers/Thor.json", false, JsonKey::ConstructJsonByteArray("Thor")},                        //
            {"Grapes/Avengers/Loki.json", false, JsonKey::ConstructJsonByteArray("Loki")},                        //
        };
    QCOMPARE(mTDir.createEntries(nodeGrapes), nodeGrapes.size());  // 8json. no ImgName, No VidName
    {
      ScnMgr scnMgr2;
      QCOMPARE(scnMgr2(grapePath), Counter(0, 8, 0, 0));  // 3updated, 3used,3VidUpdated,2ImgNameUpdated
      QCOMPARE(ScnMgr::UpdateScnFiles(grapePath), 2);     // 2 scn file writed ok
    }
  }

  void init() {
    GlobalMockObject::reset();
    Configuration().remove(SceneKey::SCENES_COUNT_EACH_PAGE.name);
  }

  void cleanup() {
    GlobalMockObject::verify();
    Configuration().remove(SceneKey::SCENES_COUNT_EACH_PAGE.name);
  }

  void initalized_ok() {
    ScenesListModel defModel{"ScenesListView"};
    QVERIFY(defModel.mPagedData.GetPerPageEleCnt() >= 10);
    QCOMPARE(defModel.rootPath(), "");
    QVERIFY(defModel.data({}, Qt::DisplayRole).isNull());
    QCOMPARE(defModel.setData({}, 999, Qt::EditRole), false);

    // will not crash down
    QCOMPARE(defModel.rowCount(), 0);

    QModelIndex invalidIndex;
    int linearInd = -1;
    QVERIFY(!defModel.isLocalIndexValid(invalidIndex, linearInd));

    // 测试各种成员函数在空模型下的行为
    QVERIFY(defModel.fileInfo(invalidIndex).fileName().isEmpty());
    QCOMPARE(defModel.filePath(invalidIndex), "");
    QCOMPARE(defModel.fileName(invalidIndex), "");
    QCOMPARE(defModel.GetRate(invalidIndex), 0);
    QCOMPARE(defModel.baseName(invalidIndex), "");
    QCOMPARE(defModel.absolutePath(invalidIndex), "");
    QCOMPARE(defModel.GetImgs(invalidIndex), (QStringList{}));
    QCOMPARE(defModel.GetVids(invalidIndex), (QStringList{}));
    QCOMPARE(defModel.GetJson(invalidIndex), "");
    QCOMPARE(defModel.GetScn(invalidIndex), "");
    QVERIFY(defModel.rel2fileNames({}).isEmpty());

    // 测试分页功能
    QCOMPARE(defModel.mPagedData.GetPerPageEleCnt(), SceneKey::SCENES_COUNT_EACH_PAGE.v);
    QVERIFY(defModel.onScenesCountsPerPageChanged(10));
    QCOMPARE(defModel.rowCount(), 0);
    QVERIFY(defModel.onPageIndexChanged(1));
    QVERIFY(defModel.onPageIndexChanged(999));

    QCOMPARE(defModel.AfterJsonFilesNameRenamed({}), 0);
  }

  void setRootPath_ok() {
    SceneInfo si0{"/", "Kaka", {}, {"Kaka.mp4"}, 0, 10, "2000-06-16 00:00:60"};
    SceneInfo si1{"/", "Cristiano Ronaldo", {}, {"Cristiano Ronaldo.mp4"}, 0, 10, "2000-06-16 00:00:60"};
    const SceneInfoList scenesLstInPath0{si0};
    const SceneInfoList scenesLstInPath1{si0, si1};

    MOCKER(SceneHelper::GetScnsLstFromPath)   //
        .expects(exactly(2))                  //
        .with(QString{"inexist/path0"})       //
        .will(returnValue(scenesLstInPath0))  //
        .id("0");
    MOCKER(SceneHelper::GetScnsLstFromPath)   //
        .expects(exactly(1))                  //
        .with(QString{"inexist/path1"})       //
        .after("0")                           //
        .will(returnValue(scenesLstInPath1))  //
        .id("1");

    Configuration().setValue(SceneKey::SCENES_COUNT_EACH_PAGE.name, 1000);
    ScenesListModel slm{"ScenesListModelSetRootPath"};
    QCOMPARE(slm.rootPath(), "");
    QCOMPARE(slm.mPagedData.GetPerPageEleCnt(), 1000);
    QSignalSpy pgCntSpy{&slm, &ScenesListModel::pagesCountChanged};

    QCOMPARE(slm.setRootPath("inexist/path0"), true);  // GetScnsLstFromPath: 1, emit pagesCountChanged
    QCOMPARE(slm.rowCount(), 1);                       // 1 item
    QCOMPARE(pgCntSpy.count(), 1);
    QCOMPARE(pgCntSpy.takeLast(), (QVariantList{1}));  // 1 page = floor(1/1000)
    QCOMPARE(slm.mPagedData.GetPageCnt(), 1);

    QCOMPARE(slm.setRootPath("inexist/path0"), false);  // again and not force->ignore, no emit pagesCountChanged
    QCOMPARE(slm.rowCount(), 1);                        // 1 item
    QCOMPARE(pgCntSpy.count(), 0);
    slm.onScenesCountsPerPageChanged(1);
    QCOMPARE(pgCntSpy.count(), 0);  // 1 / 1 = 1

    QCOMPARE(slm.setRootPath("inexist/path0", true), true);  // GetScnsLstFromPath: 2, no emit pagesCountChanged
    QCOMPARE(slm.rowCount(), 1);                             // 1 item
    QCOMPARE(slm.mPagedData.GetPageCnt(), 1);                // 1 page = floor(1/1)
    QCOMPARE(pgCntSpy.count(), 0);

    QCOMPARE(slm.setRootPath("inexist/path1"), true);  // GetScnsLstFromPath: 3, emit pagesCountChanged
    QCOMPARE(slm.rowCount(), 1);                       // 1 item in page 0
    QCOMPARE(pgCntSpy.count(), 1);
    QCOMPARE(pgCntSpy.takeLast(), (QVariantList{2}));  // 2 page = floor(2 / 1)
    QCOMPARE(slm.mPagedData.GetPageCnt(), 2);

    slm.onScenesCountsPerPageChanged(2);  // change sceneCount per page, but page cnt unchange, emit pagesCountChanged
    QCOMPARE(slm.rowCount(), 2);          // 2 item in page 0
    QCOMPARE(pgCntSpy.count(), 1);
    QCOMPARE(pgCntSpy.takeLast(), (QVariantList{1}));  // 1 page = floor(2 / 2)
    QCOMPARE(slm.mPagedData.GetPageCnt(), 1);
  }

  void data_retrieve_ok() {
    SceneInfo si0{"/", "Kaka", {"Kaka.png"}, {}, 0, 9, "2000-06-16 00:00:60"};
    SceneInfo si1{"/", "Cristiano Ronaldo", {}, {"Cristiano Ronaldo.mp4"}, 0, 10, "2000-06-16 00:00:60"};
    const SceneInfoList sceneList{si0, si1};

    MOCKER(SceneHelper::GetScnsLstFromPath)  //
        .expects(exactly(1))                 //
        .with(QString{"inexist/path"})       //
        .will(returnValue(sceneList));       //

    Configuration().setValue(SceneKey::SCENES_COUNT_EACH_PAGE.name, 1000);
    ScenesListModel slm{"ScenesListModelData"};
    QCOMPARE(slm.setRootPath("inexist/path"), true);

    QCOMPARE(slm.rowCount(), 2);
    QCOMPARE(slm.mPagedData.GetPageCnt(), 1);
    QCOMPARE(slm.mPagedData.GetCurPageIndex(), 0);

    QCOMPARE(slm.data(slm.index(0), Qt::DisplayRole).toString(), "Kaka");               // 0 image, 0 video
    QCOMPARE(slm.data(slm.index(1), Qt::DisplayRole).toString(), "Cristiano Ronaldo");  // 0 image, 0 video

    QCOMPARE(slm.data(slm.index(0), Qt::DecorationRole).canConvert<QPixmap>(), true);  // 1 image, but not exist, load failed
    QCOMPARE(slm.data(slm.index(1), Qt::DecorationRole).canConvert<QPixmap>(), true);  // 0 image, use default from res.qrc

    QCOMPARE(slm.data(slm.index(0), Qt::BackgroundRole).isValid(), true);   // 0 video, specify solid pattern
    QCOMPARE(slm.data(slm.index(1), Qt::BackgroundRole).isValid(), false);  // 1 video, no pattern

    QCOMPARE(slm.data(slm.index(0), ScenesListModel::CustomRole::RatingRole).toInt(), 9);
    QCOMPARE(slm.data(slm.index(1), ScenesListModel::CustomRole::RatingRole).toInt(), 10);

    QCOMPARE(slm.data(slm.index(0), Qt::ForegroundRole).isValid(), false);
    QCOMPARE(slm.data(slm.index(1), Qt::ForegroundRole).isValid(), false);

    QCOMPARE(slm.fileInfo(slm.index(0)).fileName().isEmpty(), true);
    QCOMPARE(slm.filePath(slm.index(0)), "");
    QCOMPARE(slm.fileName(slm.index(0)), "");
    QCOMPARE(slm.GetRate(slm.index(0)), 9);
    QCOMPARE(slm.baseName(slm.index(0)), "Kaka");
    QCOMPARE(slm.absolutePath(slm.index(0)), "inexist/path/");
    QCOMPARE(slm.GetImgs(slm.index(0)), (QStringList{"inexist/path/Kaka.png"}));
    QCOMPARE(slm.GetVids(slm.index(0)), (QStringList{}));
    QCOMPARE(slm.GetJson(slm.index(0)), "inexist/path/Kaka.json");
    QCOMPARE(slm.GetScn(slm.index(0)), "inexist/path/path.scn");

    QCOMPARE(slm.fileInfo(slm.index(1)), QFileInfo("inexist/path/Cristiano Ronaldo.mp4"));
    QCOMPARE(slm.filePath(slm.index(1)), "inexist/path/Cristiano Ronaldo.mp4");
    QCOMPARE(slm.fileName(slm.index(1)), "Cristiano Ronaldo.mp4");
    QCOMPARE(slm.GetRate(slm.index(1)), 10);
    QCOMPARE(slm.baseName(slm.index(1)), "Cristiano Ronaldo");
    QCOMPARE(slm.absolutePath(slm.index(1)), "inexist/path/");
    QCOMPARE(slm.GetImgs(slm.index(1)), (QStringList{}));
    QCOMPARE(slm.GetVids(slm.index(1)), (QStringList{"inexist/path/Cristiano Ronaldo.mp4"}));
    QCOMPARE(slm.GetJson(slm.index(1)), "inexist/path/Cristiano Ronaldo.json");
    QCOMPARE(slm.GetScn(slm.index(1)), "inexist/path/path.scn");
  }

  void setData_ok() {
    MOCKER(SceneHelper::UpdateNameWithNewRate)  //
        .expects(exactly(2))                    //
        .will(returnValue(true));
    MOCKER(RateHelper::RateMovie)  //
        .expects(exactly(2))       //
        .will(returnValue(true));
    // const bool bScnUpdatedOk = SceneHelper::UpdateNameWithNewRate(scnAbsFilePath, eleBaseName, newRate);
    // const QString jsonAbsFilePath = GetJson(index);
    // const bool bJsonUpdatedOk = RateHelper::RateMovie(jsonAbsFilePath, newRate);

    SceneInfo si0{"/", "Kaka", {"Kaka.png"}, {}, 0, 9, "2000-06-16 00:00:60"};
    const SceneInfoList sceneList{si0};

    MOCKER(SceneHelper::GetScnsLstFromPath)  //
        .expects(exactly(1))                 //
        .with(QString{"inexist/path"})       //
        .will(returnValue(sceneList));       //

    Configuration().setValue(SceneKey::SCENES_COUNT_EACH_PAGE.name, 1000);
    ScenesListModel slm{"ScenesListModelData"};
    QCOMPARE(slm.setRootPath("inexist/path"), true);

    QCOMPARE(slm.rowCount(), 1);
    QModelIndex kakaInd = slm.index(0);
    QCOMPARE(slm.GetRate(kakaInd), 9);

    QCOMPARE(slm.setData(kakaInd, 6, Qt::EditRole), false);                            // role unmatch
    QCOMPARE(slm.setData({}, 6, Qt::EditRole), false);                                 // index invalid unmatch
    QCOMPARE(slm.setData(kakaInd, 6, ScenesListModel::CustomRole::RatingRole), true);  // 1st
    QCOMPARE(slm.setData(kakaInd, 8, ScenesListModel::CustomRole::RatingRole), true);  // 1st

    QCOMPARE(slm.ModifySceneInfoRateValue({}, 8), false);
    QCOMPARE(slm.ModifySceneInfoRateValue(kakaInd, 8), true);  // 2nd
    QCOMPARE(slm.ModifySceneInfoRateValue(kakaInd, 8), true);  // rate unchange
  }

  void local_sort_by_proxyModel_given_only_one_dimension_sorted_data_ok() {
    SceneInfoList mockMoviePathAscendingList;  // asc
    QVERIFY(GetSceneInfoList(10, SceneSortOrderHelper::SortDimE::MOVIE_PATH, false, mockMoviePathAscendingList));
    SceneInfoList mockMovieSizeDescendingList;  // desc
    QVERIFY(GetSceneInfoList(10, SceneSortOrderHelper::SortDimE::MOVIE_SIZE, true, mockMovieSizeDescendingList));
    SceneInfoList mockRateAscendingList;  // asc
    QVERIFY(GetSceneInfoList(10, SceneSortOrderHelper::SortDimE::RATE, false, mockRateAscendingList));
    SceneInfoList mockUploadedDescendingList;  // desc
    QVERIFY(GetSceneInfoList(11, SceneSortOrderHelper::SortDimE::UPLOADED_TIME, true, mockUploadedDescendingList));

    MOCKER(SceneHelper::GetScnsLstFromPath)              //
        .expects(exactly(4))                             //
        .will(returnValue(mockMoviePathAscendingList))   //
        .then(returnValue(mockMovieSizeDescendingList))  //
        .then(returnValue(mockRateAscendingList))        //
        .then(returnValue(mockUploadedDescendingList))   //
        ;
    ScenesListModel slm{"ScenesListView"};
    SceneSortProxyModel sspm;
    sspm.setSourceModel(&slm);
    {  // MOVIE_PATH
      QCOMPARE(slm.setRootPath("any/inexists/path1"), true);
      QCOMPARE(slm.rowCount(), 10);
      // 升
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::AscendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 8, sspm, 8));
      QVERIFY(checkIndexMatch(slm, 9, sspm, 9));
      // 降
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 9));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 8));
      QVERIFY(checkIndexMatch(slm, 8, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 9, sspm, 0));

      // 强化用例: 只更改维度, 不改变降序(aka 在Rate+降序情况下), 预期按照Rate降序
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::RATE, Qt::DescendingOrder);
      QVERIFY(slm.GetRate(sspm.mapToSource(sspm.index(0, 0))) >= slm.GetRate(sspm.mapToSource(sspm.index(1, 0))));
      QVERIFY(slm.GetRate(sspm.mapToSource(sspm.index(1, 0))) >= slm.GetRate(sspm.mapToSource(sspm.index(2, 0))));
      QVERIFY(slm.GetRate(sspm.mapToSource(sspm.index(2, 0))) >= slm.GetRate(sspm.mapToSource(sspm.index(3, 0))));
      QVERIFY(slm.GetRate(sspm.mapToSource(sspm.index(8, 0))) >= slm.GetRate(sspm.mapToSource(sspm.index(9, 0))));
    }

    // 测试MOVIE_SIZE排序
    {
      QCOMPARE(slm.setRootPath("any/inexists/path2"), true);
      QCOMPARE(slm.rowCount(), 10);

      // 降序排序
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 8, sspm, 8));
      QVERIFY(checkIndexMatch(slm, 9, sspm, 9));

      // 升序排序
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::AscendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 9));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 8));
      QVERIFY(checkIndexMatch(slm, 8, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 9, sspm, 0));
    }

    // 测试RATE排序
    {
      QCOMPARE(slm.setRootPath("any/inexists/path3"), true);
      // 升序排序
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::RATE, Qt::AscendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 8, sspm, 8));
      QVERIFY(checkIndexMatch(slm, 9, sspm, 9));

      // 降序排序
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::RATE, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 9));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 8));
      QVERIFY(checkIndexMatch(slm, 8, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 9, sspm, 0));
    }

    // 测试UPLOADED_TIME排序
    {
      QCOMPARE(slm.setRootPath("any/inexists/path4"), true);
      QCOMPARE(slm.rowCount(), 11);
      // 降序排序
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::UPLOADED_TIME, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 8, sspm, 8));
      QVERIFY(checkIndexMatch(slm, 9, sspm, 9));

      // 升序排序
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::UPLOADED_TIME, Qt::AscendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 10));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 9));
      QVERIFY(checkIndexMatch(slm, 8, sspm, 2));
      QVERIFY(checkIndexMatch(slm, 9, sspm, 1));
    }
  }

  void local_sort_by_proxyModel_ok() {
    // precondition si 已经按照MoviePaths升序
    SceneInfo si0{"/", "Ben Affleck", {}, {"Ben Affleck.mp4"}, 100 * 1024 * 1024, 10, "2000-06-16 00:00:60"};
    SceneInfo si1{"/", "Brad Pitt", {}, {"Brad Pitt.mp4"}, 99 * 1024 * 1024, 11, "2000-06-16 00:00:59"};
    SceneInfo si2{"/", "Chris Evans", {}, {"Chris Evans.mp4"}, 98 * 1024 * 1024, 12, "2000-06-16 00:00:58"};
    SceneInfo si3{"/Brazil/", "Kaka", {}, {"Kaka.mp4"}, 90 * 1024 * 1024, 20, "2000-06-16 00:00:50"};
    SceneInfo si4{"/French/", "Raphael Varane", {}, {"Raphael Varane.mp4"}, 89 * 1024 * 1024, 21, "2000-06-16 00:00:49"};
    SceneInfo si5{"/Spain/", "Alvaro Morata", {}, {"Alvaro Morata.mp4"}, 86 * 1024 * 1024, 24, "2000-06-16 00:00:46"};

    SceneInfoList scenesLst{si0, si1, si2, si3, si4, si5};
    QCOMPARE(scenesLst.size(), 6);
    MOCKER(SceneHelper::GetScnsLstFromPath).expects(exactly(1)).will(returnValue(scenesLst));

    ScenesListModel slm{"ScenesListView"};
    SceneSortProxyModel sspm;
    {
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::AscendingOrder);  // nothing happend
      sspm.setSourceModel(nullptr);                                                               // ignored
      sspm.setSourceModel(&slm);
      sspm.setSourceModel(&slm);  // rebind, ignored

      QCOMPARE(sspm.sortOrder(), Qt::SortOrder::AscendingOrder);
      QCOMPARE(sspm.m_sortDimension, SceneSortOrderHelper::DEFAULT_SCENE_SORT_ORDER);
      QVERIFY(sspm.mComparator != nullptr);
      QVERIFY(sspm.m_sourceModel != nullptr);
    }

    // Precondition: there are 6 items in source model, and MoviePath(ascending), MovieSize(Desc), Rate(Asce), Uploaded(Desc)
    // signal pagesCountChanged emit when call setRootPath ok
    {
      QSignalSpy pagesCntChanged(&slm, &ScenesListModel::pagesCountChanged);
      QCOMPARE(slm.setRootPath("any/inexists/path"), true);
      QCOMPARE(pagesCntChanged.count(), 1);
      QCOMPARE(pagesCntChanged.takeLast(), (QVariantList{1}));  // floor(6 / 1024)
    }
    // only one page sort(MOVIE_PATH)
    QCOMPARE(slm.mPagedData.GetPageCnt(), 1);
    QCOMPARE(slm.rowCount(), 6);

    {
      QCOMPARE(sspm.sortOrder(), Qt::SortOrder::AscendingOrder);  // by default ascending in MoviePath
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));                  // 0<->0
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));                  // 1<->1
      QVERIFY(checkIndexMatch(slm, 6 - 1, sspm, 6 - 1));          // 5<->5

      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 6 - 0 - 1));            // 0<->5
      QVERIFY(checkIndexMatch(slm, 1, sspm, 6 - 1 - 1));            // 1<->4
      QVERIFY(checkIndexMatch(slm, 6 - 1, sspm, 6 - (6 - 1) - 1));  // 5<->0
    }
    // only one page sort(MOVIE_SIZE)
    {
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::AscendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 6 - 0 - 1));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 6 - 1 - 1));
      QVERIFY(checkIndexMatch(slm, 6 - 1, sspm, 6 - (6 - 1) - 1));

      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 6 - 1, sspm, 6 - 1));
    }
    // only one page sort(RATE)
    {
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::RATE, Qt::AscendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 6 - 1, sspm, 6 - 1));

      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::RATE, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 6 - 0 - 1));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 6 - 1 - 1));
      QVERIFY(checkIndexMatch(slm, 6 - 1, sspm, 6 - (6 - 1) - 1));
    }

    // only one page sort(UPLOADED_TIME)
    {
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::UPLOADED_TIME, Qt::AscendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 6 - 0 - 1));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 6 - 1 - 1));
      QVERIFY(checkIndexMatch(slm, 6 - 1, sspm, 6 - (6 - 1) - 1));

      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::UPLOADED_TIME, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 6 - 1, sspm, 6 - 1));
    }

    // 2.0 设置10 cnt/page, 第1页有10个, 第2页有5个
    // 2.0 设置4 cnt/page, 第1页有4个, 第2页有2个
    slm.onScenesCountsPerPageChanged(4);
    QCOMPARE(slm.mPagedData.GetPageCnt(), 2);

    slm.onPageIndexChanged(0);
    QCOMPARE(slm.rowCount(), 4);

    slm.onPageIndexChanged(1);
    QCOMPARE(slm.rowCount(), 2);

    // two pages, sort on second page
    {
      // 本就是MOVIE_PATH 升序
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::AscendingOrder);  // ascending
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));

      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 0));
    }
    {
      // 本就是MOVIE_SIZE 降序
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::AscendingOrder);  // ascending
      QVERIFY(checkIndexMatch(slm, 0, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 0));

      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
    }
  }

  void global_sort_ok() {
    // precondition si 已经按照MoviePaths升序
    SceneInfo si0{"/", "Ben Affleck", {}, {"Ben Affleck.mp4"}, 100 * 1024 * 1024, 10, "2000-06-16 00:00:60"};              // rate: 10, size: 100
    SceneInfo si1{"/", "Brad Pitt", {}, {"Brad Pitt.mp4"}, 99 * 1024 * 1024, 11, "2000-06-16 00:00:59"};                   // rate: 11, size: 099
    SceneInfo si2{"/", "Chris Evans", {}, {"Chris Evans.mp4"}, 98 * 1024 * 1024, 12, "2000-06-16 00:00:58"};               // rate: 12, size: 098
    SceneInfo si3{"/Brazil/", "Kaka", {}, {"Kaka.mp4"}, 90 * 1024 * 1024, 20, "2000-06-16 00:00:50"};                      // rate: 20, size: 090
    SceneInfo si4{"/French/", "Raphael Varane", {}, {"Raphael Varane.mp4"}, 89 * 1024 * 1024, 21, "2000-06-16 00:00:49"};  // rate: 21, size: 089
    SceneInfo si5{"/Spain/", "Alvaro Morata", {}, {"Alvaro Morata.mp4"}, 86 * 1024 * 1024, 24, "2000-06-16 00:00:46"};     // rate: 24, size: 086

    SceneInfoList scenesLst{si0, si1, si2, si3, si4, si5};
    QCOMPARE(scenesLst.size(), 6);
    MOCKER(SceneHelper::GetScnsLstFromPath).expects(exactly(1)).with(eq(QString{"inexist/GlobalSortPath"})).will(returnValue(scenesLst));

    Configuration().setValue(SceneKey::SCENES_COUNT_EACH_PAGE.name, 4);
    // rate descending
    ScenesListModel slm{"ScenesListViewGlobalSort"};
    slm.initSortSetting(SceneSortOrderHelper::SortDimE::RATE, true);
    QCOMPARE(slm.setRootPath("inexist/GlobalSortPath"), true);
    QCOMPARE(slm.rowCount(), 4);

    // in first page
    QCOMPARE(slm.baseName(slm.index(0)), "Alvaro Morata");
    QCOMPARE(slm.baseName(slm.index(1)), "Raphael Varane");
    QCOMPARE(slm.baseName(slm.index(2)), "Kaka");
    QCOMPARE(slm.baseName(slm.index(3)), "Chris Evans");

    // size descending
    QCOMPARE(slm.setSortDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE), true);
    QCOMPARE(slm.baseName(slm.index(0)), "Ben Affleck");
    QCOMPARE(slm.baseName(slm.index(1)), "Brad Pitt");
    QCOMPARE(slm.baseName(slm.index(2)), "Chris Evans");
    QCOMPARE(slm.baseName(slm.index(3)), "Kaka");

    // size ascending
    QCOMPARE(slm.setSortOrderReverse(false), true);
    QCOMPARE(slm.baseName(slm.index(0)), "Alvaro Morata");
    QCOMPARE(slm.baseName(slm.index(1)), "Raphael Varane");
    QCOMPARE(slm.baseName(slm.index(2)), "Kaka");
    QCOMPARE(slm.baseName(slm.index(3)), "Chris Evans");

    // in second page
    QCOMPARE(slm.onPageIndexChanged(1), true);
    QCOMPARE(slm.rowCount(), 2);
    QCOMPARE(slm.baseName(slm.index(0)), "Brad Pitt");
    QCOMPARE(slm.baseName(slm.index(1)), "Ben Affleck");
  }

  void AfterJsonFilesNameRenamed_ok() {
    // precondition si 已经按照MoviePaths升序
    SceneInfo si0{"/", "Ben Affleck", {}, {"Ben Affleck.mp4"}, 100 * 1024 * 1024, 10, "2000-06-16 00:00:60"};              // rate: 10, size: 100
    SceneInfo si1{"/", "Brad Pitt", {}, {"Brad Pitt.mp4"}, 99 * 1024 * 1024, 11, "2000-06-16 00:00:59"};                   // rate: 11, size: 099
    SceneInfo si2{"/", "Chris Evans", {}, {"Chris Evans.mp4"}, 98 * 1024 * 1024, 12, "2000-06-16 00:00:58"};               // rate: 12, size: 098
    SceneInfo si3{"/Brazil/", "Kaka", {}, {"Kaka.mp4"}, 90 * 1024 * 1024, 20, "2000-06-16 00:00:50"};                      // rate: 20, size: 090
    SceneInfo si4{"/French/", "Raphael Varane", {}, {"Raphael Varane.mp4"}, 89 * 1024 * 1024, 21, "2000-06-16 00:00:49"};  // rate: 21, size: 089
    SceneInfo si5{"/Spain/", "Alvaro Morata", {}, {"Alvaro Morata.mp4"}, 86 * 1024 * 1024, 24, "2000-06-16 00:00:46"};     // rate: 24, size: 086

    SceneInfoList scenesLst{si0, si1, si2, si3, si4, si5};
    QCOMPARE(scenesLst.size(), 6);
    MOCKER(SceneHelper::GetScnsLstFromPath).expects(exactly(1)).with(eq(QString{"inexist/RowRemovePath"})).will(returnValue(scenesLst));

    Configuration().setValue(SceneKey::SCENES_COUNT_EACH_PAGE.name, 4);
    ScenesListModel slm{"RowRemovedTest"};
    QSignalSpy pagesCntChanged(&slm, &ScenesListModel::pagesCountChanged);
    QCOMPARE(slm.setRootPath("inexist/RowRemovePath"), true);
    QCOMPARE(slm.rowCount(), 4);
    QCOMPARE(pagesCntChanged.count(), 1);
    QCOMPARE(pagesCntChanged.takeLast(), (QVariantList{2}));  // floor(6 / 4)=2

    QCOMPARE(slm.AfterJsonFilesNameRenamed({}), 0);
    QCOMPARE(slm.rowCount(), 4);

    QCOMPARE(slm.AfterJsonFilesNameRenamed({slm.index(0), slm.index(1)}), 2);
    QCOMPARE(slm.rowCount(), 4);
    QCOMPARE(pagesCntChanged.count(), 1);
    QCOMPARE(pagesCntChanged.takeLast(), (QVariantList{1}));  // floor(4 / 4)=1
    QCOMPARE(slm.baseName(slm.index(0)), "Chris Evans");
    QCOMPARE(slm.baseName(slm.index(1)), "Kaka");
    QCOMPARE(slm.baseName(slm.index(2)), "Raphael Varane");
    QCOMPARE(slm.baseName(slm.index(3)), "Alvaro Morata");

    QCOMPARE(slm.AfterJsonFilesNameRenamed({slm.index(0), slm.index(1)}), 2);
    QCOMPARE(slm.rowCount(), 2);
    QCOMPARE(pagesCntChanged.count(), 0);
    QCOMPARE(slm.baseName(slm.index(0)), "Raphael Varane");
    QCOMPARE(slm.baseName(slm.index(1)), "Alvaro Morata");
  }
};

#include "ScenesListModelTest.moc"
REGISTER_TEST(ScenesListModelTest, false)
