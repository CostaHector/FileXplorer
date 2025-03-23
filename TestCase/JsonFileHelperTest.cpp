#include <QtTest>
#include <QCoreApplication>
#include "Tools/JsonFileHelper.h"
#include "pub/MyTestSuite.h"

class JsonFileHelperTest : public MyTestSuite {
  Q_OBJECT

 public:
 private slots:
  void initTestCase() { qDebug("start to test JsonFileHelperTest"); }
  void cleanupTestCase() { qDebug("finish to test JsonFileHelperTest"); }

  void test_GetMovieFileJsonDict() {
    auto dict = JsonFileHelper::GetMovieFileJsonDict("C:/A/B.mp4", "A,B, and C", "Henri Cavill");
    foreach (const QString& key, JSONKey::JsonKeyListOrder) {
      QVERIFY(dict.contains(key));
    }
    const QStringList expectPerfs{"A", "B", "C"};
    QCOMPARE(dict[JSONKey::Name].toString(), "B");
    QCOMPARE(dict[JSONKey::Performers].toStringList(), expectPerfs);
    QCOMPARE(dict[JSONKey::Studio].toString(), "Henri Cavill");
    QVERIFY(dict[JSONKey::Tags].toStringList().isEmpty());
    QCOMPARE(dict[JSONKey::Rate].toInt(), -1);
    QCOMPARE(dict[JSONKey::Size].toString(), "0");
    QVERIFY(dict[JSONKey::Hot].toList().isEmpty());
    QVERIFY(dict[JSONKey::Detail].toString().isEmpty());
  }

  void test_GetDefaultJsonFile() {
    auto dict = JsonFileHelper::GetDefaultJsonFile();
    foreach (const QString& key, JSONKey::JsonKeyListOrder) {
      QVERIFY(dict.contains(key));
    }
    QVERIFY(dict[JSONKey::Name].toString().isEmpty());
    QVERIFY(dict[JSONKey::Performers].toStringList().isEmpty());
    QVERIFY(dict[JSONKey::Studio].toString().isEmpty());
    QVERIFY(dict[JSONKey::Tags].toStringList().isEmpty());
    QCOMPARE(dict[JSONKey::Rate].toInt(), -1);
    QCOMPARE(dict[JSONKey::Size].toString(), "0");
    QVERIFY(dict[JSONKey::Hot].toList().isEmpty());
    QVERIFY(dict[JSONKey::Detail].toString().isEmpty());
  }

  void test_JsonStr2Dict() {
    const char jsonStrArray[] = R"({"Henry Cavill": ["tall",  "muscle"], "Chris Evans": 192})";
    const QVariantHash dict = JsonFileHelper::JsonStr2Dict(jsonStrArray);
    const QStringList expectSl{"tall", "muscle"};
    QVERIFY(dict.contains("Henry Cavill"));
    QCOMPARE(dict["Henry Cavill"].toStringList(), expectSl);
    QVERIFY(dict.contains("Chris Evans"));
    QCOMPARE(dict["Chris Evans"].toInt(), 192);
  }

  void test_jsonStr2DictCompatible() {
    const char jsonStrArray[] = R"({"ProductionStudio": ["Chris Hemsworth Studio", "Keanu Reeves Studio"]})";
    const QStringList expectSl{"Chris Hemsworth Studio", "Keanu Reeves Studio"};
    const QVariantHash dict = JsonFileHelper::JsonStr2Dict(jsonStrArray);
    QVERIFY(dict.contains(JSONKey::Studio));
    QCOMPARE(dict[JSONKey::Studio].toStringList(), expectSl);
  }

  void test_GetJsonFilePath() {
    QCOMPARE(JsonFileHelper::GetJsonFilePath("Hugh Jackman.jpg"), "Hugh Jackman.json");
    QCOMPARE(JsonFileHelper::GetJsonFilePath("Kalman.m"), "Kalman.json");
    QCOMPARE(JsonFileHelper::GetJsonFilePath("John  Reese Sc.1"), "John  Reese Sc.1.json");
    QCOMPARE(JsonFileHelper::GetJsonFilePath("/home/to/file.txt"), "/home/to/file.json");
  }
};

#include "JsonFileHelperTest.moc"
JsonFileHelperTest gJsonFileHelperTest;
