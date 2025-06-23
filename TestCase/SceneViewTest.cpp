#include <QCoreApplication>
#include <QtTest>

#include "public/PathTool.h"
#include "Tools/SceneInfoManager.h"
#include "TestCase/pub/MyTestSuite.h"
#include "TestCase/pub/TDir.h"

using namespace SceneInfoManager;

class SceneViewTest : public MyTestSuite {
  Q_OBJECT
 public:
  SceneViewTest() : MyTestSuite{false} {}
  QList<FsNodeEntry> gEntryNodes;
 private slots:
  void initTestCase() {
    // somejsonfile1.json
    // somejsonfile2.json
    // somejsonfileEmpty.json
    // somejsonfileEmpty.png
    // somejsonfileEmpty.mp4
    gEntryNodes.append({"somejsonfile1.json", false, R"({
    "Name": "somejsonfile1",
    "Performers": [],
    "ProductionStudio": "[Creedo]",
    "Tags": [
        "comics",
        "nonporn"
    ],
    "Size": "59",
    "Resolution": "N/A",
    "Bitrate": "N/A",
    "Detail": "",
  "ImgName":"somejsonfile1.png",
  "vidName":"somejsonfile1.mp4",
  "VidSize":1024,
    "Rate": 99,
    "Uploaded": "06:40 23-Sep-2024"
})"});
    gEntryNodes.append({"somejsonfile2.json", false, R"({
    "Name": "somejsonfile2",
    "Performers": [],
    "ProductionStudio": "[Creedo]",
    "Tags": [
        "comics",
        "nonporn"
    ],
    "Size": "59",
    "Resolution": "N/A",
    "Bitrate": "N/A",
    "Detail": "",
  "ImgName":"somejsonfile2.png",
  "vidName":"somejsonfile2.mp4",
  "VidSize":10240,
    "Rate": 50,
    "Uploaded": "06:40 23-Sep-2024"
})"});
    gEntryNodes.append({"somejsonfileEmpty.json", false, R"({
    "Name": "somejsonfileEmpty",
    "Performers": [],
    "ProductionStudio": "[Creedo]",
    "Tags": [
        "comics",
        "nonporn"
    ],
    "Size": "59",
    "Resolution": "N/A",
    "Bitrate": "N/A",
    "Detail": ""
})"});
    gEntryNodes.append({"somejsonfileEmpty.mp4", false, R"(AAAAAAAAA)"});
    gEntryNodes.append({"somejsonfileEmpty.png", false, R"(AAAAA)"});
  }

  void test_nonExistfolder_ok() {
    QVERIFY(!QFile::exists("any/not/exist/path"));
    const auto& scenes = SceneInfoManager::GetScenesFromPath("any/not/exist/path");
    QVERIFY(scenes.isEmpty());
  }

  void test_3jsonsFolder_sort_ok() {
    TDir dir;
    QCOMPARE(dir.createEntries(gEntryNodes), gEntryNodes.size());
    QString TEST_DIR{dir.path()};

    QCOMPARE(SceneInfoManager::GenerateScnFilesDirectly(TEST_DIR), 1);

    SCENES_TYPE scenes = SceneInfoManager::GetScenesFromPath(TEST_DIR);
    QCOMPARE(scenes.size(), 3);

    QCOMPARE(scenes[0].name, "somejsonfile1");
    QCOMPARE(scenes[1].name, "somejsonfile2");
    QCOMPARE(scenes[2].name, "somejsonfileEmpty");
    SceneInfoManager::sort(scenes, SceneSortOption::NAME, true);
    QCOMPARE(scenes[0].name, "somejsonfileEmpty");
    QCOMPARE(scenes[1].name, "somejsonfile2");
    QCOMPARE(scenes[2].name, "somejsonfile1");
    SceneInfoManager::sort(scenes, SceneSortOption::NAME, false);
    QCOMPARE(scenes[0].name, "somejsonfile1");
    QCOMPARE(scenes[1].name, "somejsonfile2");
    QCOMPARE(scenes[2].name, "somejsonfileEmpty");

    SceneInfoManager::sort(scenes, SceneSortOption::RATE, true);
    QCOMPARE(scenes[0].rate, 99);
    QCOMPARE(scenes[1].rate, 50);
    QCOMPARE(scenes[2].rate, 0);
    SceneInfoManager::sort(scenes, SceneSortOption::RATE, false);
    QCOMPARE(scenes[0].rate, 0);
    QCOMPARE(scenes[1].rate, 50);
    QCOMPARE(scenes[2].rate, 99);
  }

  void test_update_inExist_path() {
    QVERIFY(!QDir("Any/Inexist/Path").exists());
    JsonDataRefresher jdr;
    QCOMPARE(jdr("Any/Inexist/Path"), -1);
  }

  void test_UpdateJsonVidImgRateUploaded() {
    // modification on json file
    // somejsonfile1.json: update img, vid to empty
    // somejsonfile2.json: update img, vid to empty
    // somejsonfileEmpty.json: insert img, vid key value pair
    TDir dir;
    QCOMPARE(dir.createEntries(gEntryNodes), gEntryNodes.size());
    QString TEST_DIR{dir.path()};

    JsonDataRefresher jdr;
    QCOMPARE(jdr(TEST_DIR), 3);

    QVERIFY(!dir.fileExists("COPY_REMOVABLE.scn"));
    QCOMPARE(jdr.GenerateScnFiles(), 1);

    const SCENES_TYPE& afterScenes = SceneInfoManager::GetScenesFromPath(TEST_DIR);
    QCOMPARE(afterScenes[0].name, "somejsonfile1");
    QCOMPARE(afterScenes[0].imgs, (QStringList{}));
    QCOMPARE(afterScenes[0].vidName, "");

    QCOMPARE(afterScenes[1].name, "somejsonfile2");
    QCOMPARE(afterScenes[1].imgs, (QStringList{}));
    QCOMPARE(afterScenes[1].vidName, "");

    QCOMPARE(afterScenes[2].name, "somejsonfileEmpty");
    QCOMPARE(afterScenes[2].imgs, QStringList{"somejsonfileEmpty.png"});
    QCOMPARE(afterScenes[2].vidName, "somejsonfileEmpty.mp4");
    QVERIFY(afterScenes[2].vidSize > 0);
    QCOMPARE(afterScenes[2].rate, 0);
    QCOMPARE(afterScenes[2].uploaded.size(), QString("yyyyMMdd hh:mm:ss").size());
  }

  void test_WriteScenesIntoScnFile() {
    TDir dir;
    QCOMPARE(dir.createEntries(gEntryNodes), gEntryNodes.size());
    QString TEST_DIR{dir.path()};

    const QString scnFileName = PathTool::fileName(TEST_DIR) + ".scn";
    QVERIFY2(!dir.fileExists(scnFileName, false), "*.scn file should not exists yet");
    QCOMPARE(SceneInfoManager::GenerateScnFilesDirectly(TEST_DIR), 1);

    QVERIFY2(dir.fileExists(scnFileName, false), "*.scn file should exists");
    QCOMPARE(SceneInfoManager::GenerateScnFilesDirectly(TEST_DIR), 1);
  }
};

#include "SceneViewTest.moc"
SceneViewTest g_SceneViewTest;
