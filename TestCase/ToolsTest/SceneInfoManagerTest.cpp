#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "SceneInfoManager.h"
#include "EndToExposePrivateMember.h"

#include "JsonHelper.h"
#include "TDir.h"
#include "JsonKey.h"

using namespace SceneInfoManager;

class SceneInfoManagerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
 private slots:
  void initTestCase() { QVERIFY(tDir.IsValid()); }
  void init() { tDir.ClearAll(); }

  void UpdateJsonUnderAPath_correct() {
    /*
Precondition: Xander.json value of key ImgName, VidName is empty
   1. 无效路径(不存在 或者 文件而不是文件夹) 返回{0, 0, 0, 0}
   2. 文件夹内直接层级无任何无json文件, 子层级有, 返回{0, 0, 0, 0}
   3. 文件夹内只有{Xander.json}文件 key齐全, 返回{0, 1, 0, 0}; 再次调用, 返回{0,1,0,0}, no need update
   4. 文件夹内只有{Xander.json, Xander.jpg}文件, 返回{1,1,1,0}; 再次调用, 返回{0,1,0,0}
   5. 文件夹内只有{Xander.json, Xander.mp4}文件, 返回{1,1,0,1}; 再次调用, 返回{0,1,0,0}
   6. 文件夹内只有{Xander.json, Xander.jpg, Xander.mp4}文件, 返回{1,1,1,1}; 再次调用, 返回{0,1,0,0}
保护性测试
   7. 文件夹内只有{MichaelNoNameKeyUseless.json, MichaelNoNameKey.jpg,
      FassbenderNameKeyDifferToFileName.json, FassbenderNameKeyDifferToFileName.mp4,
      ChrisOnlyNameKey.json, ChrisOnlyNameKey.jpg, ChrisOnlyNameKey.mp4} 七个文件, key齐全不齐全
      返回{2(更新了),2(有效json),2(img更新),2(vid更新)}
     */
    QVERIFY(tDir.IsValid());
    {  // 1.
      ScnMgr scnMgr;
      QVERIFY(scnMgr.UpdateJsonUnderAPath("/path/to/inexist").isEmpty());
      QVERIFY(scnMgr.UpdateJsonUnderAPath(__FILE__).isEmpty());
      QVERIFY(scnMgr.m_jsonsDicts.isEmpty());
    }
    using namespace JsonKey;
    {  // 2.
      QList<FsNodeEntry> notExistJsonFileNodes{
          {"some file.mp4", false, ""},               // file
          {"some file", true, ""},                    // folder
          {"some file/In subfoler.json", false, ""},  // inside folder. ignored
      };
      QCOMPARE(tDir.createEntries(notExistJsonFileNodes), 3);
      ScnMgr scnMgr;
      QVERIFY(scnMgr.UpdateJsonUnderAPath(tDir.path()).isEmpty());
      QVERIFY(scnMgr.m_jsonsDicts.isEmpty());
      tDir.ClearAll();
    }

    {
      QList<FsNodeEntry> onlyOneJsonFileNodes{
          {"Xander.json", false, ConstructJsonByteArray("Xander")},
      };
      QCOMPARE(tDir.createEntries(onlyOneJsonFileNodes), 1);
      ScnMgr scnMgr;
      QCOMPARE(scnMgr.UpdateJsonUnderAPath(tDir.path()), Counter(0, 1, 0, 0));
      QCOMPARE(scnMgr.UpdateJsonUnderAPath(tDir.path()), Counter(0, 1, 0, 0));
      QCOMPARE(scnMgr.m_jsonsDicts.size(), 1);
      tDir.ClearAll();
    }

    {
      QList<FsNodeEntry> jsonAndImgNodes{
          {"Xander.json", false, ConstructJsonByteArray("Xander")},
          {"Xander.jpg", false, ""},
      };
      QCOMPARE(tDir.createEntries(jsonAndImgNodes), 2);
      QVERIFY(tDir.checkFileContents("Xander.json", {}, {"Xander.jpg"}));
      ScnMgr scnMgr;
      QCOMPARE(scnMgr.UpdateJsonUnderAPath(tDir.path()), Counter(1, 1, 1, 0));
      QVERIFY(tDir.checkFileContents("Xander.json", {"Xander.jpg"}));
      QCOMPARE(scnMgr.UpdateJsonUnderAPath(tDir.path()), Counter(0, 1, 0, 0));
      tDir.ClearAll();
    }

    {  // 5
      const QByteArray mp4FileContentByteArray{"Contents in Xander.mp4"};
      QCOMPARE(mp4FileContentByteArray.size(), 22);
      QList<FsNodeEntry> jsonAndVidNodes{
          {"Xander.json", false, ConstructJsonByteArray("Xander")},
          {"Xander.mp4", false, mp4FileContentByteArray},
      };
      QCOMPARE(tDir.createEntries(jsonAndVidNodes), 2);
      QVERIFY(tDir.checkFileContents("Xander.json", {}, {"Xander.mp4"}));
      ScnMgr scnMgr;
      QCOMPARE(scnMgr.UpdateJsonUnderAPath(tDir.path()), Counter(1, 1, 0, 1));
      QVERIFY(tDir.checkFileContents("Xander.json", {"Xander.mp4", "22"}));  // Size changed to 22
      QCOMPARE(scnMgr.UpdateJsonUnderAPath(tDir.path()), Counter(0, 1, 0, 0));
      tDir.ClearAll();
    }

    {  // 6
      QList<FsNodeEntry> allNodes{
          {"Xander.json", false, ConstructJsonByteArray("Xander")},
          {"Xander.jpg", false, ""},
          {"Xander.mp4", false, ""},
      };
      QCOMPARE(tDir.createEntries(allNodes), 3);
      QVERIFY(tDir.checkFileContents("Xander.json", {}, {"Xander.jpg", "Xander.mp4"}));
      ScnMgr scnMgr;
      QCOMPARE(scnMgr.UpdateJsonUnderAPath(tDir.path()), Counter(1, 1, 1, 1));
      QVERIFY(tDir.checkFileContents("Xander.json", {"Xander.jpg", "Xander.mp4"}));
      QCOMPARE(scnMgr.UpdateJsonUnderAPath(tDir.path()), Counter(0, 1, 0, 0));
      tDir.ClearAll();
    }
    {  // 7. protection: it cannot write into what we don't want to
      QList<FsNodeEntry> mixedNodes{
          // Michael 文件 - 没有 Name 键的 JSON, This JSON has no Name key
          {"MichaelNoNameKeyUseless.json", false, R"({"Rate":3})"},  //
          {"MichaelNoNameKey.jpg", false, ""},                       //
          // Fassbender 文件 - value on key Name differ to its json file baseName. changed it to filename instead
          {"FassbenderNameKeyDifferToFileName.json", false, R"({"Name":"Differ to its json file base Name"})"},  //
          {"FassbenderNameKeyDifferToFileName.mp4", false, ""},                                                  //
          // Chris 文件 - 只有 Name 键的 JSON. This JSON only has Name and Description key
          {"ChrisOnlyNameKey.json", false, R"({"Name":"ChrisOnlyNameKey"})"},  //
          {"ChrisOnlyNameKey.jpg", false, ""},                                 //
          {"ChrisOnlyNameKey.mp4", false, ""},                                 //
      };

      QCOMPARE(tDir.createEntries(mixedNodes), 2 + 2 + 3);

      QVERIFY(tDir.checkFileContents("MichaelNoNameKeyUseless.json", {}, {"ImgName", "VidName"}));
      QVERIFY(tDir.checkFileContents("FassbenderNameKeyDifferToFileName.json", {"Differ to its json file base Name"}, {"ImgName", "VidName"}));
      QVERIFY(tDir.checkFileContents("ChrisOnlyNameKey.json", {"ChrisOnlyNameKey"}, {"ChrisOnlyNameKey.jpg", "ChrisOnlyNameKey.mp4"}));

      ScnMgr scnMgr;
      Counter result = scnMgr.UpdateJsonUnderAPath(tDir.path());  // first call
      QCOMPARE(result, Counter(2, 2, 2, 2));                      // only FassbenderNameKeyDifferToFileName.json/ChrisOnlyNameKey.json is usefull

      QVERIFY(tDir.checkFileContents("MichaelNoNameKeyUseless.json", {}, {"ImgName", "VidName"}));  // unmodified
      QVERIFY(tDir.checkFileContents("FassbenderNameKeyDifferToFileName.json", {"FassbenderNameKeyDifferToFileName", "ImgName", "VidName"},
                                     {"Differ to its json file base Name"}));  // Name synced with file base name, and including img/vid
      QVERIFY(tDir.checkFileContents("ChrisOnlyNameKey.json", {"ChrisOnlyNameKey.jpg", "ChrisOnlyNameKey.mp4"}));  // including img/vid

      Counter secondResult = scnMgr.UpdateJsonUnderAPath(tDir.path());  // second call. no update
      QCOMPARE(secondResult, Counter(0, 2, 0, 0));

      tDir.ClearAll();
    }
  }

  void WriteDictIntoScnFiles_correct() {
    QVERIFY(tDir.IsValid());
    using namespace JsonKey;
    {  // 1.0 ScnMgr.m_jsonsDicts为空, 写入跳过, 返回0
      ScnMgr scnMgr;
      scnMgr.mockJsonDictForTest({});
      QVERIFY(scnMgr.m_jsonsDicts.isEmpty());
      QCOMPARE(scnMgr.WriteDictIntoScnFiles(), 0);
      QCOMPARE(tDir.Snapshot(), (QSet<QString>{}));
    }

    {  // 2.0 构造一个路径, 并设置m_jsonsDicts非空, .scn写入路径成功, 名称就是${父文件夹名称.scn}, 清理m_jsonsDicts
      QVERIFY(tDir.mkpath("SuperHero"));
      const QList<QVariantHash> dictsUnderSuperHero  //
          {
              ConstructJsonDict("Chris Evans"),   //
              ConstructJsonDict("Henry Cavill"),  //
          };

      ScnMgr scnMgr;
      const ScnMgr::PATH_JSON_DICT_LIST tempPath2Dicts{
          {tDir.itemPath("SuperHero"), dictsUnderSuperHero},  //
      };                                                      //
      scnMgr.mockJsonDictForTest(tempPath2Dicts);
      QVERIFY(!scnMgr.m_jsonsDicts.isEmpty());
      QCOMPARE(scnMgr.WriteDictIntoScnFiles(), 1);
      QVERIFY(scnMgr.m_jsonsDicts.isEmpty());

      QCOMPARE(tDir.Snapshot(), (QSet<QString>{
                                    "SuperHero",
                                    "SuperHero/SuperHero.scn",
                                }));
      QVERIFY(tDir.checkFileContents("SuperHero/SuperHero.scn", {"Chris Evans", "Henry Cavill"}));
      tDir.ClearAll();
    }

    { /* 3.0 构造两个路径, 并设置m_jsonsDicts两个key,
SuperHero和XMen, SuperHero下有两个非空dict;  XMen下有一个空dict, 一个非空
预期两个.scn写入路径成功, 名称就是SuperHero.scn, XMen.scn, 简单检查内容
清理m_jsonsDicts*/
      QVERIFY(tDir.mkpath("SuperHero"));
      QVERIFY(tDir.mkpath("XMen"));

      // 准备数据
      const QList<QVariantHash> dictsUnderSuperHero{
          ConstructJsonDict("Chris Evans"),
          ConstructJsonDict("Henry Cavill"),
      };

      const QList<QVariantHash> dictsUnderXMen{
          QVariantHash(),  // 空字典
          ConstructJsonDict("Wolverine"),
      };

      ScnMgr scnMgr;
      const ScnMgr::PATH_JSON_DICT_LIST tempPath2Dicts{
          {tDir.itemPath("SuperHero"), dictsUnderSuperHero},  //
          {tDir.itemPath("XMen"), dictsUnderXMen},            //
      };
      scnMgr.mockJsonDictForTest(tempPath2Dicts);
      QCOMPARE(scnMgr.m_jsonsDicts.size(), 2);

      // 执行写入操作
      QCOMPARE(scnMgr.WriteDictIntoScnFiles(), 2);  // 预期生成两个.scn文件
      QVERIFY(scnMgr.m_jsonsDicts.isEmpty());       // 验证字典被清空

      // 验证文件系统
      QCOMPARE(tDir.Snapshot(), (QSet<QString>{
                                    "SuperHero",
                                    "SuperHero/SuperHero.scn",
                                    "XMen",
                                    "XMen/XMen.scn",
                                }));

      QVERIFY(tDir.checkFileContents("SuperHero/SuperHero.scn", {"Chris Evans", "Henry Cavill"}, {"Wolverine"}));
      QVERIFY(tDir.checkFileContents("XMen/XMen.scn", {"Wolverine"}, {"Chris Evans", "Henry Cavill"}));
      tDir.ClearAll();
    }

    {  // 4. 边界情况测试：空场景列表
      ScnMgr scnMgr;
      ScnMgr::PATH_JSON_DICT_LIST testData;

      QVERIFY(tDir.mkpath("Empty"));  // 创建空目录
      testData[tDir.itemPath("Empty")] = {};
      scnMgr.mockJsonDictForTest(testData);
      // 应该不生成 .scn 文件（因为场景列表完全为空）
      QCOMPARE(scnMgr.WriteDictIntoScnFiles(), 0);
      // 验证没有生成 .scn 文件
      QString scnFilePath = tDir.itemPath("Empty/Empty.scn");
      QVERIFY(!QFile::exists(scnFilePath));
      tDir.ClearAll();
    }
  }

  /* 3. 各个层级的json都能扫描出来
   */

  void operator_caller_test() {
    QVERIFY(tDir.IsValid());
    QVERIFY(tDir.IsEmpty());
    using namespace JsonKey;

    {  // 0. not a valid directory, skip
      ScnMgr scnMgr;
      QVERIFY(scnMgr.operator()("/path/to/not eixst path").isEmpty());
      QVERIFY(scnMgr.operator()(__FILE__).isEmpty());
      QVERIFY(scnMgr.operator()(tDir.path()).isEmpty());
    }

    {  // 1.
      QList<FsNodeEntry> mixedLevelNodes{
          {"Marvel/Captain American.json", false, ConstructJsonByteArray("Captain American")},  //
          {"Marvel/Captain American.jpg", false, ""},                                           //
          {"Marvel/Captain American.mp4", false, ""},                                           //
          {"Marvel/Bonus/Chris Evans.json", false, ConstructJsonByteArray("Chris Evans")},      //
          {"Marvel/Bonus/Chris Evans.mp4", false, ""},                                          //
          {"Disney.json", false, ConstructJsonByteArray("Disney")},                             //
          {"Disney.mp4", false, ""},                                                            //
          {"Disney.jpg", false, ""},                                                            //
      };
      QCOMPARE(tDir.createEntries(mixedLevelNodes), 3 + 2 + 3);
      ScnMgr scnMgr;

      // 第一次调用：应处理所有层级的JSON文件
      Counter result = scnMgr(tDir.path());

      QCOMPARE(result, Counter(3, 3, 2, 3));  // 预期：更新3个JSON，使用3个JSON，更新2个图片字段，更新3个视频字段

      // 验证文件更新
      // 根目录：Disney.json
      // Marvel目录：Captain American.json
      // Marvel/Bonus目录：Chris Evans.json, 验证图片字段未更新（Chris Evans没有图片）
      QVERIFY(tDir.checkFileContents("Disney.json", {"Disney", "Disney.jpg", "Disney.mp4"}));
      QVERIFY(tDir.checkFileContents("Marvel/Captain American.json", {"Captain American", "Captain American.jpg", "Captain American.mp4"}));
      QVERIFY(tDir.checkFileContents("Marvel/Bonus/Chris Evans.json", {"Chris Evans", "Chris Evans.mp4"}, {"Chris Evans.jpg"}));
      const QStringList& beforeList = tDir.FilesContentsSnapshot({"Disney.json", "Marvel/Captain American.json", "Marvel/Bonus/Chris Evans.json"});
      QCOMPARE(scnMgr.m_jsonsDicts.size(), 3);  // 3 json files in used in total QCOMPARE(scnMgr.WriteDictIntoScnFiles(), 3);
      // 3 scn file in total QVERIFY(scnMgr.m_jsonsDicts.isEmpty());

      // 第二次调用：不应有更新, 文件未改变
      Counter secondResult = scnMgr(tDir.path());
      QCOMPARE(secondResult, Counter(0, 3, 0, 0));  // 预期：{0,3,0,0}
      const QStringList& afterList = tDir.FilesContentsSnapshot({"Disney.json", "Marvel/Captain American.json", "Marvel/Bonus/Chris Evans.json"});
      QCOMPARE(afterList, beforeList);

      tDir.ClearAll();
    }
  }
  void test_ParseAScnFile_Integration() {
    QVERIFY(tDir.IsValid());
    using namespace JsonKey;
    {  // 1. 基本功能测试：单个组场景
      // 准备测试数据
      QVariantHash scene1;
      scene1["Name"] = "Captain America";
      scene1["ImgName"] = QStringList{"cap1.jpg", "cap2.jpg"};
      scene1["VidName"] = "captain_america.mp4";
      scene1["Size"] = 1024 * 1024 * 500;  // 500MB
      scene1["Rate"] = 85;
      scene1["Uploaded"] = "20241212 12:50:50";

      QList<QVariantHash> scenes{scene1};

      // 创建目录并设置 ScnMgr
      QVERIFY(tDir.mkpath("Marvel"));
      ScnMgr scnMgr;
      ScnMgr::PATH_JSON_DICT_LIST testData{{tDir.itemPath("Marvel"), scenes}};
      scnMgr.mockJsonDictForTest(testData);

      // 生成 .scn 文件
      QCOMPARE(scnMgr.WriteDictIntoScnFiles(), 1);

      // 验证 .scn 文件存在
      QString scnFilePath = tDir.itemPath("Marvel/Marvel.scn");
      QVERIFY(QFile::exists(scnFilePath));

      // 解析 .scn 文件
      SCENE_INFO_LIST parsedScenes =
          ParseAScnFile(scnFilePath, "/Marvel/");  //  here mRootPath=tDir.path(); tDir.path() + "/Marvel/" + Marvel.scn=the json scn

      // 验证解析结果
      QCOMPARE(parsedScenes.size(), 1);

      const SCENE_INFO& parsedScene = parsedScenes.first();
      QCOMPARE(parsedScene.rel2scn, "/Marvel/");
      QCOMPARE(parsedScene.name, "Captain America");
      QCOMPARE(parsedScene.imgs, QStringList({"cap1.jpg", "cap2.jpg"}));
      QCOMPARE(parsedScene.vidName, "captain_america.mp4");
      QCOMPARE(parsedScene.vidSize, 1024 * 1024 * 500);
      QCOMPARE(parsedScene.rate, 85);
      QCOMPARE(parsedScene.uploaded, "20241212 12:50:50");


      { // bounder test
        QVERIFY(ParseAScnFile("/home/to/inexist path", "").isEmpty());
      }

      tDir.ClearAll();
    }

    {  // 2. 多个组场景测试
      QVariantHash scene1;
      scene1["Name"] = "Iron Man";
      scene1["ImgName"] = QStringList{"iron1.jpg"};
      scene1["VidName"] = "iron_man.mp4";
      scene1["Size"] = 1024 * 1024 * 700;
      scene1["Rate"] = 92;
      scene1["Uploaded"] = "20241212 13:00:00";

      QVariantHash scene2;
      scene2["Name"] = "Thor";
      scene2["ImgName"] = QStringList{"thor1.jpg", "thor2.jpg", "thor3.jpg"};
      scene2["VidName"] = "thor.mp4";
      scene2["Size"] = 1024 * 1024 * 800;
      scene2["Rate"] = 88;
      scene2["Uploaded"] = "20241212 14:00:00";

      QVariantHash scene3;  // 没有图片的场景
      scene3["Name"] = "Hulk";
      scene3["ImgName"] = QStringList();
      scene3["VidName"] = "hulk.mp4";
      scene3["Size"] = 1024 * 1024 * 900;
      scene3["Rate"] = 0;  // 未评分
      scene3["Uploaded"] = "20241212 15:00:00";

      QList<QVariantHash> scenes{scene1, scene2, scene3};

      // 创建目录并设置 ScnMgr
      QVERIFY(tDir.mkpath("Avengers"));
      ScnMgr scnMgr;
      ScnMgr::PATH_JSON_DICT_LIST testData{{tDir.itemPath("Avengers"), scenes}};
      scnMgr.mockJsonDictForTest(testData);

      // 生成 .scn 文件
      QCOMPARE(scnMgr.WriteDictIntoScnFiles(), 1);

      // 解析 .scn 文件
      QString scnFilePath = tDir.itemPath("Avengers/Avengers.scn");
      SCENE_INFO_LIST parsedScenes = ParseAScnFile(scnFilePath, "/Avengers/");

      // 验证解析结果
      QCOMPARE(parsedScenes.size(), 3);

      // 验证第一个场景
      QCOMPARE(parsedScenes[0].name, "Iron Man");
      QCOMPARE(parsedScenes[0].imgs, QStringList({"iron1.jpg"}));
      QCOMPARE(parsedScenes[0].vidSize, 1024 * 1024 * 700);
      QCOMPARE(parsedScenes[0].rate, 92);

      // 验证第二个场景
      QCOMPARE(parsedScenes[1].name, "Thor");
      QCOMPARE(parsedScenes[1].imgs, QStringList({"thor1.jpg", "thor2.jpg", "thor3.jpg"}));
      QCOMPARE(parsedScenes[1].vidSize, 1024 * 1024 * 800);
      QCOMPARE(parsedScenes[1].rate, 88);

      // 验证第三个场景（无图片）
      QCOMPARE(parsedScenes[2].name, "Hulk");
      QVERIFY(parsedScenes[2].imgs.isEmpty());
      QCOMPARE(parsedScenes[2].vidSize, 1024 * 1024 * 900);
      QCOMPARE(parsedScenes[2].rate, 0);

      tDir.ClearAll();
    }

    {  // 5. 文件格式错误测试
      // 创建格式错误的 .scn 文件, 写入不完整的数据（缺少某些行）,只有名称，缺少其他字段
      tDir.touch("Corrupted/Corrupted.scn", "Scene Name\n");
      // 应该返回空列表或部分数据（根据 ParseAScnFile 的错误处理）
      SCENE_INFO_LIST result = ParseAScnFile(tDir.path(), "/Corrupted/");
      // 根据当前实现，遇到错误会返回空列表
      QVERIFY(result.isEmpty());
      tDir.ClearAll();
    }
  }

  void GetScnsLstFromPath_correct() {
    QVariantHash scene1;
    scene1["Name"] = "The Last Fight";
    scene1["ImgName"] = QStringList{"The Last Fight 1.jpg", "The Last Fight 2.jpg"};
    scene1["VidName"] = "The Last Fight.mp4";
    scene1["Size"] = 1024 * 1024 * 500;
    scene1["Rate"] = 10;  // 10 in 10
    scene1["Uploaded"] = "20221212 13:00:00";
    QList<QVariantHash> scenesUnderRoot{scene1};

    QVariantHash scene2;
    scene2["Name"] = "Iron Man";
    scene2["ImgName"] = QStringList{"iron1.jpg"};
    scene2["VidName"] = "iron_man.mp4";
    scene2["Size"] = 1024 * 1024 * 700;
    scene2["Rate"] = 9;  // 9 in 10
    scene2["Uploaded"] = "20241212 13:00:00";
    QList<QVariantHash> scenesUnderAvengers{scene2};

    const QString scene1BaseName = tDir.baseName();
    QVERIFY(tDir.mkpath("Avengers"));
    ScnMgr scnMgr;
    ScnMgr::PATH_JSON_DICT_LIST testData{
        {tDir.path(), {scenesUnderRoot}},
        {tDir.itemPath("Avengers"), {scenesUnderAvengers}},
    };
    scnMgr.mockJsonDictForTest(testData);
    QCOMPARE(scnMgr.WriteDictIntoScnFiles(), 2);  // 2 scn files
    const QString expect1stScnFileAbsPath = tDir.itemPath(scene1BaseName + ".scn");
    const QString expect2ndScnFileAbsPath = tDir.itemPath("Avengers/Avengers.scn");
    QVERIFY(QFile::exists(expect1stScnFileAbsPath));
    QVERIFY(QFile::exists(expect2ndScnFileAbsPath));

    SCENE_INFO_LIST scenesFromScn = GetScnsLstFromPath(tDir.path());
    QCOMPARE(scenesFromScn.size(), 2);

    // start check;
    // when traverse a directory to read scn file. it use sort by relName ascii QDirIterator. so ascii least means read first
    SCENE_INFO parsedLastFightScene = scenesFromScn[0];  // scene1BaseName  "Filexxxx"  "The Last Fight"
    SCENE_INFO parsedIconMen = scenesFromScn[1];         //                 "Avengers"  "Iron Man"
    if (parsedLastFightScene.name != "The Last Fight") {
      std::swap(parsedLastFightScene, parsedIconMen);
    }

    // check key `Name`, `ImgName`, `VidName`, `Size` field value should same as QVariantHash
    QCOMPARE(parsedLastFightScene.name, "The Last Fight");  // The Last Fight
    QCOMPARE(parsedLastFightScene.imgs, scene1["ImgName"].toStringList());
    QCOMPARE(parsedLastFightScene.vidName, scene1["VidName"].toString());
    QCOMPARE(parsedLastFightScene.vidSize, scene1["Size"].toLongLong());
    const QString LastFightSceneScnFileAbsPath = tDir.path() + parsedLastFightScene.rel2scn + (scene1BaseName + ".scn");
    QCOMPARE(LastFightSceneScnFileAbsPath, expect1stScnFileAbsPath);

    QCOMPARE(parsedIconMen.name, "Iron Man");  //
    QCOMPARE(parsedIconMen.imgs, scene2["ImgName"].toStringList());
    QCOMPARE(parsedIconMen.vidName, scene2["VidName"].toString());
    QCOMPARE(parsedIconMen.vidSize, scene2["Size"].toLongLong());
    const QString IconMenScnFileAbsPath = tDir.path() + parsedIconMen.rel2scn + "Avengers.scn";
    QCOMPARE(IconMenScnFileAbsPath, expect2ndScnFileAbsPath);
  }
};

#include "SceneInfoManagerTest.moc"
REGISTER_TEST(SceneInfoManagerTest, false)
