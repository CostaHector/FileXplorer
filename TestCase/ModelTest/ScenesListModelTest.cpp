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
#include <QDir>
#include <QDirIterator>
#include "TDir.h"

#include "JsonTestPrecoditionTools.h"
#include "JsonKey.h"
#include <random>

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
    // 一个dimension有序
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
  TDir tDir;  // 类内常驻, 不要手动清理, 由合成析构自动清理
  QString lastFight1SvgContents = GetSvgContentTemplate().arg(SVG_FILL_COLORS[0]).arg("The Last Fight 1");
  QString ironMenSvgContents = GetSvgContentTemplate().arg(SVG_FILL_COLORS[2]).arg("Iron Man");

  QList<FsNodeEntry> nodeBamboo;
  QList<FsNodeEntry> nodeGrapes;

  const QString bambooPath = tDir.itemPath("Bamboo");
  const QString grapePath = tDir.itemPath("Grapes");

 private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());
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
    QCOMPARE(tDir.createEntries(nodeBamboo), nodeBamboo.size());  // 3json
    {
      ScnMgr scnMgr1;
      QCOMPARE(scnMgr1(bambooPath), Counter(3, 3, 3, 2));  // 3updated, 3used,3VidUpdated,2ImgNameUpdated
      QCOMPARE(scnMgr1.WriteDictIntoScnFiles(), 2);        // 2 scn file writed ok
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
    QCOMPARE(tDir.createEntries(nodeGrapes), nodeGrapes.size());  // 8json. no ImgName, No VidName
    {
      ScnMgr scnMgr2;
      QCOMPARE(scnMgr2(grapePath), Counter(0, 8, 0, 0));  // 3updated, 3used,3VidUpdated,2ImgNameUpdated
      QCOMPARE(scnMgr2.WriteDictIntoScnFiles(), 2);       // 2 scn file writed ok
    }
  }

  void cleanupTestCase() {
    SceneInfoManager::mockScenesInfoList().clear();
  }

  void initalized_ok() {
    Configuration().setValue("SCENES_COUNT_EACH_PAGE", 1024);  //

    ScenesListModel defaultConstruct;
    QVERIFY(defaultConstruct.data(QModelIndex{}, Qt::DisplayRole).isNull());
    // will not crash down
    QCOMPARE(defaultConstruct.rowCount(), 0);
    QCOMPARE(defaultConstruct.mCurBegin, defaultConstruct.mEntryList.cbegin());
    QCOMPARE(defaultConstruct.mCurBegin, defaultConstruct.mEntryList.cend());
    QModelIndex invalidIndex;
    int linearInd = -1;
    QVERIFY(!defaultConstruct.isIndexValid(invalidIndex, linearInd));

    // 测试各种成员函数在空模型下的行为
    QVERIFY(defaultConstruct.fileInfo(invalidIndex).fileName().isEmpty());
    QVERIFY(defaultConstruct.filePath(invalidIndex).isEmpty());
    QVERIFY(defaultConstruct.fileName(invalidIndex).isEmpty());
    QVERIFY(defaultConstruct.baseName(invalidIndex).isEmpty());
    QVERIFY(defaultConstruct.absolutePath(invalidIndex).isEmpty());
    QVERIFY(defaultConstruct.GetImgs(invalidIndex).isEmpty());
    QVERIFY(defaultConstruct.GetVids(invalidIndex).isEmpty());

    // 测试分页功能
    QVERIFY(defaultConstruct.onScenesCountsPerPageChanged(10));
    QCOMPARE(defaultConstruct.mScenesCountPerPage, 10);
    QCOMPARE(defaultConstruct.rowCount(), 0);
    QCOMPARE(defaultConstruct.mCurBegin, defaultConstruct.mEntryList.cbegin());
    QCOMPARE(defaultConstruct.mCurEnd, defaultConstruct.mEntryList.cend());

    QVERIFY(defaultConstruct.onPageIndexChanged(1));
    QCOMPARE(defaultConstruct.mPageIndex, 1);
    QCOMPARE(defaultConstruct.GetEntryListLen(), 0);

    QCOMPARE(defaultConstruct.GetPageCnt(), 0);
    QCOMPARE(defaultConstruct.GetEntryListLen(), 0);
  }

  void setRootPath_works_ok() {
    Configuration().setValue("SCENES_COUNT_EACH_PAGE", 1024);
    SceneInfoManager::mockScenesInfoList().clear();
    ScenesListModel slm;
    {
      // 1 预期有3行, 因为有3个json读取到了
      QVERIFY(slm.setRootPath(bambooPath, false));
      QCOMPARE(slm.rootPath(), tDir.itemPath("Bamboo"));
      QCOMPARE(slm.rowCount(), 3);

      // data retrieve ok
      // rel2Scn of the first 3 row is "/", "/Avengers/", "/Avengers/" respectively.
      // we sort scenes by rel2Scn acii ascending. if rel2Scn equal, by names ascending.
      QCOMPARE(slm.data(slm.index(0), Qt::DisplayRole).toString(), "The Last Fight");   // 2 images, 1 video
      QCOMPARE(slm.data(slm.index(1), Qt::DisplayRole).toString(), "Captain America");  // 0 image, 1 video
      QCOMPARE(slm.data(slm.index(2), Qt::DisplayRole).toString(), "Iron Man");         // 1 image, 1 video

      {
        QCOMPARE(slm.GetImgs(slm.index(0)).size(), 2);
        QCOMPARE(slm.GetImgs(slm.index(1)).size(), 0);
        QCOMPARE(slm.GetImgs(slm.index(2)).size(), 1);
        const QVariant& imgAtRow0Valid = slm.data(slm.index(0), Qt::DecorationRole);
        const QVariant& imgAtRow1Empty = slm.data(slm.index(1), Qt::DecorationRole);
        const QVariant& imgAtRow2Valid = slm.data(slm.index(2), Qt::DecorationRole);
        QVERIFY(isValidPixmap(imgAtRow0Valid));
        QVERIFY(!isValidPixmap(imgAtRow1Empty));
        QVERIFY(isValidPixmap(imgAtRow2Valid));

        // this time read it from pixmap directly
        QStringList imgsAtFirstRow = slm.GetImgs(slm.index(0));
        const QString firstImagePath = tDir.itemPath("Bamboo/The Last Fight 1.jpg");
        QCOMPARE(imgsAtFirstRow.size(), 2);
        QCOMPARE(imgsAtFirstRow.front(), firstImagePath);
        QCOMPARE(slm.mCurBegin[0].GetFirstImageAbsPath(slm.rootPath()), firstImagePath);
        QVERIFY(slm.mPixCache.find(firstImagePath, nullptr));
        slm.data(slm.index(0), Qt::DecorationRole);

        QSize widthSize(1000, 10);
        slm.onIconSizeChange(widthSize);  // change to new icon size. cache cleared
        QVERIFY(!slm.mPixCache.find(firstImagePath, nullptr));

        QSize heightSize(10, 1000);
        slm.onIconSizeChange(heightSize);  // change to new icon size. cache cleared
        QVERIFY(!slm.mPixCache.find(firstImagePath, nullptr));
      }

      {
        QCOMPARE(slm.GetVids(slm.index(0)).size(), 1);
        QCOMPARE(slm.GetVids(slm.index(1)).size(), 1);
        QCOMPARE(slm.GetVids(slm.index(2)).size(), 1);
        const QVariant& vidBgAtRow0 = slm.data(slm.index(0), Qt::BackgroundRole);
        const QVariant& vidBgAtRow1 = slm.data(slm.index(1), Qt::BackgroundRole);
        const QVariant& vidBgAtRow2 = slm.data(slm.index(2), Qt::BackgroundRole);
        QVERIFY(vidBgAtRow0.isNull());
        QVERIFY(vidBgAtRow1.isNull());
        QVERIFY(vidBgAtRow2.isNull());
      }

      // baseName: finally from json file base name, directly from scn contens
      QCOMPARE(slm.baseName(slm.index(0)), "The Last Fight");
      QCOMPARE(slm.baseName(slm.index(1)), "Captain America");
      QCOMPARE(slm.baseName(slm.index(2)), "Iron Man");

      QCOMPARE(slm.absolutePath(slm.index(0)), tDir.itemPath("Bamboo") + "/");
      QCOMPARE(slm.absolutePath(slm.index(1)), tDir.itemPath("Bamboo") + "/Avengers/");
      QCOMPARE(slm.absolutePath(slm.index(2)), tDir.itemPath("Bamboo") + "/Avengers/");

      QCOMPARE(slm.filePath(slm.index(0)), tDir.itemPath("Bamboo") + "/" + "The Last Fight.mp4");
      QCOMPARE(slm.filePath(slm.index(1)), tDir.itemPath("Bamboo") + "/Avengers/" + "Captain America.mp4");
      QCOMPARE(slm.filePath(slm.index(2)), tDir.itemPath("Bamboo") + "/Avengers/" + "Iron Man.mp4");

      QCOMPARE(slm.fileInfo(slm.index(0)).absoluteFilePath(), slm.filePath(slm.index(0)));
      QCOMPARE(slm.fileInfo(slm.index(1)).absoluteFilePath(), slm.filePath(slm.index(1)));
      QCOMPARE(slm.fileInfo(slm.index(2)).absoluteFilePath(), slm.filePath(slm.index(2)));

      QCOMPARE(slm.fileName(slm.index(0)), "The Last Fight.mp4");
      QCOMPARE(slm.fileName(slm.index(1)), "Captain America.mp4");
      QCOMPARE(slm.fileName(slm.index(2)), "Iron Man.mp4");
    }

    // 2. 手动增加一条json, 测试row增加
    SceneInfoList manualAppendToBamboo{
        SceneInfo{"/",                                           //
                   "Henry Cavill",                                //
                   {"Henry Cavill 1.jpg", "Henry Cavill 2.jpg"},  //
                   {"Henry Cavill Biology.mp4"},                  //
                   50 * 1024 * 1024,                              //
                   99,                                            //
                   ""},                                           //
    };
    SceneInfoManager::mockScenesInfoList().append(manualAppendToBamboo);
    ON_SCOPE_EXIT {
      SceneInfoManager::mockScenesInfoList().clear();
    };

    {  // 2.1 预期不刷新, 还是3行
      QVERIFY(!slm.setRootPath(bambooPath, false));
      QCOMPARE(slm.rowCount(), 3);
    }
    {  // 2.2 force模式下, 预期row会刷新为3+1 mock 行
      QVERIFY(slm.setRootPath(bambooPath, true));
      QCOMPARE(slm.rowCount(), 4);
    }
    SceneInfoManager::mockScenesInfoList().clear();

    {  // 3. 路径切换, 预期row会刷新为8行, 因为有8个json
      QVERIFY(slm.setRootPath(grapePath, true));
      QCOMPARE(slm.rowCount(), 8);
      // here the first 4 rel2Scn are "/", sort sort by name ascending
      QCOMPARE(slm.data(slm.index(0), Qt::DisplayRole).toString(), "Jane Grey");           // 0 image, 0 video
      QCOMPARE(slm.data(slm.index(1), Qt::DisplayRole).toString(), "Michael Fassbender");  // 0 image, 0 video
      QCOMPARE(slm.data(slm.index(2), Qt::DisplayRole).toString(), "Storm");               // 0 image, 0 video
      QCOMPARE(slm.data(slm.index(3), Qt::DisplayRole).toString(), "The Last Fight");      // 0 image, 0 video

      QCOMPARE(slm.GetImgs(slm.index(0)).size(), 0);  // no images, no pixmap at all
      const QVariant& imgAtRow0NotExist = slm.data(slm.index(0), Qt::DecorationRole);
      QVERIFY(!isValidPixmap(imgAtRow0NotExist));

      QCOMPARE(slm.GetVids(slm.index(0)).size(), 1);  // no vid, return a placeholder vidName
      const QVariant& vidBgAtRow0 = slm.data(slm.index(0), Qt::BackgroundRole);
      QVERIFY(!vidBgAtRow0.isNull());
    }

    {  // 4. Show by page works fine 每页显示3个场景
      QCOMPARE(slm.rowCount(), 8);

      QVERIFY(slm.onScenesCountsPerPageChanged(3));
      QCOMPARE(slm.rowCount(), 3);    // 第一页显示3个
      QCOMPARE(slm.GetPageCnt(), 3);  // 总共8个场景，共3页, 3+3+2

      QVERIFY(slm.onPageIndexChanged(1));
      QCOMPARE(slm.mPageIndex, 1);
      QCOMPARE(slm.rowCount(), 3);  // 第二页显示3个

      QVERIFY(slm.onPageIndexChanged(2));
      QVERIFY(slm.onPageIndexChanged(2));
      QCOMPARE(slm.mPageIndex, 2);  // already there
      QCOMPARE(slm.rowCount(), 2);  // 第三页显示2个

      QVERIFY(slm.onPageIndexChanged(3));  // rowCount() == 0, show nothing
      QCOMPARE(slm.rowCount(), 0);         // 保持当前页

      {
        QVERIFY(!slm.onPageIndexChanged(-1));  // not crash down
        QVERIFY(slm.mPageIndex != -1);
      }

      // 设置显示所有场景
      QVERIFY(slm.onScenesCountsPerPageChanged(-1));
      QCOMPARE(slm.mScenesCountPerPage, -1);
      QCOMPARE(slm.rowCount(), 8);         // 显示所有场景
      QVERIFY(slm.onPageIndexChanged(0));  // no need setting
      QCOMPARE(slm.GetEntryIndexBE(-1, 8), (std::pair<int, int>(0, 8)));
    }
  }

  void sort_only_one_dimension_sorted() {
    Configuration().setValue("SCENES_COUNT_EACH_PAGE", 1024);

    ScenesListModel slm;
    SceneSortProxyModel sspm;
    sspm.setSourceModel(&slm);
    {  // MOVIE_PATH
      SceneInfoList mockMoviePathAscendingList;
      QVERIFY(GetSceneInfoList(10, SceneSortOrderHelper::SortDimE::MOVIE_PATH, false, mockMoviePathAscendingList));
      SceneInfoManager::mockScenesInfoList() = mockMoviePathAscendingList;
      QCOMPARE(slm.setRootPath("any/inexists/path1"), true);
      QCOMPARE(slm.rowCount(), 10);
      // 升
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::AscendingOrder);
      checkIndexMatch(slm, 0, sspm, 0);
      checkIndexMatch(slm, 1, sspm, 1);
      checkIndexMatch(slm, 8, sspm, 8);
      checkIndexMatch(slm, 9, sspm, 9);
      // 降
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::DescendingOrder);
      checkIndexMatch(slm, 0, sspm, 9);
      checkIndexMatch(slm, 1, sspm, 8);
      checkIndexMatch(slm, 8, sspm, 1);
      checkIndexMatch(slm, 9, sspm, 0);
    }

    // 测试MOVIE_SIZE排序
    {
      SceneInfoList mockMovieSizeDescendingList;  // 降序
      QVERIFY(GetSceneInfoList(10, SceneSortOrderHelper::SortDimE::MOVIE_SIZE, true, mockMovieSizeDescendingList));
      SceneInfoManager::mockScenesInfoList() = mockMovieSizeDescendingList;
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
      SceneInfoList mockRateAscendingList;  // 升
      QVERIFY(GetSceneInfoList(10, SceneSortOrderHelper::SortDimE::RATE, false, mockRateAscendingList));
      SceneInfoManager::mockScenesInfoList() = mockRateAscendingList;
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
      SceneInfoList mockUploadedDescendingList;  // 降
      QVERIFY(GetSceneInfoList(11, SceneSortOrderHelper::SortDimE::UPLOADED_TIME, true, mockUploadedDescendingList));
      SceneInfoManager::mockScenesInfoList() = mockUploadedDescendingList;
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

  void sort_no_directory_system_ok() {
    // precondition
    Configuration().setValue("SCENES_COUNT_EACH_PAGE", 1024);

    SceneInfo si0{"/", "Ben Affleck", {}, {"Ben Affleck.mp4"}, 100 * 1024 * 1024, 10, "2000-06-16 00:00:60"};
    SceneInfo si1{"/", "Brad Pitt", {}, {"Brad Pitt.mp4"}, 99 * 1024 * 1024, 11, "2000-06-16 00:00:59"};
    SceneInfo si2{"/", "Chris Evans", {}, {"Chris Evans.mp4"}, 98 * 1024 * 1024, 12, "2000-06-16 00:00:58"};
    SceneInfo si3{"/", "Chris Hemsworth", {}, {"Chris Hemsworth.mp4"}, 97 * 1024 * 1024, 13, "2000-06-16 00:00:57"};
    SceneInfo si4{"/", "Chris Pine", {}, {"Chris Pine.mp4"}, 96 * 1024 * 1024, 14, "2000-06-16 00:00:56"};
    SceneInfo si5{"/", "Chris Pratt", {}, {"Chris Pratt.mp4"}, 95 * 1024 * 1024, 15, "2000-06-16 00:00:55"};
    SceneInfo si6{"/", "Henry Cavill", {}, {"Henry Cavill.mp4"}, 94 * 1024 * 1024, 16, "2000-06-16 00:00:54"};
    SceneInfo si7{"/", "Keanu Reeves", {}, {"Keanu Reeves.mp4"}, 93 * 1024 * 1024, 17, "2000-06-16 00:00:53"};
    SceneInfo si8{"/", "Michael Fassbender", {}, {"Michael Fassbender.mp4"}, 92 * 1024 * 1024, 18, "2000-06-16 00:00:52"};
    SceneInfo si9{"/", "Tom Cruise", {}, {"Tom Cruise.mp4"}, 91 * 1024 * 1024, 19, "2000-06-16 00:00:51"};
    SceneInfo si10{"/Brazil/", "Kaka", {}, {"Kaka.mp4"}, 90 * 1024 * 1024, 20, "2000-06-16 00:00:50"};
    SceneInfo si11{"/French/", "Raphael Varane", {}, {"Raphael Varane.mp4"}, 89 * 1024 * 1024, 21, "2000-06-16 00:00:49"};
    SceneInfo si12{"/Polish/", "Robert Lewandowski", {}, {"Robert Lewandowski.mp4"}, 88 * 1024 * 1024, 22, "2000-06-16 00:00:48"};
    SceneInfo si13{"/Portuguese/", "Cristiano Ronaldo", {}, {"Cristiano Ronaldo.mp4"}, 87 * 1024 * 1024, 23, "2000-06-16 00:00:47"};
    SceneInfo si14{"/Spain/", "Alvaro Morata", {}, {"Alvaro Morata.mp4"}, 86 * 1024 * 1024, 24, "2000-06-16 00:00:46"};

    SceneInfoList manualAppendToAInExistPath{
        si0, si1, si2, si3, si4, si5, si6, si7, si8, si9, si10, si11, si12, si13, si14  //
    };
    QCOMPARE(manualAppendToAInExistPath.size(), 15);
    SceneInfoManager::mockScenesInfoList() = manualAppendToAInExistPath;  // the mock works here

    ScenesListModel slm;
    SceneSortProxyModel sspm;
    {
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::AscendingOrder);  // nothing happend
      sspm.setSourceModel(nullptr);                                                  // ignored
      sspm.setSourceModel(&slm);
      sspm.setSourceModel(&slm);  // ignored
    }

    {
      QCOMPARE(sspm.sortOrder(), Qt::SortOrder::AscendingOrder);
      QCOMPARE(sspm.m_sortDimension, SceneSortOrderHelper::DEFAULT_SCENE_SORT_ORDER);
      QVERIFY(sspm.mComparator != nullptr);
      QVERIFY(sspm.m_sourceModel != nullptr);
    }

    {  // signal pagesCountChanged ok
      QCOMPARE(slm.mScenesCountPerPage, 1024);
      constexpr int EXPECT_PAGES_COUNT = 1;  // ceil(15 / 1024)
      QSignalSpy pagesCntChanged(&slm, &ScenesListModel::pagesCountChanged);
      QCOMPARE(slm.setRootPath("any/inexists/path"), true);
      QCOMPARE(pagesCntChanged.count(), 1);

      QVariantList countChangedToParams = pagesCntChanged.back();
      QCOMPARE(countChangedToParams.size(), 1);
      QCOMPARE(countChangedToParams[0].toInt(), EXPECT_PAGES_COUNT);
      QCOMPARE(slm.GetPageCnt(), EXPECT_PAGES_COUNT);
      QCOMPARE(slm.rowCount(), 15);
    }

    // Precondition: there are 15 items in source model, and MoviePath(ascending), MovieSize(Desc), Rate(Asce), Uploaded(Desc)
    QCOMPARE(slm.rowCount(), 15);
    QCOMPARE(sspm.sortOrder(), Qt::SortOrder::AscendingOrder);  // by default ascending in MoviePath
    {
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - 1));

      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 15 - 0 - 1));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 15 - 1 - 1));
      QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - (15 - 1) - 1));
    }

    {  //
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::AscendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 15 - 0 - 1));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 15 - 1 - 1));
      QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - (15 - 1) - 1));

      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - 1));
    }

    {
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::RATE, Qt::AscendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - 1));

      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::RATE, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 15 - 0 - 1));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 15 - 1 - 1));
      QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - (15 - 1) - 1));
    }

    {  //
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::UPLOADED_TIME, Qt::AscendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 15 - 0 - 1));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 15 - 1 - 1));
      QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - (15 - 1) - 1));

      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::UPLOADED_TIME, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 15 - 1, sspm, 15 - 1));
    }

    // 2.0 设置10cnt/page, 第1页有10个, 第2页有5个
    QVERIFY(slm.mScenesCountPerPage > 15);
    QCOMPARE(slm.rowCount(), 15);

    slm.onScenesCountsPerPageChanged(10);
    slm.onPageIndexChanged(0);
    QCOMPARE(slm.rowCount(), 10);

    slm.onPageIndexChanged(1);
    QCOMPARE(slm.rowCount(), 5);

    {
      // 本就是MOVIE_PATH 升序
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::AscendingOrder); // ascending
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 4, sspm, 4));

      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_PATH, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 4));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 3));
      QVERIFY(checkIndexMatch(slm, 4, sspm, 0));
    }
    {
      // 本就是MOVIE_SIZE 降序
      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::AscendingOrder); // ascending
      QVERIFY(checkIndexMatch(slm, 0, sspm, 4));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 3));
      QVERIFY(checkIndexMatch(slm, 4, sspm, 0));

      sspm.sortByFieldDimension(SceneSortOrderHelper::SortDimE::MOVIE_SIZE, Qt::DescendingOrder);
      QVERIFY(checkIndexMatch(slm, 0, sspm, 0));
      QVERIFY(checkIndexMatch(slm, 1, sspm, 1));
      QVERIFY(checkIndexMatch(slm, 4, sspm, 4));
    }
  }
};

#include "ScenesListModelTest.moc"
REGISTER_TEST(ScenesListModelTest, false)
