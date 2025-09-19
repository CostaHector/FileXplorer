#include <QtTest>
#include <QCoreApplication>
#include "OnScopeExit.h"
#include "PlainTestSuite.h"
#include "TDir.h"
#include "JsonHelper.h"
#include "JsonPr.h"
#include "CastManager.h"

#include "BeginToExposePrivateMember.h"
#include "StudiosManager.h"
#include "EndToExposePrivateMember.h"

#include "NameTool.h"
#include "PathTool.h"
#include "PublicMacro.h"

class JsonPrTest : public PlainTestSuite {
  Q_OBJECT
 public:
  JsonPrTest() : PlainTestSuite{} {}
  TDir mDir;
  QString rootpath{mDir.path()};
  QList<FsNodeEntry> gNodeEntries;
  const QString fixedJsonName = "SuperMan - Henry Cavill.json";
  const QString fixedJsonBaseName = "SuperMan - Henry Cavill";
  const QString newJsonName = "SuperMan - Chris Evans.json";
  const QString occupiedJsonName = "My Good Boy.json";
  const QString fixedAbsPath{rootpath + '/' + fixedJsonName};
 private slots:
  void initTestCase() {
    static const char JSON_CONTENTS[]{R"({
    "Bitrate": "",
    "Cast": [
        "Cast1NotExist",
        "Cast2NotExist"
    ],
    "Detail": "This is just a json example.",
    "Duration": 0,
    "Hot": [
    ],
    "Name": "My Good Boy",
    "Rate": 4,
    "Resolution": "720p",
    "Size": "126113854",
    "Studio": "StudioNotExist",
    "Tags": [
        "nonporn"
    ],
    "Uploaded": "20231022"
}
)"};
    QVERIFY(mDir.IsValid());
    gNodeEntries = QList<FsNodeEntry>  //
        {
            FsNodeEntry{"cast_list.txt", false, {}},                    //
            FsNodeEntry{"My Good Boy.json", false, JSON_CONTENTS},      //
            FsNodeEntry{"studio_list.txt", false, {}},                  //
            FsNodeEntry{"SuperMan - Henry Cavill 1.jpg", false, {}},    //
            FsNodeEntry{"SuperMan - Henry Cavill 999.mp4", false, {}},  //
            FsNodeEntry{"SuperMan - Henry Cavill.jpg", false, {}},      //
            FsNodeEntry{"SuperMan - Henry Cavill.json", false, {}},
        };
    mDir.createEntries(gNodeEntries);
  }

  void init() {
    QFile fixedJsonFile{fixedAbsPath};
    if (fixedJsonFile.size() != 0) {
      fixedJsonFile.resize(0);
    }
  }

  void test_HintForCastStudio() {  //
    JsonPr jpr;
    bool studioChange{true}, castChanged{true};
    jpr.HintForCastStudio("", studioChange, castChanged);
    QCOMPARE(studioChange, false);
    QCOMPARE(castChanged, false);

    auto& psm = StudiosManager::getInst();
    STUDIO_MGR_DATA_T tempStudios;
    tempStudios["marvelfilms"] = "MarvelFilms";
    tempStudios["marvel films"] = "MarvelFilms";
    tempStudios["realmadrid"] = "ReadMadrid";
    tempStudios["real madrid"] = "ReadMadrid";
    psm.ProStudioMap().swap(tempStudios);
    ON_SCOPE_EXIT {
      psm.ProStudioMap().swap(tempStudios);
    };
    auto& pm = CastManager::getInst();
    decltype(pm.m_casts) tempCast{"a1 c1", "b1 d1", "a1 b1"};
    pm.m_casts.swap(tempCast);
    ON_SCOPE_EXIT {
      pm.m_casts.swap(tempCast);
    };

    QString sentence{"A1 C1, G1, A1 B1, B1 D1.mp4"};
    jpr.m_Name = "Marvel Films- Read Madrid - A1 C1, G1, A1 B1";

    studioChange = false;
    castChanged = false;
    jpr.HintForCastStudio(sentence, studioChange, castChanged);
    QCOMPARE(studioChange, true);
    QCOMPARE(castChanged, true);

    QSet<QString> expectHintCast{"A1 C1", "A1 B1", "B1 D1"};
    QStringList actualHintCastLst{jpr.hintCast.split(NameTool::CSV_COMMA)};
    QSet<QString> actualHintCast{actualHintCastLst.begin(), actualHintCastLst.end()};
    QCOMPARE(expectHintCast, actualHintCast);  // hint cast no need sorted
    jpr.RejectCastHint();
    QCOMPARE(jpr.hintCast, "");

    QCOMPARE(jpr.hintStudio, "MarvelFilms");
    jpr.RejectStudioHint();
    QCOMPARE(jpr.hintStudio, "");

    QCOMPARE(jpr.m_Name, "Marvel Films- Read Madrid - A1 C1, G1, A1 B1");  // name not change
    QVERIFY(jpr.m_Studio.isEmpty());                                       // studio not fill automatically
    QVERIFY(jpr.m_Cast.isEmpty());                                         // cast not fill automatically
  }

  void test_fromJsonFile() {
    // precondition
    const QFile fixedFi{fixedAbsPath};
    QCOMPARE(fixedFi.size(), 0);  // empty at first
    const auto& jPr = JsonPr::fromJsonFile(fixedFi.fileName());
    QCOMPARE(jPr.m_Prepath, rootpath);
    QCOMPARE(jPr.jsonFileName, fixedJsonName);

    // 2. "Name" find, other key need insert by StandardlizeKeyValue ok
    QVERIFY(jPr.m_Name != PathTool::GetBaseName(fixedJsonName));  // should not be replaced automatically
    QCOMPARE(jPr.m_Name, "");
    QVERIFY(jPr.m_Cast.isEmpty());
    QVERIFY(jPr.m_Studio.isEmpty());
    QVERIFY(jPr.m_Tags.isEmpty());
    QVERIFY(jPr.m_Detail.isEmpty());
    QVERIFY(jPr.m_Uploaded.isEmpty());
    QCOMPARE(jPr.m_Rate, 0);
    QCOMPARE(jPr.m_Size, 0);
    QVERIFY(jPr.m_Resolution.isEmpty());
    QVERIFY(jPr.m_Bitrate.isEmpty());
    QVERIFY(jPr.m_Hot.isEmpty());
    QCOMPARE(jPr.m_Duration, 0);
    QVERIFY(jPr.m_ImgName.isEmpty());
    QVERIFY(jPr.m_VidName.isEmpty());

    JsonPr jPr2{fixedFi.fileName()};
    QCOMPARE(jPr, jPr2);

    // default constructor and one path element constructor only differ in jsonFileName
    JsonPr jPr3;
    jPr3.jsonFileName = jPr2.jsonFileName;
    QCOMPARE(jPr3, jPr2);
  }

  void test_construct_ok() {
    // precondition
    const QFile fixedFi{fixedAbsPath};
    QCOMPARE(fixedFi.size(), 0);

    // procedure
    // 1. load from empty file
    JsonPr jPr{fixedFi.fileName()};
    QCOMPARE(jPr.m_Prepath, rootpath);
    QCOMPARE(jPr.jsonFileName, fixedJsonName);

    // 3. prepare a json that contains deprecated key ProductionStudio, Performers
    using namespace JsonKey;
    QVariantHash dict;
    dict[ENUM_2_STR(Name)] = "SuperMan - Henry Cavill [2020]";
    dict["ProductionStudio"] = "SuperMan";
    dict[ENUM_2_STR(Performers)] = QStringList{"Henry Cavill", "Chris Evans"};
    dict[ENUM_2_STR(Tags)] = QStringList{"friction", "science"};
    dict[ENUM_2_STR(Detail)] = "Description of SuperMan - Henry Cavill";
    dict[ENUM_2_STR(ImgName)] = QStringList{"A 2.jpg", "A 1.jpg"};
    dict[ENUM_2_STR(VidName)] = "A.mp4";
    QVERIFY(JsonHelper::DumpJsonDict(dict, jPr.GetAbsPath()));

    // Reload deprecated should be replaced by m_Studio, m_Cast in cache
    QVERIFY(jPr.Reload());
    QCOMPARE(jPr.m_Name, "SuperMan - Henry Cavill [2020]");
    QCOMPARE(jPr.m_Studio, "SuperMan");
    QCOMPARE(jPr.m_Cast.join(), "Chris Evans,Henry Cavill");
    QCOMPARE(jPr.m_Tags.join(), "friction,science");
    QCOMPARE(jPr.m_Detail, "Description of SuperMan - Henry Cavill");
    QCOMPARE(jPr.m_ImgName, (QStringList{"A 2.jpg", "A 1.jpg"}));
    QCOMPARE(jPr.m_VidName, "A.mp4");

    // 4. Write without deprecated key ProductionStudio, Performers should ok, hint filed should be cleared
    // Tags/Cast/Hot should be sorted and unique before write into json file
    jPr.hintCast = "Lalala..";
    jPr.hintStudio = "BLA.BLA..";
    QVERIFY(jPr.WriteIntoFiles());
    QVERIFY(jPr.hintCast.isEmpty());
    QVERIFY(jPr.hintStudio.isEmpty());

    const auto& writedJson = JsonHelper::MovieJsonLoader(jPr.GetAbsPath());
    QVERIFY(!writedJson.contains("ProductionStudio"));
    QVERIFY(!writedJson.contains(ENUM_2_STR(Performers)));
    QCOMPARE(writedJson[ENUM_2_STR(Name)], dict[ENUM_2_STR(Name)]);
    QCOMPARE(writedJson[ENUM_2_STR(Studio)], dict["ProductionStudio"]);
    QCOMPARE(writedJson[ENUM_2_STR(Cast)], (QStringList{"Chris Evans", "Henry Cavill"}));  // Cast should sorted
    QCOMPARE(writedJson[ENUM_2_STR(Tags)], (QStringList{"friction", "science"}));          // Tags should also sorted
    QCOMPARE(writedJson[ENUM_2_STR(Detail)], dict[ENUM_2_STR(Detail)]);
    QCOMPARE(writedJson[ENUM_2_STR(ImgName)], dict[ENUM_2_STR(ImgName)]);
    QCOMPARE(writedJson[ENUM_2_STR(VidName)], dict[ENUM_2_STR(VidName)]);
  }

  void test_Sync_Name_value_by_Json_file_basename() {
    // precondition
    const QFile fixedFi{fixedAbsPath};
    QCOMPARE(fixedFi.size(), 0);
    QCOMPARE(fixedJsonBaseName, PathTool::GetBaseName(fixedJsonName));

    // procedure
    JsonPr jPr{fixedFi.fileName()};
    jPr.m_Name = "Anything else name";
    // Sync first time ok
    QVERIFY(jPr.m_Name != fixedJsonBaseName);
    QVERIFY(jPr.SyncNameValueFromFileBaseName());
    QVERIFY(jPr.m_Name == fixedJsonBaseName);
    // Sync again, skip
    QVERIFY(!jPr.SyncNameValueFromFileBaseName());
    QVERIFY(jPr.m_Name == fixedJsonBaseName);
  }

  void test_Rename_ok() {
    const QStringList relatedNames{
        // environment exist MUST
        "SuperMan - Henry Cavill.json",    // fixedJsonName
        "SuperMan - Henry Cavill.jpg",     //
        "SuperMan - Henry Cavill 1.jpg",   //
        "SuperMan - Henry Cavill 999.mp4"  //
    };
    const QStringList newRelatedNames{
        // environment not exist MUST
        "SuperMan - Chris Evans.json",    // newJsonName
        "SuperMan - Chris Evans.jpg",     //
        "SuperMan - Chris Evans 1.jpg",   //
        "SuperMan - Chris Evans 999.mp4"  //
    };
    QCOMPARE(relatedNames.size(), newRelatedNames.size());

    // precondition
    QDir dir{rootpath};
    for (const QString& name : relatedNames) {
      QVERIFY2(dir.exists(name), qPrintable(name));
    }
    const QFile fixedFi{fixedAbsPath};
    QCOMPARE(fixedFi.size(), 0);

    // procedure
    // 1. load from empty file
    JsonPr jPr{fixedFi.fileName()};
    QCOMPARE(jPr.m_Prepath, rootpath);
    QCOMPARE(jPr.jsonFileName, fixedJsonName);

    // 2. rename succeed
    // name equal skip rename
    {
      QVERIFY(dir.exists(fixedJsonName));
      QCOMPARE(jPr.RenameJsonAndRelated(fixedJsonName), JsonPr::E_OK);
    }

    // json been moved
    {
      const QString movedFixedJsonName = "[moved]" + fixedJsonName;
      QVERIFY(dir.exists(fixedJsonName));
      QVERIFY(dir.rename(fixedJsonName, movedFixedJsonName));
      QVERIFY(!dir.exists(fixedJsonName));

      QCOMPARE(jPr.RenameJsonAndRelated(newJsonName), JsonPr::E_JSON_NOT_EXIST);
      ON_SCOPE_EXIT {
        QVERIFY(dir.rename(movedFixedJsonName, fixedJsonName));
      };
    }

    // new json name occupied
    {
      QVERIFY(dir.exists(occupiedJsonName));
      QCOMPARE(jPr.RenameJsonAndRelated(occupiedJsonName), JsonPr::E_JSON_NEW_NAME_OCCUPID);
    }

    // cnt = itself + else file
    {
      QCOMPARE(jPr.RenameJsonAndRelated(newJsonName), relatedNames.size());
      for (const QString& name : relatedNames) {
        QVERIFY(!dir.exists(name));
      }
      for (const QString& name : newRelatedNames) {
        QVERIFY(dir.exists(name));
      }
      ON_SCOPE_EXIT {
        for (int i = 0; i < newRelatedNames.size(); ++i) {
          QVERIFY(dir.rename(newRelatedNames[i], relatedNames[i]));
        }
      };
    }
  }

  void test_Construct_Clear_CastStudioValue() {
    auto& pm = CastManager::getInst();
    auto& psm = StudiosManager::getInst();
    decltype(pm.m_casts) tempCastsList{"chris hemsworth", "keanu reeves", "chris evans"};
    STUDIO_MGR_DATA_T tempStudiosMap{{"paramount pictures", "Paramount Pictures"}};
    pm.m_casts.swap(tempCastsList);
    psm.ProStudioMap().swap(tempStudiosMap);
    ON_SCOPE_EXIT {
      pm.m_casts.swap(tempCastsList);
      psm.ProStudioMap().swap(tempStudiosMap);
    };

    JsonPr jr{""};
    // 1. Name Fields value is empty, skip
    QVERIFY(jr.m_Name.isEmpty());
    QVERIFY(!jr.ConstructCastStudioValue());
    // 2.0 when inited studio/cast are empty, return false
    jr.m_Name = "Marvil Films X Men HughX JackmanX, MichaelX FassbenderX";
    QCOMPARE(pm.m_casts.contains(QString("HughX JackmanX").toLower()), false);
    QCOMPARE(pm.m_casts.contains(QString("MichaelX FassbenderX").toLower()), false);
    int isHypenIndexInvalid{-1};
    QCOMPARE(StudiosManager::isHypenIndexValid(jr.m_Name, isHypenIndexInvalid), false);
    QCOMPARE(isHypenIndexInvalid, -1);
    // 2.1 should no change
    QVERIFY(!jr.ConstructCastStudioValue());
    QCOMPARE(jr.m_Studio.isEmpty(), true);
    QCOMPARE(jr.m_Cast.isEmpty(), true);

    const QString& expectName{"Paramount Pictures - Thunder - Keanu Reeves, Chris Hemsworth, Chris Evans"};
    const QString& expectStudio{"Paramount Pictures"};
    const QStringList& expectCast{"Chris Evans", "Chris Hemsworth", "Keanu Reeves"};

    jr.m_Name = expectName;
    // construct first time, change
    QVERIFY(jr.ConstructCastStudioValue());
    QCOMPARE(jr.m_Studio, expectStudio);
    QCOMPARE(jr.m_Cast.toSortedList(), expectCast);
    // construct again, no change
    QVERIFY(!jr.ConstructCastStudioValue());
    QCOMPARE(jr.m_Studio, expectStudio);
    QCOMPARE(jr.m_Cast.toSortedList(), expectCast);

    // clear first time, change
    QVERIFY(!jr.m_Cast.isEmpty());
    QVERIFY(!jr.m_Studio.isEmpty());
    QVERIFY(jr.ClearCastStudioValue());
    QVERIFY(jr.m_Cast.isEmpty());
    QVERIFY(jr.m_Studio.isEmpty());
    // clear again, no change
    QVERIFY(!jr.ClearCastStudioValue());
    QVERIFY(jr.m_Cast.isEmpty());
    QVERIFY(jr.m_Studio.isEmpty());
  }

  void test_SetStudio_ok() {
    JsonPr jr{""};
    // set new name first time, change
    QString newStudioName = "New XXX Studio";
    QVERIFY(jr.m_Name != newStudioName);
    QVERIFY(jr.SetStudio(newStudioName));
    QCOMPARE(jr.m_Studio, newStudioName);
    // set same name again, no change
    QVERIFY(!jr.SetStudio(newStudioName));
    QCOMPARE(jr.m_Studio, newStudioName);
  }

  void test_SetCastOrTags_ok() {
    JsonPr jrDefaultConstuct;
    QCOMPARE(jrDefaultConstuct.m_Name, "");
    QCOMPARE(jrDefaultConstuct.jsonFileName, "");

    JsonPr jr{""};
    QVERIFY(!jr.SetCastOrTags("", FIELD_OP_TYPE::BUTT, FIELD_OP_MODE::BUTT));
    QVERIFY(!jr.SetCastOrTags("", FIELD_OP_TYPE::BUTT, FIELD_OP_MODE::SET));
    QVERIFY(!jr.SetCastOrTags("", FIELD_OP_TYPE::BUTT, FIELD_OP_MODE::APPEND));
    QVERIFY(!jr.SetCastOrTags("", FIELD_OP_TYPE::BUTT, FIELD_OP_MODE::REMOVE));
    QVERIFY(!jr.SetCastOrTags("", FIELD_OP_TYPE::CAST, FIELD_OP_MODE::BUTT));
    QVERIFY(!jr.SetCastOrTags("", FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::BUTT));

    // --------------------------- Cast
    jr.m_Cast.setBatch(QStringList{"C", "D"});
    QCOMPARE(jr.m_Cast.join(), "C,D");

    // set first time, change
    QVERIFY(jr.SetCastOrTags("A,B", FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET));
    QCOMPARE(jr.m_Cast.join(), "A,B");
    // set again, no change
    QVERIFY(jr.SetCastOrTags("A,B", FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET));
    QCOMPARE(jr.m_Cast.join(), "A,B");

    // append first time, change
    QVERIFY(jr.SetCastOrTags("C,D", FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND));
    QCOMPARE(jr.m_Cast.join(), "A,B,C,D");
    // append again, no change
    QVERIFY(jr.SetCastOrTags("C,D", FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND));
    QCOMPARE(jr.m_Cast.join(), "A,B,C,D");

    // remove first time, change
    QVERIFY(jr.SetCastOrTags("C", FIELD_OP_TYPE::CAST, FIELD_OP_MODE::REMOVE));
    QCOMPARE(jr.m_Cast.join(), "A,B,D");
    // remove again, no change
    QVERIFY(jr.SetCastOrTags("C", FIELD_OP_TYPE::CAST, FIELD_OP_MODE::REMOVE));
    QCOMPARE(jr.m_Cast.join(), "A,B,D");

    // set an empty sentence should empty all
    QVERIFY(jr.SetCastOrTags("", FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET));
    QVERIFY(jr.m_Cast.isEmpty());

    // --------------------------- Tags
    jr.m_Tags.setBatch(QStringList{"C", "D"});
    QCOMPARE(jr.m_Tags.join(), "C,D");

    // set first time, change
    QVERIFY(jr.SetCastOrTags("A,B", FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::SET));
    QCOMPARE(jr.m_Tags.join(), "A,B");
    // set again, no change
    QVERIFY(jr.SetCastOrTags("A,B", FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::SET));
    QCOMPARE(jr.m_Tags.join(), "A,B");

    // append first time, change
    QVERIFY(jr.SetCastOrTags("C,D", FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::APPEND));
    QCOMPARE(jr.m_Tags.join(), "A,B,C,D");
    // append again, no change
    QVERIFY(jr.SetCastOrTags("C,D", FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::APPEND));
    QCOMPARE(jr.m_Tags.join(), "A,B,C,D");

    // remove first time, change
    QVERIFY(jr.SetCastOrTags("C", FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::REMOVE));
    QCOMPARE(jr.m_Tags.join(), "A,B,D");
    // remove again, no change
    QVERIFY(jr.SetCastOrTags("C", FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::REMOVE));
    QCOMPARE(jr.m_Tags.join(), "A,B,D");

    // set an empty sentence should empty all
    QVERIFY(jr.SetCastOrTags("", FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::SET));
    QVERIFY(jr.m_Tags.isEmpty());
  }
};

#include "JsonPrTest.moc"
REGISTER_TEST(JsonPrTest, false)
