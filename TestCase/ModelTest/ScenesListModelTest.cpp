#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "ScenesListModel.h"
#include "EndToExposePrivateMember.h"
#include "SceneInfoManager.h"
#include "ImageTestPrecoditionTools.h"
#include <QDir>
#include <QDirIterator>
#include "TDir.h"

#include "JsonTestPrecoditionTools.h"
#include "JsonKey.h"

using namespace ImageTestPrecoditionTools;
using namespace SceneInfoManager;

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
    // Configuration().clear();
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
    QVERIFY(defaultConstruct.ChangeItemsCntIn1Page(10));
    QCOMPARE(defaultConstruct.mScenesCountPerPage, 10);
    QCOMPARE(defaultConstruct.rowCount(), 0);
    QCOMPARE(defaultConstruct.mCurBegin, defaultConstruct.mEntryList.cbegin());
    QCOMPARE(defaultConstruct.mCurEnd, defaultConstruct.mEntryList.cend());

    QVERIFY(defaultConstruct.SetPageIndex(1));
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

        QSize newSize(200, 150);
        slm.onIconSizeChange(newSize);  // change to new icon size. cache cleared
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
    SCENE_INFO_LIST manualAppendToBamboo{
        SCENE_INFO{"/",                                           //
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

      QCOMPARE(slm.GetVids(slm.index(0)).size(), 0);  // no vid, QBrush been set
      const QVariant& vidBgAtRow0 = slm.data(slm.index(0), Qt::BackgroundRole);
      QVERIFY(!vidBgAtRow0.isNull());
    }

    {  // 4. Show by page works fine 每页显示3个场景
      QCOMPARE(slm.rowCount(), 8);

      QVERIFY(slm.ChangeItemsCntIn1Page(3));
      QCOMPARE(slm.rowCount(), 3);    // 第一页显示3个
      QCOMPARE(slm.GetPageCnt(), 3);  // 总共8个场景，共3页, 3+3+2

      QVERIFY(slm.SetPageIndex(1));
      QCOMPARE(slm.mPageIndex, 1);
      QCOMPARE(slm.rowCount(), 3);  // 第二页显示3个

      QVERIFY(slm.SetPageIndex(2));
      QVERIFY(slm.SetPageIndex(2));
      QCOMPARE(slm.mPageIndex, 2);  // already there
      QCOMPARE(slm.rowCount(), 2);  // 第三页显示2个

      QVERIFY(slm.SetPageIndex(3));  // rowCount() == 0, show nothing
      QCOMPARE(slm.rowCount(), 0);   // 保持当前页

      {
        QVERIFY(!slm.SetPageIndex(-1));  // not crash down
        QVERIFY(slm.mPageIndex != -1);
      }

      // 设置显示所有场景
      QVERIFY(slm.ChangeItemsCntIn1Page(-1));
      QCOMPARE(slm.mScenesCountPerPage, -1);
      QCOMPARE(slm.rowCount(), 8);   // 显示所有场景
      QVERIFY(slm.SetPageIndex(0));  // no need setting
      QCOMPARE(slm.GetEntryIndexBE(-1, 8), (std::pair<int, int>(0, 8)));
    }
  }
};

#include "ScenesListModelTest.moc"
REGISTER_TEST(ScenesListModelTest, false)

