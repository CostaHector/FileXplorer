#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"

#include "Tools/SceneInfoManager.h"
#include "Tools/Json/JsonHelper.h"
using namespace SceneInfoManager;

class SceneInfoManagerTest : public MyTestSuite {
  Q_OBJECT

 public:
  SceneInfoManagerTest() : MyTestSuite{false} {}
 private slots:
  void test_GetSceneFileContents_invalid() {
    QStringList names{"adsafsd.json"};
    QString contents;
    int jsonCnt = 0;
    std::tie(contents, jsonCnt) = GetScnFileContents(names, {});
    QVERIFY(contents.isEmpty());
    QCOMPARE(jsonCnt, -1);
  }

  void test_GetSceneFileContents_empty() {
    QStringList names;
    QList<QVariantHash> jsonDicts;
    QString contents;
    int jsonCnt = 0;
    std::tie(contents, jsonCnt) = GetScnFileContents(names, jsonDicts);
    QVERIFY(contents.isEmpty());
    QCOMPARE(jsonCnt, 0);
  }

  void test_GetSceneFileContents() {
    const char aJsonStr[] = R"({
    "Bitrate": "N/A",
    "Detail": "detail",
    "ImgName": "a.jpg",
    "Name": "a",
    "Cast": [
    ],
    "ProductionStudio": "MarvelFilms",
    "Rate": 0,
    "Resolution": "1080p",
    "Size": "MiB",
    "Tags": [
        "creampie",
        "porn"
    ],
    "Uploaded": "14:45 12-Oct-2024",
    "VidName": "a.mp4",
    "Size": 907613690
})";
    const char movieJsonStr[] = R"({
    "Bitrate": "N/A",
    "Detail": "detail",
    "ImgName": "Movie - performer 1, performer 2.jpg",
    "Name": "Movie - performer 1, performer 2",
    "Cast": [
    ],
    "ProductionStudio": "",
    "Rate": 0,
    "Resolution": "1080p",
    "Size": "MiB",
    "Tags": [
        "creampie",
        "porn"
    ],
    "Uploaded": "14:45 12-Oct-2024",
    "VidName": "Movie - performer 1, performer 2.mp4",
    "Size": 1024
})";

    QStringList names;
    names << "a.json"
          << "Movie - performer 1, performer 2.json";

    QList<QVariantHash> jsonDicts;
    jsonDicts << JsonHelper::DeserializedJsonStr2Dict(aJsonStr)       //
              << JsonHelper::DeserializedJsonStr2Dict(movieJsonStr);  //

    QString contents;
    int jsonCnt = 0;
    std::tie(contents, jsonCnt) = GetScnFileContents(names, jsonDicts);
    QVERIFY(!contents.isEmpty());
    QCOMPARE(jsonCnt, 2);
    QVERIFY((contents.count('\n') & 1) == 0);  // \n count even
    QVERIFY(contents.contains("a\n"));
    QVERIFY(contents.contains("Movie - performer 1, performer 2\n"));
  }
};

SceneInfoManagerTest g_SceneInfoManagerTest;
#include "SceneInfoManagerTest.moc"
