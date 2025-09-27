#include <QtTest>
#include <QCoreApplication>

#include "OnScopeExit.h"
#include "PlainTestSuite.h"
#include "TDir.h"
#include "JsonKey.h"
#include "JsonHelper.h"
#include "JsonPr.h"
#include "PublicMacro.h"

using namespace JsonHelper;
using namespace JsonKey;

class JsonHelperTest : public PlainTestSuite {
  Q_OBJECT

 public:
  JsonHelperTest() : PlainTestSuite{} {}
  TDir mDir;
  const QString mWorkPath{mDir.path()};
  static const char mJsonContentStr[];
  static const char mInvalidJsonContentStr[];
 private slots:
  void initTestCase() {
    QVERIFY(mDir.IsValid());
    QVERIFY(mDir.touch("file_with_non_utf8_char.json", mJsonContentStr));
    QVERIFY(mDir.touch("file_with_extra_trailing_comma_invalid.json", mInvalidJsonContentStr));
  }

  void test_GetDefaultJsonFile() {
    const auto dict = GetJsonDictDefault();
    QCOMPARE(dict[ENUM_2_STR(Name)].toString(), JSON_DEF_VAL_Name);
    QCOMPARE(dict[ENUM_2_STR(Cast)].toStringList(), JSON_DEF_VAL_Cast);
    QCOMPARE(dict[ENUM_2_STR(Studio)].toString(), JSON_DEF_VAL_Studio);
    QCOMPARE(dict[ENUM_2_STR(Tags)].toStringList(), JSON_DEF_VAL_Tags);
    QCOMPARE(dict[ENUM_2_STR(Rate)].toInt(), JSON_DEF_VAL_Rate);
    QCOMPARE(dict[ENUM_2_STR(Size)].toInt(), JSON_DEF_VAL_Size);
    QCOMPARE(dict[ENUM_2_STR(Hot)].toStringList(), JSON_DEF_VAL_Hot);
    QCOMPARE(dict[ENUM_2_STR(Detail)].toString(), JSON_DEF_VAL_Detail);
  }

  void test_JsonStr2Dict_JsonDict2ByteArray() {
    const char jsonStr[] = R"({"Henry Cavill": ["tall",  "muscle"], "Chris Evans": 192})";
    const QVariantHash expectDict{
        {"Henry Cavill", QStringList{"tall", "muscle"}},  //
        {"Chris Evans", 192}                              //
    };
    bool bParseStrOk = false;
    const QVariantHash dictFromStr = DeserializedJsonStr2Dict(jsonStr, &bParseStrOk);
    QVERIFY(bParseStrOk);
    QCOMPARE(dictFromStr, expectDict);

    const QByteArray jsonByteArray = SerializedJsonDict2ByteArray(dictFromStr);
    bool bParseBaOk = false;
    const QVariantHash dictFromBa = DeserializedJsonByteArray2Dict(jsonByteArray, &bParseBaOk);
    QVERIFY(bParseBaOk);
    QCOMPARE(dictFromBa, expectDict);

    { // bounder test
      // 1.0 empty dict
      const char emptyJsonStr[] = R"({})";
      bool bParseEmptyStr = false;
      const QVariantHash dictFromEmptyStr = DeserializedJsonStr2Dict(emptyJsonStr, &bParseEmptyStr);
      QVERIFY(bParseEmptyStr);
      QVERIFY(dictFromEmptyStr.isEmpty());

      const QByteArray emptyJsonByteArray = SerializedJsonDict2ByteArray(dictFromEmptyStr);
      bool bParseEmptyBaOk = false;
      const QVariantHash dictFromEmptyBa = DeserializedJsonByteArray2Dict(emptyJsonByteArray, &bParseEmptyBaOk);
      QVERIFY(bParseEmptyBaOk);
      QVERIFY(dictFromEmptyBa.isEmpty());

      // 2.0 invalid extra comma at the end
      const char invalidJsonStr[] = R"({"Name": "Chris Evans",})";
      bool bParseInvalidStr = false;
      const QVariantHash dictFromInvalidStr = DeserializedJsonStr2Dict(invalidJsonStr, &bParseInvalidStr);
      QVERIFY(!bParseInvalidStr);
      QVERIFY(dictFromInvalidStr.isEmpty());
    }
  }

  void invalid_json_parse_failed() {
    QString invalidJsonAbsFilePath = mDir.itemPath("file_with_extra_trailing_comma_invalid.json");
    QVERIFY(QFile::exists(invalidJsonAbsFilePath));
    QJsonObject jsonObj = GetJsonObject(invalidJsonAbsFilePath);
    QVERIFY(jsonObj.isEmpty());
  }

  void test_GetJsonObject_ByteArrayWriter_contains_non_utf8_char_ok() {  // priority: top 1
    // contents in json file is below, will recover automatically
    // precondition
    QDir dir{mWorkPath};
    const QString nonUtf8JsonFileName = "file_with_non_utf8_char.json";
    QVERIFY(dir.exists(nonUtf8JsonFileName));
    QFile jsonFi{dir.absoluteFilePath(nonUtf8JsonFileName)};
    QVERIFY(jsonFi.size() > 0);
    ON_SCOPE_EXIT {
      QVERIFY(jsonFi.open(QIODevice::OpenModeFlag::WriteOnly));
      QVERIFY(jsonFi.write(mJsonContentStr) >= 0);
      jsonFi.close();
    };

    const QString jsonAbsPath = dir.absoluteFilePath(nonUtf8JsonFileName);

    // load ok (using bytearray)
    JsonPr json1 = JsonPr::fromJsonFile(jsonAbsPath);
    QCOMPARE(json1.m_Detail, "Frank boy’s adventure.");

    // write ok (using bytearray)
    QVERIFY(json1.WriteIntoFiles());

    // reload ok (using bytearray)
    JsonPr json2 = JsonPr::fromJsonFile(jsonFi.fileName());
    QCOMPARE(json2.m_Detail, "Frank boy’s adventure.");

    QCOMPARE(json1, json2);
  }

  void test_InsertOrUpdateDurationStudioCastTags() {  //
    QDir dir{mWorkPath};

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
    QCOMPARE(dict.value(ENUM_2_STR(Duration)).toInt(), 8);
    QCOMPARE(dict.value(ENUM_2_STR(Studio)).toString(), "Fox 20 century");
    QCOMPARE(dict.value(ENUM_2_STR(Cast)).toStringList(), expectCastLst);
    QCOMPARE(dict.value(ENUM_2_STR(Tags)).toStringList(), expectTagsLst);

    // json unchange
    ret = InsertOrUpdateDurationStudioCastTags(jsonPth, 8, "Fox 20 century", expectCastLst.join(ELEMENT_JOINER), expectTagsLst.join(ELEMENT_JOINER));
    QCOMPARE(ret, NOCHANGED_OK);
  }

  void test_ReadStudioCastTagsOut() {
    QDir dir{mWorkPath};
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
    QVariantHash emptyValueDict{{ENUM_2_STR(Studio), ""}, {ENUM_2_STR(Cast), QStringList{}}, {ENUM_2_STR(Tags), QStringList{}}};
    QVERIFY(DumpJsonDict(emptyValueDict, jsonPth));
    QVERIFY(dir.exists(jsonName));
    QMap<uint, JsonDict2Table> fileNameHash2Json = ReadStudioCastTagsOut(mWorkPath);
    QVERIFY(fileNameHash2Json.isEmpty());

    // 2. only contains studio and tags, but not performers
    QVariantHash notFullDict{{ENUM_2_STR(Studio), "Fox 2000"}, {ENUM_2_STR(Tags), QStringList{"Happiness", "Comedy"}}};
    QVERIFY(DumpJsonDict(notFullDict, jsonPth));
    QVERIFY(dir.exists(jsonName));
    fileNameHash2Json = ReadStudioCastTagsOut(mWorkPath);
    QVERIFY(fileNameHash2Json.isEmpty());

    // 3. contains studio and tags and performers
    QVariantHash fullDict{{ENUM_2_STR(Studio), "Fox 2000"},                    //
                          {ENUM_2_STR(Cast), QStringList{"Jocker", "Queen"}},  //
                          {ENUM_2_STR(Tags), QStringList{"Happiness", "Comedy"}}};
    QVERIFY(DumpJsonDict(fullDict, jsonPth));
    QVERIFY(dir.exists(jsonName));
    fileNameHash2Json = ReadStudioCastTagsOut(mWorkPath);
    QCOMPARE(fileNameHash2Json.size(), 1);
    const JsonDict2Table& info = fileNameHash2Json.cbegin().value();
    const QStringList expectCasts{"Jocker", "Queen"};
    const QStringList expectTags{"Happiness", "Comedy"};
    QCOMPARE(info.Studio, "Fox 2000");
    QCOMPARE(info.Cast, expectCasts);
    QCOMPARE(info.Tags, expectTags);
  }
};

const char JsonHelperTest::mJsonContentStr[]{R"(
{
    "Name": "Frank - 1",
    "Cast": [],
    "Studio": "Marvel",
    "Duration": 0,
    "Uploaded": "17:22 25-Apr-1999",
    "Tags": [
        "comedy"
    ],
    "Rate": 0,
    "Size": "1.00 MiB",
    "Resolution": "1080p",
    "Bitrate": "5000 kbps",
    "Detail": "Frank boy’s adventure."
}
)"};
const char JsonHelperTest::mInvalidJsonContentStr[]{R"({"Name": "Henry Canvill", })"};  // error: extra trailing comma here

#include "JsonHelperTest.moc"
REGISTER_TEST(JsonHelperTest, false)
