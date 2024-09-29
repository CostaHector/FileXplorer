#include <QCoreApplication>
#include <QtTest>

// #include "pub/BeginToExposePrivateMember.h"
#include "Tools/SceneInfoManager.h"
// #include "pub/EndToExposePrivateMember.h"
#include "PublicVariable.h"
#include "PublicTool.h"

#include <QDir>
#include <QFileInfo>
#include <QDirIterator>

const QString SCENE_VIEW_PATH{QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_SceneViewTest/DONT_CHANGE")};
const QString TEST_DIR{QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_SceneViewTest/COPY_REMOVABLE")};

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
    auto ret = PublicTool::copyDirectoryFiles(SCENE_VIEW_PATH, TEST_DIR);
    assert(ret);  // should copied ok
  }
  void cleanup() {
    if (QDir(TEST_DIR).exists()) {
      QDir(TEST_DIR).removeRecursively();
    }
  }

  void test_nonExistfolder_ok() {
    SceneInfoManager sim;
    auto scenes = sim.GetScenesFromPath("any/not/exist/path");
    QVERIFY(scenes.isEmpty());
  }

  void test_3jsonsFolder_ok() {
    SceneInfoManager sim;
    auto scenes = sim.GetScenesFromPath(TEST_DIR);
    QCOMPARE(scenes.size(), 3);

    sim.swap(scenes);
    QVERIFY(scenes.isEmpty());

    QCOMPARE(sim[0].name, "somejsonfile1");
    QCOMPARE(sim[1].name, "somejsonfile2");
    QCOMPARE(sim[2].name, "somejsonfileEmpty");
    sim.sort(SceneInfoManager::NAME, true);
    QCOMPARE(sim[0].name, "somejsonfileEmpty");
    QCOMPARE(sim[1].name, "somejsonfile2");
    QCOMPARE(sim[2].name, "somejsonfile1");
    sim.sort(SceneInfoManager::NAME, false);
    QCOMPARE(sim[0].name, "somejsonfile1");
    QCOMPARE(sim[1].name, "somejsonfile2");
    QCOMPARE(sim[2].name, "somejsonfileEmpty");

    sim.sort(SceneInfoManager::RATE, true);
    QCOMPARE(sim[0].rate, 99);
    QCOMPARE(sim[1].rate, 50);
    QCOMPARE(sim[2].rate, 0);
    sim.sort(SceneInfoManager::RATE, false);
    QCOMPARE(sim[0].rate, 0);
    QCOMPARE(sim[1].rate, 50);
    QCOMPARE(sim[2].rate, 99);
  }
  void test_UpdateInExistPath() { QCOMPARE(SceneInfoManager::UpdateJsonImgVidSize("Any/Inexist/Path"), -1); }

  void test_UpdateJsonVidImgRateUploaded() {
    SceneInfoManager sim;
    auto beforeScenes = sim.GetScenesFromPath(TEST_DIR);
    QCOMPARE(beforeScenes[2].name, "somejsonfileEmpty");
    QCOMPARE(beforeScenes[2].imgName, "");
    QCOMPARE(beforeScenes[2].vidName, "");
    QCOMPARE(beforeScenes[2].vidSize, 0);
    QCOMPARE(beforeScenes[2].rate, 0);
    QCOMPARE(beforeScenes[2].uploaded, "");

    QCOMPARE(SceneInfoManager::UpdateJsonImgVidSize(TEST_DIR), 1);
    auto afterScenes = sim.GetScenesFromPath(TEST_DIR);
    QCOMPARE(afterScenes[2].name, "somejsonfileEmpty");
    QCOMPARE(afterScenes[2].imgName, "somejsonfileEmpty.png");
    QCOMPARE(afterScenes[2].vidName, "somejsonfileEmpty.mp4");
    QVERIFY(afterScenes[2].vidSize > 0);
    QCOMPARE(afterScenes[2].rate, 0);
    QCOMPARE(afterScenes[2].uploaded.size(), QString("yyyyMMdd hh:mm:ss").size());

    QCOMPARE(SceneInfoManager::UpdateJsonImgVidSize(TEST_DIR), 0);
  }
};

QTEST_MAIN(SceneViewTest)
#include "SceneViewTest.moc"
