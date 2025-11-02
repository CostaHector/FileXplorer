#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "SceneInfoManager.h"
#include "EndToExposePrivateMember.h"

#include "JsonHelper.h"
#include "TDir.h"
#include "JsonKey.h"
#include "JsonTestPrecoditionTools.h"

using namespace SceneHelper;
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
      QCOMPARE(scnMgr.UpdateJsonUnderAPath(tDir.path()), Counter(1, 1, 0, 1));
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
      QCOMPARE(scnMgr.UpdateJsonUnderAPath(tDir.path()), Counter(1, 1, 1, 0));
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
      QCOMPARE(scnMgr.UpdateScnFiles(tDir.path()), 0);
      QCOMPARE(tDir.Snapshot(), (QSet<QString>{}));
    }

    {  // 2.0 构造一个路径, 并设置m_jsonsDicts非空, .scn写入路径成功, 名称就是${父文件夹名称.scn}, 清理m_jsonsDicts
      QVERIFY(tDir.mkpath("SuperHero"));

      QVERIFY(JsonHelper::DumpJsonDict(ConstructJsonDict("Chris Evans"), tDir.itemPath("SuperHero/Chris Evans.json")));
      QVERIFY(JsonHelper::DumpJsonDict(ConstructJsonDict("Henry Cavill"), tDir.itemPath("SuperHero/Henry Cavill.json")));

      QCOMPARE(ScnMgr::UpdateScnFiles(tDir.itemPath("SuperHero")), 1);
      QCOMPARE(tDir.Snapshot(QDir::Filter::Files), (QSet<QString>{
                                                       "SuperHero/SuperHero.scn",      //
                                                       "SuperHero/Chris Evans.json",   //
                                                       "SuperHero/Henry Cavill.json",  //
                                                   }));
      tDir.ClearAll();
    }

    { /* 3.0 构造两个路径, 并设置m_jsonsDicts两个key,
SuperHero和XMen, SuperHero下有两个非空dict;  XMen下有一个空dict, 一个非空
预期两个.scn写入路径成功, 名称就是SuperHero.scn, XMen.scn, 简单检查内容
清理m_jsonsDicts*/
      QVERIFY(tDir.mkpath("SuperHero"));
      QVERIFY(tDir.mkpath("XMen"));

      // 准备数据
      QVERIFY(JsonHelper::DumpJsonDict(ConstructJsonDict("Chris Evans"), tDir.itemPath("SuperHero/Chris Evans.json")));
      QVERIFY(JsonHelper::DumpJsonDict(ConstructJsonDict("Henry Cavill"), tDir.itemPath("SuperHero/Henry Cavill.json")));

      QVERIFY(JsonHelper::DumpJsonDict({}, tDir.itemPath("XMen/EmptyJson.json")));
      QVERIFY(JsonHelper::DumpJsonDict(ConstructJsonDict("Wolverine"), tDir.itemPath("XMen/Wolverine.json")));

      // 执行写入操作
      QCOMPARE(ScnMgr::UpdateScnFiles(tDir.path()), 2);  // 预期生成两个.scn文件

      // 验证文件系统
      QCOMPARE(tDir.Snapshot(QDir::Filter::Files), (QSet<QString>{
                                                       "SuperHero/SuperHero.scn",      //
                                                       "SuperHero/Chris Evans.json",   //
                                                       "SuperHero/Henry Cavill.json",  //
                                                       "XMen/XMen.scn",
                                                       "XMen/EmptyJson.json",
                                                       "XMen/Wolverine.json",
                                                   }));
      tDir.ClearAll();
    }

    {
      // 4. 边界情况测试：空场景列表
      QVERIFY(tDir.touch("Empty/EmptyJson.json", "{}"));  // empty json
      // 应该不生成 .scn 文件（因为场景列表完全为空）
      QCOMPARE(ScnMgr::UpdateScnFiles(tDir.path()), 0);
      // 验证没有生成 .scn 文件
      QVERIFY(!tDir.exists("Empty/Empty.scn"));
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

      QCOMPARE(result, Counter(3, 3, 3, 2));  // 预期：更新3个JSON，使用3个JSON，更新3个视频字段, 更新2个图片字段

      // 验证文件更新
      // 根目录：Disney.json
      // Marvel目录：Captain American.json
      // Marvel/Bonus目录：Chris Evans.json, 验证图片字段未更新（Chris Evans没有图片）
      QVERIFY(tDir.checkFileContents("Disney.json", {"Disney", "Disney.jpg", "Disney.mp4"}));
      QVERIFY(tDir.checkFileContents("Marvel/Captain American.json", {"Captain American", "Captain American.jpg", "Captain American.mp4"}));
      QVERIFY(tDir.checkFileContents("Marvel/Bonus/Chris Evans.json", {"Chris Evans", "Chris Evans.mp4"}, {"Chris Evans.jpg"}));
      const QStringList& beforeList = tDir.FilesContentsSnapshot({"Disney.json", "Marvel/Captain American.json", "Marvel/Bonus/Chris Evans.json"});
      // 3 json files in used in total QCOMPARE(scnMgr.UpdateScnFiles(), 3);
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

      // 创建目录
      QVERIFY(tDir.mkpath("Marvel"));
      QVERIFY(JsonHelper::DumpJsonDict(scene1, tDir.itemPath("Marvel/Captain America.json")));

      // 生成 .scn 文件
      QCOMPARE(ScnMgr::UpdateScnFiles(tDir.itemPath("Marvel")), 1);

      // 验证 .scn 文件存在
      QVERIFY(tDir.exists("Marvel/Marvel.scn"));

      // 解析 .scn 文件
      SceneInfoList parsedScenes = ParseAScnFile(tDir.itemPath("Marvel/Marvel.scn"),
                                                 "/Marvel/");  //  here mRootPath=tDir.path(); tDir.path() + "/Marvel/" + Marvel.scn=the json scn

      // 验证解析结果
      QCOMPARE(parsedScenes.size(), 1);

      const SceneInfo& parsedScene = parsedScenes.first();
      QCOMPARE(parsedScene.rel2scn, "/Marvel/");
      QCOMPARE(parsedScene.name, "Captain America");
      QCOMPARE(parsedScene.imgs, QStringList({"cap1.jpg", "cap2.jpg"}));
      QCOMPARE(parsedScene.vidName, "captain_america.mp4");
      QCOMPARE(parsedScene.vidSize, 1024 * 1024 * 500);
      QCOMPARE(parsedScene.rate, 85);
      QCOMPARE(parsedScene.uploaded, "20241212 12:50:50");

      {  // bounder test
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

      // 创建目录
      QVERIFY(tDir.mkpath("Avengers"));
      QVERIFY(JsonHelper::DumpJsonDict(scene1, tDir.itemPath("Avengers/Iron Man.json")));
      QVERIFY(JsonHelper::DumpJsonDict(scene2, tDir.itemPath("Avengers/Thor.json")));
      QVERIFY(JsonHelper::DumpJsonDict(scene3, tDir.itemPath("Avengers/Hulk.json")));

      // 生成 .scn 文件
      QCOMPARE(ScnMgr::UpdateScnFiles(tDir.path()), 1);

      // 解析 .scn 文件, 并按照Rate Descending
      QString scnFilePath = tDir.itemPath("Avengers/Avengers.scn");
      SceneInfoList parsedScenes = ParseAScnFile(scnFilePath, "/Avengers/");
      std::sort(parsedScenes.begin(), parsedScenes.end(),                 //
                [](const SceneInfo& lhs, const SceneInfo& rhs) -> bool {  //
                  return !lhs.lessThanRate(rhs);
                });

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
      QVERIFY(tDir.touch("Corrupted/Corrupted.scn", "Scene Name\n"));
      // 应该返回空列表或部分数据（根据 ParseAScnFile 的错误处理）
      SceneInfoList result = ParseAScnFile(tDir.path(), "/Corrupted/");
      // 根据当前实现，遇到错误会返回空列表
      QVERIFY(result.isEmpty());
      tDir.ClearAll();
    }
  }

  void GetScnsLstFromPath_correct() {
    QVERIFY(tDir.mkpath("Avengers"));

    QString scenesUnderRootPath = tDir.path();
    QString scenesUnderSubPath = tDir.itemPath("Avengers");

    const QString rootScnAbsFilePath{ScnMgr::GetScnAbsFilePath(scenesUnderRootPath)};
    const QString avengersScnAbsFilePath{ScnMgr::GetScnAbsFilePath(scenesUnderSubPath)};

    QString name0 = "The Last Fight";
    QVariantHash scene0;
    scene0["Name"] = name0;
    scene0["ImgName"] = QStringList{name0 + " 1.jpg", name0 + " 2.jpg"};
    scene0["VidName"] = name0 + ".mp4";
    scene0["Size"] = 1024 * 1024 * 500;
    scene0["Rate"] = 10;  // 10 in 10
    scene0["Uploaded"] = "20221212 13:00:00";
    QVERIFY(JsonHelper::DumpJsonDict(scene0, tDir.itemPath(name0 + ".json")));

    QString name1 = "Iron Man";
    QVariantHash scene1;
    scene1["Name"] = name1;
    scene1["ImgName"] = QStringList{name1 + ".jpg"};
    scene1["VidName"] = name1 + ".mp4";
    scene1["Size"] = 1024 * 1024 * 700;
    scene1["Rate"] = 9;  // 9 in 10
    scene1["Uploaded"] = "20241212 13:00:00";
    QVERIFY(JsonHelper::DumpJsonDict(scene1, tDir.itemPath("Avengers/" + name1 + ".json")));

    QCOMPARE(ScnMgr::UpdateScnFiles(tDir.path()), 2);  // 2 scn files
    QVERIFY(QFile::exists(rootScnAbsFilePath));
    QVERIFY(QFile::exists(avengersScnAbsFilePath));

    SceneInfoList scenesFromScn = SceneHelper::GetScnsLstFromPath(tDir.path());
    QCOMPARE(scenesFromScn.size(), 2);

    QCOMPARE(scenesFromScn[0].rel2scn, "/");           // be the first
    QCOMPARE(scenesFromScn[1].rel2scn, "/Avengers/");  // be the second

    QCOMPARE(scenesFromScn[0].name, "The Last Fight");
    QCOMPARE(scenesFromScn[1].name, "Iron Man");

    // start check;
    // when traverse a directory to read scn file. it use sort by rel2scn. so ascii least means read first

    // check key `Name`, `ImgName`, `VidName`, `Size` field value should same as QVariantHash
    QCOMPARE(scenesFromScn[0].name, "The Last Fight");  // The Last Fight
    QCOMPARE(scenesFromScn[0].imgs, scene0["ImgName"].toStringList());
    QCOMPARE(scenesFromScn[0].vidName, scene0["VidName"].toString());
    QCOMPARE(scenesFromScn[0].vidSize, scene0["Size"].toLongLong());
    const QString LastFightSceneScnFileAbsPath = tDir.path() + scenesFromScn[0].rel2scn + (tDir.baseName() + ".scn");
    QCOMPARE(LastFightSceneScnFileAbsPath, rootScnAbsFilePath);

    QCOMPARE(scenesFromScn[1].name, "Iron Man");  //
    QCOMPARE(scenesFromScn[1].imgs, scene1["ImgName"].toStringList());
    QCOMPARE(scenesFromScn[1].vidName, scene1["VidName"].toString());
    QCOMPARE(scenesFromScn[1].vidSize, scene1["Size"].toLongLong());
    const QString IconMenScnFileAbsPath = tDir.path() + scenesFromScn[1].rel2scn + "Avengers.scn";
    QCOMPARE(IconMenScnFileAbsPath, avengersScnAbsFilePath);
  }
};

#include "SceneInfoManagerTest.moc"
REGISTER_TEST(SceneInfoManagerTest, false)
