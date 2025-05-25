#include <QCoreApplication>
#include <QtTest>

// #include "pub/BeginToExposePrivateMember.h"
#include "public/PathTool.h"
#include "Tools/SceneInfoManager.h"
// #include "pub/EndToExposePrivateMember.h"
#include "public/PublicVariable.h"
#include "public/PublicTool.h"

#include <QDir>
#include <QFileInfo>
#include <QDirIterator>

const QString SCENE_VIEW_PATH{QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_SceneViewTest/DONT_CHANGE")};
const QString TEST_DIR{QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_SceneViewTest/COPY_REMOVABLE")};

using namespace SceneInfoManager;

class SceneViewTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {
    // somejsonfile1.json
    // somejsonfile2.json
    // somejsonfileEmpty.json
    // somejsonfileEmpty.png
    // somejsonfileEmpty.mp4
    QFileInfo fi{SCENE_VIEW_PATH};
    QVERIFY(fi.exists());
    QVERIFY(fi.isDir());
    const QStringList jsons = QDir(SCENE_VIEW_PATH).entryList(TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDir::SortFlag::Name);
    QCOMPARE(jsons.size(), 3);
    QCOMPARE(jsons[0], "somejsonfile1.json");
    QCOMPARE(jsons[1], "somejsonfile2.json");
    QCOMPARE(jsons[2], "somejsonfileEmpty.json");
  }
  void cleanupTestCase() {}

  void init() {
    if (QDir(TEST_DIR).exists()) {
      QDir(TEST_DIR).removeRecursively();
    }
    auto ret = PathTool::copyDirectoryFiles(SCENE_VIEW_PATH, TEST_DIR);
    assert(ret);  // should copied ok
  }
  void cleanup() {
    if (QDir(TEST_DIR).exists()) {
      QDir(TEST_DIR).removeRecursively();
    }
  }

  void test_nonExistfolder_ok() {
    auto scenes = SceneInfoManager::GetScenesFromPath("any/not/exist/path");
    QVERIFY(scenes.isEmpty());
  }

  void test_3jsonsFolder_ok() {
    QCOMPARE(SceneInfoManager::GenerateScnFilesDirectly(TEST_DIR), 1);

    auto scenes = SceneInfoManager::GetScenesFromPath(TEST_DIR);
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
  void test_UpdateInExistPath() {
    QVERIFY(!QFileInfo("Any/Inexist/Path").isDir());
    JsonDataRefresher jdr;
    QCOMPARE(jdr("Any/Inexist/Path"), -1);
  }

  void test_UpdateJsonVidImgRateUploaded() {
    // modification on json file
    // somejsonfile1.json: update img, vid to empty
    // somejsonfile2.json: update img, vid to empty
    // somejsonfileEmpty.json: insert img, vid key value pair
    JsonDataRefresher jdr;
    QCOMPARE(jdr(TEST_DIR), 3);

    QVERIFY(!QDir(TEST_DIR).exists("COPY_REMOVABLE.scn"));
    QCOMPARE(jdr.GenerateScnFiles(), 1);

    auto afterScenes = SceneInfoManager::GetScenesFromPath(TEST_DIR);
    QCOMPARE(afterScenes[0].name, "somejsonfile1");
    QVERIFY(afterScenes[0].imgs.isEmpty());
    QCOMPARE(afterScenes[0].vidName, "");

    QCOMPARE(afterScenes[1].name, "somejsonfile2");
    QVERIFY(afterScenes[1].imgs.isEmpty());
    QCOMPARE(afterScenes[1].vidName, "");

    QCOMPARE(afterScenes[2].name, "somejsonfileEmpty");
    QCOMPARE(afterScenes[2].imgs, QStringList{"somejsonfileEmpty.png"});
    QCOMPARE(afterScenes[2].vidName, "somejsonfileEmpty.mp4");
    QVERIFY(afterScenes[2].vidSize > 0);
    QCOMPARE(afterScenes[2].rate, 0);
    QCOMPARE(afterScenes[2].uploaded.size(), QString("yyyyMMdd hh:mm:ss").size());
  }

  void test_WriteScenesIntoScnFile() {
    const QString scnFileName = PathTool::fileName(TEST_DIR) + ".scn";
    QVERIFY2(!QDir(TEST_DIR).exists(scnFileName), "*.scn file should not exists yet");
    QCOMPARE(SceneInfoManager::GenerateScnFilesDirectly(TEST_DIR), 1);
    QVERIFY2(QDir(TEST_DIR).exists(scnFileName), "*.scn file should exists");
    QCOMPARE(SceneInfoManager::GenerateScnFilesDirectly(TEST_DIR), 1);
  }
};

//QTEST_MAIN(SceneViewTest)
#include "SceneViewTest.moc"
