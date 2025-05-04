#include <QtTest>
#include <QCoreApplication>
#include "TestCase/PathRelatedTool.h"
#include "TestCase/pub/OnScopeExit.h"
#include "Tools/JsonFileHelper.h"
#include "pub/MyTestSuite.h"

const QString rootpath = TestCaseRootPath() + "/test/TestEnv_VideosDurationGetter";

using namespace JsonFileHelper;

class JsonFileHelperTest : public MyTestSuite {
  Q_OBJECT

 public:
  JsonFileHelperTest() : MyTestSuite{false} {}
 private slots:
  void test_GetMovieFileJsonDict() {
    auto dict = GetJsonDictByMovieFile("C:/A/B.mp4", "A,B, and C", "Henri Cavill");
    foreach (const QString& key, JSONKey::JsonKeyListOrder) {
      QVERIFY(dict.contains(key));
    }
    const QStringList expectPerfs{"A", "B", "C"};
    QCOMPARE(dict[JSONKey::Name].toString(), "B");
    QCOMPARE(dict[JSONKey::Performers].toStringList(), expectPerfs);
    QCOMPARE(dict[JSONKey::Studio].toString(), "Henri Cavill");
    QVERIFY(dict[JSONKey::Tags].toStringList().isEmpty());
    QCOMPARE(dict[JSONKey::Rate].toInt(), -1);
    QCOMPARE(dict[JSONKey::Size].toString(), "0'0'0'0");
    QVERIFY(dict[JSONKey::Hot].toList().isEmpty());
    QVERIFY(dict[JSONKey::Detail].toString().isEmpty());
  }

  void test_GetDefaultJsonFile() {
    auto dict = GetJsonDictDefault();
    foreach (const QString& key, JSONKey::JsonKeyListOrder) {
      QVERIFY(dict.contains(key));
    }
    QVERIFY(dict[JSONKey::Name].toString().isEmpty());
    QVERIFY(dict[JSONKey::Performers].toStringList().isEmpty());
    QVERIFY(dict[JSONKey::Studio].toString().isEmpty());
    QVERIFY(dict[JSONKey::Tags].toStringList().isEmpty());
    QCOMPARE(dict[JSONKey::Rate].toInt(), -1);
    QCOMPARE(dict[JSONKey::Size].toString(), "0'0'0'0");
    QVERIFY(dict[JSONKey::Hot].toList().isEmpty());
    QVERIFY(dict[JSONKey::Detail].toString().isEmpty());
  }

  void test_JsonStr2Dict() {
    const char jsonStrArray[] = R"({"Henry Cavill": ["tall",  "muscle"], "Chris Evans": 192})";
    const QVariantHash dict = DeserializedJsonStr2Dict(jsonStrArray);
    const QStringList expectSl{"tall", "muscle"};
    QVERIFY(dict.contains("Henry Cavill"));
    QCOMPARE(dict["Henry Cavill"].toStringList(), expectSl);
    QVERIFY(dict.contains("Chris Evans"));
    QCOMPARE(dict["Chris Evans"].toInt(), 192);
  }

  void test_CompatibleJsonKey() {
    const char jsonStrArray[] = R"({"ProductionStudio": ["Chris Hemsworth Studio", "Keanu Reeves Studio"]})";
    const QStringList expectSl{"Chris Hemsworth Studio", "Keanu Reeves Studio"};
    const QVariantHash dict = DeserializedJsonStr2Dict(jsonStrArray);
    QVERIFY(dict.contains(JSONKey::Studio));
    QCOMPARE(dict[JSONKey::Studio].toStringList(), expectSl);
  }

  void test_ClearPerformerAndStudio_ok() {
    const char jsonStrArray[] = R"({"Performers": ["Chris Hemsworth Studio", "Keanu Reeves Studio"], "Name": "Marvel Film"})";
    QVariantHash dictWithPerfAndStudio = DeserializedJsonStr2Dict(jsonStrArray);
    QVERIFY(dictWithPerfAndStudio.contains(JSONKey::Performers));
    QVERIFY(!dictWithPerfAndStudio.contains(JSONKey::Studio));
    QVERIFY(!dictWithPerfAndStudio[JSONKey::Performers].toStringList().isEmpty());
    VariantHashHelper::ClearPerformerAndStudio cps;
    QVERIFY(cps(dictWithPerfAndStudio));  // has cleared
    QVERIFY(dictWithPerfAndStudio.contains(JSONKey::Performers));
    QVERIFY(!dictWithPerfAndStudio.contains(JSONKey::Studio));
    QVERIFY(dictWithPerfAndStudio[JSONKey::Performers].toStringList().isEmpty());
  }

  void test_ClearPerformerAndStudio_skip_keyNotExist() {
    const char jsonStrArray[] = R"({"Name": "Chris Hemsworth life story"})";
    QVariantHash dictWithOnlyName = DeserializedJsonStr2Dict(jsonStrArray);
    QVERIFY(dictWithOnlyName.contains(JSONKey::Name));
    QVERIFY(!dictWithOnlyName.contains(JSONKey::Performers));
    QVERIFY(!dictWithOnlyName.contains(JSONKey::Studio));
    VariantHashHelper::ClearPerformerAndStudio cps;
    QVERIFY(!cps(dictWithOnlyName));  // has cleared
    QVERIFY(dictWithOnlyName.contains(JSONKey::Name));
    QVERIFY(!dictWithOnlyName.contains(JSONKey::Performers));
    QVERIFY(!dictWithOnlyName.contains(JSONKey::Studio));
  }

  void test_InsertPerfsPairToDictByNameHint_skip_perfsPair_alreadyExist() {
    const char jsonStrArray[] = R"({"Performers": ["Chris Hemsworth Studio", "Keanu Reeves Studio"]})";
    const QStringList& expectSl{"Chris Hemsworth Studio", "Keanu Reeves Studio"};
    QVariantHash dictWithPerfs = DeserializedJsonStr2Dict(jsonStrArray);
    VariantHashHelper::InsertPerfsPairToDictByNameHint ipdnh;
    QVERIFY(!ipdnh(dictWithPerfs));
    auto perfIt = dictWithPerfs.find(JSONKey::Performers);
    QVERIFY(perfIt != dictWithPerfs.end());
    QCOMPARE(perfIt.value().toStringList(), expectSl);
  }

  void test_InsertPerfsPairToDictByNameHint_perfsPair_byHint_insertOk() {
    const char jsonStrArray[] = R"({"Name":""})";
    const QStringList expectEmptySl;
    QVariantHash dictWithPerfs = DeserializedJsonStr2Dict(jsonStrArray);
    VariantHashHelper::InsertPerfsPairToDictByNameHint ipdnh;
    QVERIFY(ipdnh(dictWithPerfs));
    auto perfIt = dictWithPerfs.find(JSONKey::Performers);
    QVERIFY(perfIt != dictWithPerfs.end());
    QCOMPARE(perfIt.value().toStringList(), expectEmptySl);
  }

  void test_AppendPerfsToDict_sorted_and_unique_ok() {
    const QString toBeInsertedPerfs{"A, B, and C, D"};
    VariantHashHelper::AppendPerfsToDict apd{toBeInsertedPerfs};
    const char jsonStrArray[] = R"({"Performers":["A", "E"]})";
    QVariantHash dictPerfs = DeserializedJsonStr2Dict(jsonStrArray);
    const QStringList alreadyExistPerfs{"A", "E"};
    QCOMPARE(dictPerfs[JSONKey::Performers], alreadyExistPerfs);
    QVERIFY(apd(dictPerfs));  // B C D inserted ok, A E already exist
    auto perfIt = dictPerfs.find(JSONKey::Performers);
    QVERIFY(perfIt != dictPerfs.end());
    const QStringList afterInsertedPerfs{"A", "B", "C", "D", "E"};
    QCOMPARE(perfIt.value().toStringList(), afterInsertedPerfs);
  }

  void test_AppendPerfsToDict_skip_alreadyExist() {
    const QString toBeInsertedPerfs{"A and E"};
    VariantHashHelper::AppendPerfsToDict apd{toBeInsertedPerfs};
    const char jsonStrArray[] = R"({"Performers":["A", "E"]})";
    QVariantHash dictPerfs = DeserializedJsonStr2Dict(jsonStrArray);
    const QStringList alreadyExistPerfs{"A", "E"};
    QCOMPARE(dictPerfs[JSONKey::Performers], alreadyExistPerfs);
    QVERIFY(!apd(dictPerfs));  // A E already exist, nothing inserted
    auto perfIt = dictPerfs.find(JSONKey::Performers);
    QVERIFY(perfIt != dictPerfs.end());
    QCOMPARE(perfIt.value().toStringList(), alreadyExistPerfs);
  }

  void test_InsertStudioPairIntoDict_skip_studioPair_alreadyExist() {
    VariantHashHelper::InsertStudioPairIntoDict ispd;
    const char jsonStrArray[] = R"({"Studio":"Marvel Films"})";
    QVariantHash dict = DeserializedJsonStr2Dict(jsonStrArray);
    QVERIFY(dict.contains(JSONKey::Studio));
    QCOMPARE(dict[JSONKey::Studio].toString(), "Marvel Films");
    QVERIFY(!ispd(dict));
    auto studioIt = dict.find(JSONKey::Studio);
    QVERIFY(studioIt != dict.end());
    QCOMPARE(studioIt.value().toString(), "Marvel Films");
  }

  void test_InsertStudioPairIntoDict_studioPair_byHint_insertOk() {
    VariantHashHelper::InsertStudioPairIntoDict ispd;
    const char jsonStrArray[] = R"({"Name":""})";
    QVariantHash dict = DeserializedJsonStr2Dict(jsonStrArray);
    QVERIFY(!dict.contains(JSONKey::Studio));
    QVERIFY(ispd(dict));
    auto studioIt = dict.find(JSONKey::Studio);
    QVERIFY(studioIt != dict.end());
    QCOMPARE(studioIt.value().toString(), "");
  }

  void test_UpdateStudio_skip_studio_notChange() {
    VariantHashHelper::UpdateStudio us{"Fox"};
    const char jsonStrArray[] = R"({"Name":"", "Studio": "Fox"})";
    QVariantHash dict = DeserializedJsonStr2Dict(jsonStrArray);
    QVERIFY(dict.contains(JSONKey::Studio));
    QCOMPARE(dict[JSONKey::Studio].toString(), "Fox");
    QVERIFY(!us(dict));
    QVERIFY(dict.contains(JSONKey::Studio));
    QCOMPARE(dict[JSONKey::Studio].toString(), "Fox");
  }

  void test_UpdateStudio_studio_pair_insertOk() {
    VariantHashHelper::UpdateStudio us{"Fox"};
    const char jsonStrArray[] = R"({"Name":""})";
    QVariantHash dict = DeserializedJsonStr2Dict(jsonStrArray);
    QVERIFY(!dict.contains(JSONKey::Studio));
    QVERIFY(us(dict));
    auto studioIt = dict.find(JSONKey::Studio);
    QVERIFY(studioIt != dict.end());
    QCOMPARE(studioIt.value().toString(), "Fox");
  }

  void test_UpdateStudio_studioName_ChangedOk() {
    VariantHashHelper::UpdateStudio us{"Fox"};
    const char jsonStrArray[] = R"({"Name":"", "Studio": "Marvel"})";
    QVariantHash dict = DeserializedJsonStr2Dict(jsonStrArray);
    QVERIFY(dict.contains(JSONKey::Studio));
    QCOMPARE(dict[JSONKey::Studio].toString(), "Marvel");
    QVERIFY(us(dict));
    auto studioIt = dict.find(JSONKey::Studio);
    QVERIFY(studioIt != dict.end());
    QCOMPARE(studioIt.value().toString(), "Fox");
  }

  void test_InsertOrUpdateDurationStudioCastTags() {  //
    const QString pth = QFileInfo(__FILE__).absolutePath();
    QDir dir{pth};

    const QString jsonName{"InsertOrUpdateDurationStudioCastTags.json"};
    const QString& jsonPth = dir.absoluteFilePath(jsonName);
    // precondition, json not exist
    QVERIFY(!dir.exists(jsonName));

    // json created
    auto ret = InsertOrUpdateDurationStudioCastTags(jsonPth, 0, {}, {}, {});
    QVERIFY(dir.exists(jsonName));
    QCOMPARE(ret, CHANGED_OK);
    ON_SCOPE_EXIT {
      if (dir.exists(jsonName)) {
        QVERIFY(dir.remove(jsonName));
      }
    };

    // json unchange
    ret = InsertOrUpdateDurationStudioCastTags(jsonPth, 0, {}, {}, {});
    QCOMPARE(ret, OK);

    // json unchange
    ret = InsertOrUpdateDurationStudioCastTags(jsonPth, 0, "Fox 20 century", {"Henry Cavill"}, {});
    QCOMPARE(ret, CHANGED_OK);
  }

  void test_ReadStudioCastTagsOut() {
    QDir dir{rootpath};
    const QString jsonName{"ReadStudioCastTagsOut.json"};
    const QString& jsonPth = dir.absoluteFilePath(jsonName);
    // precondition, json not exist
    QVERIFY(!dir.exists(jsonName));
    ON_SCOPE_EXIT {
      if (dir.exists(jsonName)) {
        QVERIFY(dir.remove(jsonName));
      }
    };

    // 1. empty value json
    QVariantHash emptyValueDict{{JSONKey::Studio, ""}, {JSONKey::Performers, ""}, {JSONKey::Tags, ""}};
    QVERIFY(DumpJsonDict(emptyValueDict, jsonPth));
    QVERIFY(dir.exists(jsonName));
    QMap<uint, JsonDict2Table> fileNameHash2Json = ReadStudioCastTagsOut(rootpath);
    QVERIFY(fileNameHash2Json.isEmpty());

    // 2. only contains studio and tags, but not performers
    QVariantHash notFullDict{{JSONKey::Studio, "Fox 2000"}, {JSONKey::Tags, "Happiness, Comedy"}};
    QVERIFY(DumpJsonDict(notFullDict, jsonPth));
    QVERIFY(dir.exists(jsonName));
    fileNameHash2Json = ReadStudioCastTagsOut(rootpath);
    QVERIFY(fileNameHash2Json.isEmpty());

    // 3. contains studio and tags and performers
    QVariantHash fullDict{{JSONKey::Studio, "Fox 2000"}, //
                          {JSONKey::Performers, "Jocker, Queen"}, //
                          {JSONKey::Tags, "Happiness, Comedy"}};
    QVERIFY(DumpJsonDict(fullDict, jsonPth));
    QVERIFY(dir.exists(jsonName));
    fileNameHash2Json = ReadStudioCastTagsOut(rootpath);
    QCOMPARE(fileNameHash2Json.size(), 1);
    const JsonDict2Table& info = fileNameHash2Json.cbegin().value();
    QCOMPARE(info.Studio, "Fox 2000");
    QCOMPARE(info.Cast, "Jocker, Queen");
    QCOMPARE(info.Tags, "Happiness, Comedy");
  };
};

#include "JsonFileHelperTest.moc"
JsonFileHelperTest gJsonFileHelperTest;
