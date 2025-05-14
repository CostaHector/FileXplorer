#include <QtTest>
#include <QCoreApplication>
#include "TestCase/PathRelatedTool.h"
#include "TestCase/pub/OnScopeExit.h"
#include "TestCase/pub/MyTestSuite.h"
#include "Tools/Json/JsonKey.h"
#include "Tools/Json/JsonHelper.h"
#include "public/PublicMacro.h"

const QString rootpath = TestCaseRootPath() + "/test/TestEnv_VideosDurationGetter";

using namespace JsonHelper;
using namespace JsonKey;

class JsonFileHelperTest : public MyTestSuite {
  Q_OBJECT

 public:
  JsonFileHelperTest() : MyTestSuite{false} {}
 private slots:
  void test_GetDefaultJsonFile() {
    const auto dict = GetJsonDictDefault();
    QCOMPARE(dict[ENUM_TO_STRING(Name)].toString(), JSON_DEF_VAL_Name);
    QCOMPARE(dict[ENUM_TO_STRING(Cast)].toStringList(), JSON_DEF_VAL_Cast);
    QCOMPARE(dict[ENUM_TO_STRING(Studio)].toString(), JSON_DEF_VAL_Studio);
    QCOMPARE(dict[ENUM_TO_STRING(Tags)].toStringList(), JSON_DEF_VAL_Tags);
    QCOMPARE(dict[ENUM_TO_STRING(Rate)].toInt(), JSON_DEF_VAL_Rate);
    QCOMPARE(dict[ENUM_TO_STRING(Size)].toInt(), JSON_DEF_VAL_Size);
    QCOMPARE(dict[ENUM_TO_STRING(Hot)].toStringList(), JSON_DEF_VAL_Hot);
    QCOMPARE(dict[ENUM_TO_STRING(Detail)].toString(), JSON_DEF_VAL_Detail);
  }

  void test_JsonStr2Dict() {
    const char jsonStrArray[] = R"({"Henry Cavill": ["tall",  "muscle"], "Chris Evans": 192})";
    const QVariantHash actualDict = DeserializedJsonStr2Dict(jsonStrArray);
    const QVariantHash expectDict{
        {"Henry Cavill", QStringList{"tall", "muscle"}},  //
        {"Chris Evans", 192}                              //
    };
    QCOMPARE(actualDict, expectDict);
  }

  void test_reconstruct_cast_studio_ok() {
    // two cast are not from Name, after clear, construct cannot recover cast
    QVariantHash dictWithPerfAndStudio{
        {"Cast", QStringList{"Chris Hemsworth Studio", "Keanu Reeves Studio"}},  //
        {"Name", "Marvel Film"}                                                  //
    };
    QVERIFY(dictWithPerfAndStudio.contains(ENUM_TO_STRING(Cast)));
    QVERIFY(!dictWithPerfAndStudio.contains(ENUM_TO_STRING(Studio)));
    QVERIFY(!dictWithPerfAndStudio[ENUM_TO_STRING(Cast)].toStringList().isEmpty());
    DictEditOperator::ReCastAndStudio cps;
    QVERIFY(cps(dictWithPerfAndStudio));
    QVERIFY(dictWithPerfAndStudio.contains(ENUM_TO_STRING(Cast)));
    QVERIFY(!dictWithPerfAndStudio.contains(ENUM_TO_STRING(Studio)));
    QVERIFY(dictWithPerfAndStudio[ENUM_TO_STRING(Cast)].toStringList().isEmpty());
  }

  void test_reconstruct_skip_when_cast_studio_not_exist() {
    QVariantHash dictWithOnlyName{{"Name", "Chris Hemsworth life story"}};
    QVERIFY(dictWithOnlyName.contains(ENUM_TO_STRING(Name)));
    QVERIFY(!dictWithOnlyName.contains(ENUM_TO_STRING(Cast)));
    QVERIFY(!dictWithOnlyName.contains(ENUM_TO_STRING(Studio)));
    DictEditOperator::ReCastAndStudio cps;
    QVERIFY(!cps(dictWithOnlyName));
    QVERIFY(dictWithOnlyName.contains(ENUM_TO_STRING(Name)));
    QVERIFY(!dictWithOnlyName.contains(ENUM_TO_STRING(Cast)));
    QVERIFY(!dictWithOnlyName.contains(ENUM_TO_STRING(Studio)));
  }

  void test_ConstructStudioCastByName_ok() {
    const QString& expectName{"Paramount Pictures - Thunder - Chris Hemsworth, Keanu Reeves"};
    const QString& expectStudio{"Paramount Pictures"};
    const QStringList& expectCast{"Chris Hemsworth", "Keanu Reeves"};
    DictEditOperator::ConstructStudioCastByName ipdnh;

    // 0. no name skip
    QVariantHash dictWithPerfs;
    QVERIFY(!ipdnh(dictWithPerfs));
    QVERIFY(dictWithPerfs.isEmpty());

    // 1. name empty, all key added default
    dictWithPerfs["Name"] = JSON_DEF_VAL_Name;
    QVERIFY(ipdnh(dictWithPerfs));
    auto nameIt = dictWithPerfs.find(ENUM_TO_STRING(Name));
    QVERIFY(nameIt != dictWithPerfs.end());
    QCOMPARE(nameIt.value().toString(), JSON_DEF_VAL_Name);

    auto studioIt = dictWithPerfs.find(ENUM_TO_STRING(Studio));
    QVERIFY(studioIt != dictWithPerfs.end());
    QCOMPARE(studioIt.value().toString(), JSON_DEF_VAL_Studio);

    auto perfIt = dictWithPerfs.find(ENUM_TO_STRING(Cast));
    QVERIFY(perfIt != dictWithPerfs.end());
    QCOMPARE(perfIt.value().toStringList(), JSON_DEF_VAL_Cast);

    // 2. all full no need change
    dictWithPerfs["Name"] = expectName;
    dictWithPerfs["Studio"] = expectStudio;
    dictWithPerfs["Cast"] = expectCast;
    QVERIFY(!ipdnh(dictWithPerfs));

    nameIt = dictWithPerfs.find(ENUM_TO_STRING(Name));
    QVERIFY(nameIt != dictWithPerfs.end());
    QCOMPARE(nameIt.value().toString(), expectName);

    studioIt = dictWithPerfs.find(ENUM_TO_STRING(Studio));
    QVERIFY(studioIt != dictWithPerfs.end());
    QCOMPARE(studioIt.value().toString(), expectStudio);

    perfIt = dictWithPerfs.find(ENUM_TO_STRING(Cast));
    QVERIFY(perfIt != dictWithPerfs.end());
    QCOMPARE(perfIt.value().toStringList(), expectCast);
  }

  void test_AppendPerfsToDict_sorted_and_unique_ok() {
    const QString toBeInsertedPerfs{"A, B, and C, D"};
    DictEditOperator::AppendPerfsToDict apd{toBeInsertedPerfs};

    QVariantHash dictPerfs{{"Cast", QStringList{"A", "E"}}};
    const QStringList alreadyExistPerfs{"A", "E"};
    QCOMPARE(dictPerfs[ENUM_TO_STRING(Cast)], alreadyExistPerfs);
    QVERIFY(apd(dictPerfs));  // B C D inserted ok, A E already exist
    auto perfIt = dictPerfs.find(ENUM_TO_STRING(Cast));
    QVERIFY(perfIt != dictPerfs.end());
    const QStringList afterInsertedPerfs{"A", "B", "C", "D", "E"};
    QCOMPARE(perfIt.value().toStringList(), afterInsertedPerfs);
  }

  void test_AppendPerfsToDict_skip_alreadyExist() {
    const QString toBeInsertedPerfs{"A and E"};
    DictEditOperator::AppendPerfsToDict apd{toBeInsertedPerfs};

    QVariantHash dictPerfs{{"Cast", QStringList{"A", "E"}}};
    const QStringList alreadyExistPerfs{"A", "E"};
    QCOMPARE(dictPerfs[ENUM_TO_STRING(Cast)], alreadyExistPerfs);
    QVERIFY(!apd(dictPerfs));  // A E already exist, nothing inserted
    auto perfIt = dictPerfs.find(ENUM_TO_STRING(Cast));
    QVERIFY(perfIt != dictPerfs.end());
    QCOMPARE(perfIt.value().toStringList(), alreadyExistPerfs);
  }

  void test_UpdateStudio_skip_studio_notChange() {
    DictEditOperator::UpdateStudio us{"Fox"};

    QVariantHash dict{{"Name", ""}, {"Studio", "Fox"}};
    QVERIFY(dict.contains(ENUM_TO_STRING(Studio)));
    QCOMPARE(dict[ENUM_TO_STRING(Studio)].toString(), "Fox");
    QVERIFY(!us(dict));
    QVERIFY(dict.contains(ENUM_TO_STRING(Studio)));
    QCOMPARE(dict[ENUM_TO_STRING(Studio)].toString(), "Fox");
  }

  void test_UpdateStudio_studio_pair_insertOk() {
    DictEditOperator::UpdateStudio us{"Fox"};
    QVariantHash dict{{"Name", ""}};
    QVERIFY(!dict.contains(ENUM_TO_STRING(Studio)));
    QVERIFY(us(dict));
    auto studioIt = dict.find(ENUM_TO_STRING(Studio));
    QVERIFY(studioIt != dict.end());
    QCOMPARE(studioIt.value().toString(), "Fox");
  }

  void test_UpdateStudio_studioName_ChangedOk() {
    DictEditOperator::UpdateStudio us{"Fox"};
    QVariantHash dict{{"Name", ""}, {"Studio", "Marvel"}};
    QVERIFY(dict.contains(ENUM_TO_STRING(Studio)));
    QCOMPARE(dict[ENUM_TO_STRING(Studio)].toString(), "Marvel");
    QVERIFY(us(dict));
    auto studioIt = dict.find(ENUM_TO_STRING(Studio));
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

    // json changed
    const QStringList expectCastLst{"Henry Cavill", "Paddy", "Chris", "Fassbender"};
    const QStringList expectTagsLst{"Comedy"};
    ret = InsertOrUpdateDurationStudioCastTags(jsonPth, 8, "Fox 20 century", expectCastLst.join(ELEMENT_JOINER), expectTagsLst.join(ELEMENT_JOINER));
    QCOMPARE(ret, CHANGED_OK);

    const auto& dict = MovieJsonLoader(jsonPth);
    QCOMPARE(dict.value(ENUM_TO_STRING(Duration)).toInt(), 8);
    QCOMPARE(dict.value(ENUM_TO_STRING(Studio)).toString(), "Fox 20 century");
    QCOMPARE(dict.value(ENUM_TO_STRING(Cast)).toStringList(), expectCastLst);
    QCOMPARE(dict.value(ENUM_TO_STRING(Tags)).toStringList(), expectTagsLst);

    // json unchange
    ret = InsertOrUpdateDurationStudioCastTags(jsonPth, 8, "Fox 20 century", expectCastLst.join(ELEMENT_JOINER), expectTagsLst.join(ELEMENT_JOINER));
    QCOMPARE(ret, NOCHANGED_OK);
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
    QVariantHash emptyValueDict{{ENUM_TO_STRING(Studio), ""}, {ENUM_TO_STRING(Cast), QStringList{}}, {ENUM_TO_STRING(Tags), QStringList{}}};
    QVERIFY(DumpJsonDict(emptyValueDict, jsonPth));
    QVERIFY(dir.exists(jsonName));
    QMap<uint, JsonDict2Table> fileNameHash2Json = ReadStudioCastTagsOut(rootpath);
    QVERIFY(fileNameHash2Json.isEmpty());

    // 2. only contains studio and tags, but not performers
    QVariantHash notFullDict{{ENUM_TO_STRING(Studio), "Fox 2000"}, {ENUM_TO_STRING(Tags), QStringList{"Happiness", "Comedy"}}};
    QVERIFY(DumpJsonDict(notFullDict, jsonPth));
    QVERIFY(dir.exists(jsonName));
    fileNameHash2Json = ReadStudioCastTagsOut(rootpath);
    QVERIFY(fileNameHash2Json.isEmpty());

    // 3. contains studio and tags and performers
    QVariantHash fullDict{{ENUM_TO_STRING(Studio), "Fox 2000"},                    //
                          {ENUM_TO_STRING(Cast), QStringList{"Jocker", "Queen"}},  //
                          {ENUM_TO_STRING(Tags), QStringList{"Happiness", "Comedy"}}};
    QVERIFY(DumpJsonDict(fullDict, jsonPth));
    QVERIFY(dir.exists(jsonName));
    fileNameHash2Json = ReadStudioCastTagsOut(rootpath);
    QCOMPARE(fileNameHash2Json.size(), 1);
    const JsonDict2Table& info = fileNameHash2Json.cbegin().value();
    const QStringList expectCasts{"Jocker", "Queen"};
    const QStringList expectTags{"Happiness", "Comedy"};
    QCOMPARE(info.Studio, "Fox 2000");
    QCOMPARE(info.Cast, expectCasts);
    QCOMPARE(info.Tags, expectTags);
  };
};

#include "JsonFileHelperTest.moc"
JsonFileHelperTest gJsonFileHelperTest;
