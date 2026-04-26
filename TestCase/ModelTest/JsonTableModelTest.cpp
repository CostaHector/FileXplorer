#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include "BeginToExposePrivateMember.h"
#include "JsonTableModel.h"
#include "StudiosManager.h"
#include "CastManager.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "AutoRollbackFileContentModify.h"
#include "JsonTestPrecoditionTools.h"
#include "PublicVariable.h"
#include "FileTool.h"
#include "PathTool.h"
#include "ModelTestHelper.h"
#include "StudioActorManagerTestHelper.h"

#include "VideoDurationGetter.h"
#include "MD5Calculator.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

using namespace JsonTestPrecoditionTools;

class JsonTableModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
  static constexpr int mJsonsFileCountInitial = 2;

  ActorManagerTestHelper actorHelper{mTDir};
  StudioManagerTestHelper studioHelper{mTDir};

  const QList<FsNodeEntry> nodes{
      //
      {"a.json", false, JSON_CONTENTS_A_RANK_IN_MODEL},               //
      {"b.json", false, JSON_CONTENTS_B_RANK_IN_MODEL},               //
      {"jazz .txt", false, "don't care me here"},                     //
      {"a.mp4", false, "only care me in rename json related files"},  //
  };

  StudiosManager& studioMgr = StudiosManager::getInst();
  CastManager& actorMgr = CastManager::getInst();

  void CheckModelData(JsonTableModel& model) {
    QCOMPARE(model.data(model.index(0, JsonKey::Name), Qt::DisplayRole).toString(), "GameTurbo - A rank - GGG YYYYY");
    QCOMPARE(model.data(model.index(0, JsonKey::Cast), Qt::DisplayRole).toString(), "Empty Cast A 1,Empty Cast A 2");
    QCOMPARE(model.data(model.index(0, JsonKey::Studio), Qt::DisplayRole).toString(), "Empty Studio A");
    QCOMPARE(model.data(model.index(0, JsonKey::Tags), Qt::DisplayRole).toString(), "Empty Tag A");
    QCOMPARE(model.data(model.index(0, JsonKey::Detail), Qt::DisplayRole).toString(), "This is just a json example.");
    QCOMPARE(model.data(model.index(0, JsonKey::Duration), Qt::DisplayRole).toString(), "00:00:36");  // 36000ms

    QCOMPARE(model.data(model.index(1, JsonKey::Name), Qt::DisplayRole).toString(), "GameTurbo - B rank - XX YY and ZZ DD EE");
    QCOMPARE(model.data(model.index(1, JsonKey::Cast), Qt::DisplayRole).toString(), "Empty Cast B 1,Empty Cast B 2");
    QCOMPARE(model.data(model.index(1, JsonKey::Studio), Qt::DisplayRole).toString(), "Empty Studio B");
    QCOMPARE(model.data(model.index(1, JsonKey::Tags), Qt::DisplayRole).toString(), "Empty Tag B");
    QCOMPARE(model.data(model.index(1, JsonKey::Detail), Qt::DisplayRole).toString(), "This is just b json example.");
    QCOMPARE(model.data(model.index(1, JsonKey::Duration), Qt::DisplayRole).toString(), "00:00:03");  // 3600ms
  }

 private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());

    bool readAok = false;
    QCOMPARE(FileTool::TextReader(mTDir.itemPath("a.json"), &readAok), JSON_CONTENTS_A_RANK_IN_MODEL);

    GlobalMockObject::reset();
    actorHelper.init();
    studioHelper.init();
  }

  void cleanupTestCase() {  //
    GlobalMockObject::verify();
  }

  void initalize_ok() {
    JsonTableModel jtm;
    QCOMPARE(jtm.rowCount(), 0);
    QCOMPARE(jtm.columnCount(), JsonKey::JSON_TABLE_HEADERS_COUNT);
    QVERIFY(1 <= JsonKey::JSON_TABLE_HEADERS_COUNT && JsonKey::JSON_TABLE_HEADERS_COUNT <= 999);
    QCOMPARE(jtm.headerData(0, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(), JsonKey::JSON_TABLE_HEADERS[0]);
    QCOMPARE(jtm.headerData(999, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toInt(), 999 + 1);
    QCOMPARE(jtm.headerData(0, Qt::Horizontal, Qt::ItemDataRole::TextAlignmentRole).isNull(), true);

    QCOMPARE(jtm.headerData(3, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(),
             3 + 1);  // not depend on array, never out of
    QCOMPARE(jtm.headerData(0, Qt::Vertical, Qt::ItemDataRole::TextAlignmentRole).toInt(), ((int)Qt::AlignRight));
    QCOMPARE(jtm.headerData(102400000, Qt::Orientation::Vertical, Qt::ItemDataRole::ForegroundRole).isNull(), true);

    QVERIFY(jtm.rel2fileNames({}).isEmpty());

    {  // should not crash down
      QCOMPARE(jtm.setModified(102400, true), false);
      QCOMPARE(jtm.setModifiedNoEmit(102400, true), false);
      QCOMPARE(jtm.m_modifiedRows.any(), false);

      QModelIndex invalidIndex;
      QModelIndexList invalidIndexes{invalidIndex, invalidIndex};
      QCOMPARE(jtm.data(invalidIndex, Qt::ItemDataRole::DisplayRole).isNull(), true);

      jtm.setData(invalidIndex, "123", Qt::ItemDataRole::DisplayRole);
      jtm.forceReloadPath();

      QFileInfo invalidFi = jtm.fileInfo(invalidIndex);
      QCOMPARE(invalidFi.absoluteFilePath(), "");
      QCOMPARE(jtm.filePath(invalidIndex), "");
      QCOMPARE(jtm.fileName(invalidIndex), "");
      QCOMPARE(jtm.fileBaseName(invalidIndex), "");
      QCOMPARE(jtm.absolutePath(invalidIndex), "");
      QCOMPARE(jtm.fullInfo(invalidIndex), "");

      QCOMPARE(jtm.SetStudio({}, "new cast"), 0);
      QCOMPARE(jtm.SetStudio(invalidIndexes, "new cast"), 0);
      QCOMPARE(jtm.SetCastOrTags(invalidIndexes, JSON_KEY_E::Name, "SetCastOrTags not allowed field"), -1);
      QCOMPARE(jtm.SetCastOrTags(invalidIndexes, JSON_KEY_E::Cast, "learn from this sentence"), 0);
      QCOMPARE(jtm.AddCastOrTags(invalidIndexes, JSON_KEY_E::Name, "AddCastOrTags not allowed field"), -1);
      QCOMPARE(jtm.AddCastOrTags(invalidIndexes, JSON_KEY_E::Cast, "learn from this sentence"), 0);
      QCOMPARE(jtm.RmvCastOrTags(invalidIndexes, JSON_KEY_E::Name, "AddCastOrTags not allowed field"), -1);
      QCOMPARE(jtm.RmvCastOrTags(invalidIndexes, JSON_KEY_E::Cast, "remove me cast"), 0);
      QCOMPARE(jtm.SetCastOrTags({}, JSON_KEY_E::Cast, "learn from this sentence"), 0);
      QCOMPARE(jtm.AddCastOrTags({}, JSON_KEY_E::Cast, "learn from this sentence"), 0);
      QCOMPARE(jtm.RmvCastOrTags({}, JSON_KEY_E::Cast, "remove me cast"), 0);
      QCOMPARE(jtm.SetCastOrTags(invalidIndexes, JSON_KEY_E::Tags, "learn from this sentence"), 0);
      QCOMPARE(jtm.AddCastOrTags(invalidIndexes, JSON_KEY_E::Tags, "learn from this sentence"), 0);
      QCOMPARE(jtm.RmvCastOrTags(invalidIndexes, JSON_KEY_E::Tags, "remove me tag"), 0);
      QCOMPARE(jtm.SetCastOrTags({}, JSON_KEY_E::Tags, "learn from this sentence"), 0);
      QCOMPARE(jtm.AddCastOrTags({}, JSON_KEY_E::Tags, "learn from this sentence"), 0);
      QCOMPARE(jtm.RmvCastOrTags({}, JSON_KEY_E::Tags, "remove me tag"), 0);
      QCOMPARE(jtm.InitCastAndStudio(invalidIndexes), 0);
      QCOMPARE(jtm.HintCastAndStudio(invalidIndexes, "learn from this sentence"), 0);
      QCOMPARE(jtm.FormatCast(invalidIndexes), 0);
      QCOMPARE(jtm.SyncFieldNameByJsonBaseName(invalidIndexes), 0);
      QCOMPARE(jtm.onRowsRemoved({}, nullptr), 0);
      QCOMPARE(jtm.onRowsRemoved({invalidIndex}, nullptr), 0);
      QCOMPARE(jtm.SaveCurrentChanges(invalidIndexes), 0);
      QCOMPARE(jtm.ExportCastStudioToLocalDictionaryFile(invalidIndexes), (std::pair<int, int>{-1, -1}));
      QCOMPARE(jtm.ExportCastStudioToLocalDictionaryFile({}), (std::pair<int, int>{0, 0}));
      QCOMPARE(jtm.AppendCastFromSentence(invalidIndex, "world AAA BB world", true), -1);
      QCOMPARE(jtm.AppendCastFromSentence(invalidIndex, "hello Aaa Bb world", false), -1);

      QCOMPARE(jtm.UpdateFizeSizeField({}), 0);
      QCOMPARE(jtm.UpdateDurationField({}), 0);
      QCOMPARE(jtm.UpdateMD5Field({}), 0);
      QCOMPARE(jtm.JsonFieldValueUpdateCore({QModelIndex{}}, JSON_KEY_E::Name), -1); // field cannot update
      QCOMPARE(jtm.JsonFieldValueUpdateCore({QModelIndex{}}, JSON_KEY_E::Detail), -1);
    }
  }

  void setRootPath_force_reload_ok() {
    constexpr char contentsAJson[]{R"({
    "Name": "GameTurbo - A rank - GGG YYYYY",
    "Studio": "Empty Studio A",
    "Cast": []
})"};
    constexpr char contentsBJson[]{R"({
    "Name": "GameTurbo - B rank - XX YY and ZZ DD EE",
    "Studio": "Empty Studio B",
    "Cast": []
})"};
    constexpr char contentsZJson[]{R"({
    "Name": "GameTurbo - Z rank - ActorZ",
    "Studio": "Empty Studio Z",
    "Cast": []
})"};

    const QString rootPath{mTDir.itemPath("set_root_path")};
    QList<FsNodeEntry> rootPathNodes{
        {"set_root_path/a.json", false, contentsAJson},
        {"set_root_path/b.json", false, contentsBJson},
    };
    QCOMPARE(mTDir.createEntries(rootPathNodes), 2);
    OnScopeExit {
      QVERIFY(QDir(mTDir.itemPath("set_root_path")).removeRecursively());
    };

    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(rootPath), 2);
    QCOMPARE(jtm.mRootPath, rootPath);
    QCOMPARE(jtm.rowCount(), 2);
    QCOMPARE(jtm.mCachedJsons.size(), 2);
    decltype(jtm.mCachedJsons) mCachedJsonsBackup = jtm.mCachedJsons;  // save backup here

    // 1.1 modify json file name
    {
      AutoRollbackFileContentModify autoRollbackContentMod(mTDir.itemPath("set_root_path/a.json"),  //
                                                           "GameTurbo - A rank - GGG YYYYY",        //
                                                           "GAMETURBO - A RANK - GGG YYYYY");       //
      QCOMPARE(autoRollbackContentMod.Execute(), true);
      // contains "\r\n" and not contain "\n"
      QVERIFY(mTDir.checkFileContents("set_root_path/a.json",                    //
                                      {"\n", "GAMETURBO - A RANK - GGG YYYYY"},  //
                                      {"\r\n"}));

      // 1.2 path unchange, skip directly, will not reload from file
      QCOMPARE(jtm.setRootPath(rootPath), 0);
      QCOMPARE(jtm.rowCount(), 2);                     // unchange at all
      QCOMPARE(jtm.mCachedJsons, mCachedJsonsBackup);  // unchange at all

      // 1.3 force reload will reload from file and update model data
      QCOMPARE(jtm.forceReloadPath(), 2);
      QCOMPARE(jtm.mRootPath, rootPath);
      QCOMPARE(jtm.rowCount(), 2);                      // still 2 file
      QVERIFY(jtm.mCachedJsons != mCachedJsonsBackup);  // will not same as initial

      QCOMPARE(jtm.data(jtm.index(0, JsonKey::Name), Qt::DisplayRole).toString(), "GAMETURBO - A RANK - GGG YYYYY");
    }

    // AutoRollbackFileContentModify rollback should be ok
    const QByteArray aJsonContents = mTDir.readByteArray("set_root_path/a.json", nullptr);
    QCOMPARE(aJsonContents, contentsAJson);

    // 2.1 create a new json file "z.json". after sort "z.json" is the last one row
    QVERIFY(mTDir.touch("set_root_path/z.json", contentsZJson));
    QCOMPARE(jtm.mRootPath, rootPath);
    QCOMPARE(jtm.forceReloadPath(), 3);
    QCOMPARE(jtm.rowCount(), 3);
    QCOMPARE(jtm.mCachedJsons.size(), 3);
    QCOMPARE(jtm.data(jtm.index(2, JsonKey::Name), Qt::DisplayRole).toString(), "GameTurbo - Z rank - ActorZ");

    QCOMPARE(jtm.m_modifiedRows.any(), false);  // force reload can be regard as model reset
  }

  void decoration_check_ok() {
    constexpr char decorationJson[]{R"({
    "Name": "Real Madrid - decoration - Cristiano Ronaldo, Leite Kaka",
    "Studio": "",
    "Cast": [],
    "ContentFixed": true,
    "Detail": ""
})"};
    studioHelper.setFileContents(
        "real madrid\tRealMadrid\n"
        "realmadrid\tRealMadrid\n",
        "");

    actorHelper.setFileContents(
        "cristiano ronaldo\n"
        "leite kaka\n",
        "");

    const QString rootPath{mTDir.itemPath("decoration_check")};
    mTDir.touch("decoration_check/decoration.json", decorationJson);
    OnScopeExit {
      QVERIFY(QDir(mTDir.itemPath("decoration_check")).removeRecursively());
    };

    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(rootPath), 1);
    QCOMPARE(jtm.rowCount(), 1);
    QModelIndex ind = jtm.index(0, JsonKey::ContentFixed);
    QCOMPARE(ind.siblingAtColumn(JsonKey::Name).data(Qt::DisplayRole).toString(), "Real Madrid - decoration - Cristiano Ronaldo, Leite Kaka");

    // unmark fixed: can init/hint/set/rmv/add/clear
    QCOMPARE(jtm.SetRecordContentsFixed({ind}, false), 1);
    {
      QVariant emptyVar = jtm.data(ind, Qt::DecorationRole);
      QVERIFY(!emptyVar.isValid());
    }
    QCOMPARE(jtm.InitCastAndStudio({ind}), 1);
    jtm.SetStudio({ind}, "");
    jtm.SetCastOrTags({ind}, JsonKey::Cast, "");
    QCOMPARE(jtm.HintCastAndStudio({ind}, "Real Madrid - decoration - Cristiano Ronaldo, Leite Kaka"), 2);  // both cast abd studio hint succeed
    QCOMPARE(ind.siblingAtColumn(JsonKey::Studio).data(Qt::DisplayRole).toString(), "RealMadrid");
    QVERIFY(jtm.data(ind.siblingAtColumn(JsonKey::Studio), Qt::ItemDataRole::ForegroundRole) != QColor{Qt::GlobalColor::darkRed});
    QCOMPARE(ind.siblingAtColumn(JsonKey::Cast).data(Qt::DisplayRole).toString(), "Cristiano Ronaldo,Leite Kaka");
    jtm.SetStudio({ind}, "");
    jtm.SetCastOrTags({ind}, JsonKey::Cast, "");
    jtm.SetCastOrTags({ind}, JsonKey::Tags, "");
    QCOMPARE(ind.siblingAtColumn(JsonKey::Studio).data(Qt::DisplayRole).toString(), "");
    QCOMPARE(ind.siblingAtColumn(JsonKey::Cast).data(Qt::DisplayRole).toString(), "");
    QCOMPARE(ind.siblingAtColumn(JsonKey::Tags).data(Qt::DisplayRole).toString(), "");
    // modified red
    QCOMPARE(jtm.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::ForegroundRole), QBrush(Qt::GlobalColor::red));

    // mark fixed: can init/hint/set/rmv/add/clear
    QCOMPARE(jtm.SetRecordContentsFixed({ind}, true), 1);
    {
      QVariant pixmapVar = jtm.data(ind, Qt::DecorationRole);
      QVERIFY(pixmapVar.isValid());
      QVERIFY(pixmapVar.canConvert<QPixmap>());
    }
    QCOMPARE(jtm.InitCastAndStudio({ind}), 0);
    jtm.SetStudio({ind}, "");
    jtm.SetCastOrTags({ind}, JsonKey::Cast, "");
    QCOMPARE(jtm.HintCastAndStudio({ind}, "Real Madrid - decoration - Cristiano Ronaldo,Leite Kaka"), 0);
    QCOMPARE(ind.siblingAtColumn(JsonKey::Studio).data(Qt::DisplayRole).toString(), "");
    QCOMPARE(ind.siblingAtColumn(JsonKey::Cast).data(Qt::DisplayRole).toString(), "");
    QCOMPARE(jtm.SetStudio({ind}, "Bayern"), 1);
    QCOMPARE(jtm.SetCastOrTags({ind}, JsonKey::Cast, "Muller"), 1);
    QCOMPARE(jtm.SetCastOrTags({ind}, JsonKey::Tags, "Football"), 1);
    QCOMPARE(ind.siblingAtColumn(JsonKey::Studio).data(Qt::DisplayRole).toString(), "Bayern");
    // modified red->saved->studio not in dictionary(dark red)
    QCOMPARE(jtm.data(ind.siblingAtColumn(JsonKey::Studio), Qt::ItemDataRole::ForegroundRole), QColor{Qt::GlobalColor::red});
    QCOMPARE(jtm.SaveCurrentChanges({ind}), 1);
    QCOMPARE(jtm.data(ind.siblingAtColumn(JsonKey::Studio), Qt::ItemDataRole::ForegroundRole), QColor{Qt::GlobalColor::darkRed});
    QCOMPARE(ind.siblingAtColumn(JsonKey::Cast).data(Qt::DisplayRole).toString(), "Muller");
    QCOMPARE(ind.siblingAtColumn(JsonKey::Tags).data(Qt::DisplayRole).toString(), "Football");
  }

  void update_duration_ok() {
    constexpr char durationJson[]{R"({
    "Name": "duration_test",
    "Studio": "",
    "Cast": [],
    "Duration": 0,
    "ContentFixed": true,
    "Detail": ""
})"};
    const QString rootPath{mTDir.itemPath("duration_check")};
    mTDir.touch("duration_check/duration_test.json", durationJson);
    mTDir.touch("duration_check/duration_test.mp4", "not zero size video");
    OnScopeExit {
      QVERIFY(QDir(mTDir.itemPath("duration_check")).removeRecursively());
    };

    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(rootPath), 1);
    QCOMPARE(jtm.rowCount(), 1);

    const QModelIndex ind = jtm.index(0, JsonKey::Duration);
    QCOMPARE(ind.siblingAtColumn(JsonKey::Size).data(Qt::DisplayRole).toString(), "0'0'0'0");
    QCOMPARE(jtm.UpdateFizeSizeField({ind}, 1), 1);
    QVERIFY(ind.siblingAtColumn(JsonKey::Size).data(Qt::DisplayRole).toString() != "0'0'0'0");

    MOCKER(VideoDurationGetter::ReadAVideo)
        .expects(exactly(1))
        .with(mTDir.itemPath("duration_check/duration_test.mp4"))
        .will(returnValue(10 * 60 * 1000));  // 10min
    QCOMPARE(ind.siblingAtColumn(JsonKey::Duration).data(Qt::DisplayRole).toString(), "00:00:00");
    QCOMPARE(jtm.UpdateDurationField({ind}, 1), 1);
    QCOMPARE(ind.siblingAtColumn(JsonKey::Duration).data(Qt::DisplayRole).toString(), "00:10:00");

    MOCKER(MD5Calculator::GetFileMD5).expects(exactly(1)).will(returnValue(QByteArray{"AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD"}));  // 10min
    QCOMPARE(ind.siblingAtColumn(JsonKey::MD5).data(Qt::DisplayRole).toString(), "");
    QCOMPARE(jtm.UpdateMD5Field({ind}, 1), 1);
    QCOMPARE(ind.siblingAtColumn(JsonKey::MD5).data(Qt::DisplayRole).toString(), "AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD");
  }

  void jsonFileProperty_retrieve_correct() {
    constexpr char contentsAJson[]{R"({
    "Name": "GameTurbo - A rank - GGG YYYYY",
    "Studio": "Empty Studio A",
    "Cast": [],
    "Detail": "This is just a json example."
})"};
    QList<FsNodeEntry> rootPathNodes{
        {"property_retrieve/a.json", false, contentsAJson},
    };
    QCOMPARE(mTDir.createEntries(rootPathNodes), 1);

    const QString rootPath{mTDir.itemPath("property_retrieve")};
    OnScopeExit {
      QVERIFY(QDir(mTDir.itemPath("property_retrieve")).removeRecursively());
    };

    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(rootPath), 1);
    QCOMPARE(jtm.rowCount(), 1);

    QModelIndex firstLineIndex{jtm.index(0, 0)};

    QFileInfo firstFi = jtm.fileInfo(firstLineIndex);
    QCOMPARE(firstFi.isFile(), true);
    QCOMPARE(firstFi.absoluteFilePath(), mTDir.itemPath("property_retrieve/a.json"));
    QCOMPARE(jtm.filePath(firstLineIndex), mTDir.itemPath("property_retrieve/a.json"));
    QCOMPARE(jtm.fileName(firstLineIndex), "a.json");
    QCOMPARE(jtm.fileBaseName(firstLineIndex), "a");
    QCOMPARE(jtm.absolutePath(firstLineIndex), rootPath);

    const QString fullInfoStr = jtm.fullInfo(firstLineIndex);
    QCOMPARE(fullInfoStr.contains("GameTurbo - A rank - GGG YYYYY"), true);
    QCOMPARE(fullInfoStr.contains("This is just a json example."), true);
  }

  void dataField_modify_correct() {
    studioHelper.setFileContents(
        "game turbo\tGameTurbo\n"
        "gameturbo\tGameTurbo\n",
        "");
    QCOMPARE(studioMgr.count(), 2);

    actorHelper.setFileContents(
        "ggg yyyyy\n"
        "xx yy\n"
        "zz dd ee\n",
        "");
    QCOMPARE(actorMgr.count(), 3);

    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mTDir.path()), mJsonsFileCountInitial);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);
    QCOMPARE(mJsonsFileCountInitial, 2);

    decltype(jtm.mCachedJsons) mCachedJsonsBackup = jtm.mCachedJsons;  // save backup here

    QModelIndex firstLineIndex{jtm.index(0, JsonKey::Name)};
    QModelIndex secondLineIndex{jtm.index(1, JsonKey::Name)};
    QCOMPARE(firstLineIndex.siblingAtColumn(JsonKey::Cast).isValid(), true);

    QModelIndexList valid2Indexes{firstLineIndex, secondLineIndex};
    {  // 1. cast and studio already has value. hint init get skipped
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Name), Qt::ItemDataRole::DisplayRole).toString(), "GameTurbo - A rank - GGG YYYYY");
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Cast), Qt::ItemDataRole::DisplayRole).toString().isEmpty(), false);
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Studio), Qt::ItemDataRole::DisplayRole).toString().isEmpty(), false);

      QCOMPARE(jtm.data(secondLineIndex.siblingAtColumn(JsonKey::Name), Qt::ItemDataRole::DisplayRole).toString(),
               "GameTurbo - B rank - XX YY and ZZ DD EE");
      QCOMPARE(jtm.data(secondLineIndex.siblingAtColumn(JsonKey::Cast), Qt::ItemDataRole::DisplayRole).toString().isEmpty(), false);
      QCOMPARE(jtm.data(secondLineIndex.siblingAtColumn(JsonKey::Studio), Qt::ItemDataRole::DisplayRole).toString().isEmpty(), false);

      QCOMPARE(jtm.InitCastAndStudio(valid2Indexes), 0);  // field Name/Cast/Studio already has value
      QCOMPARE(jtm.mCachedJsons, mCachedJsonsBackup);
      QCOMPARE(jtm.m_modifiedRows.any(), false);

      // clear this field
      QCOMPARE(jtm.SetStudio(valid2Indexes, ""), 2);
      QCOMPARE(jtm.SetCastOrTags(valid2Indexes, JSON_KEY_E::Cast, ""), 2);
      QCOMPARE(jtm.SetCastOrTags(valid2Indexes, JSON_KEY_E::Tags, ""), 2);

      // here 2 row affected.
      // but we don't assume what value it has been changed to.
      // because it depends on CastManager, StudiosManager Behavior
      QCOMPARE(jtm.InitCastAndStudio(valid2Indexes), 2);
      QVERIFY(jtm.mCachedJsons != mCachedJsonsBackup);
      QCOMPARE(jtm.mCachedJsons.front().m_Studio, "GameTurbo");
      QCOMPARE(jtm.mCachedJsons.front().m_Cast, (SortedUniqStrLst{QStringList{"GGG YYYYY"}}));

      QCOMPARE(jtm.mCachedJsons.back().m_Studio, "GameTurbo");
      QCOMPARE(jtm.mCachedJsons.back().m_Cast, (SortedUniqStrLst{QStringList{"XX YY", "ZZ DD EE"}}));
      QCOMPARE(jtm.m_modifiedRows.any(), true);
    }

    {  // 2. set directly succeed
      QCOMPARE(jtm.SetStudio(valid2Indexes, "Marvel"), 2);
      QCOMPARE(jtm.SetCastOrTags(valid2Indexes, JSON_KEY_E::Cast, "Chris Evans,Michael Fassbender"), 2);
      QCOMPARE(jtm.SetCastOrTags(valid2Indexes, JSON_KEY_E::Tags, "Documentary,Comic"), 2);
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Studio), Qt::ItemDataRole::DisplayRole).toString(), "Marvel");
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Cast), Qt::ItemDataRole::DisplayRole).toString(), "Chris Evans,Michael Fassbender");
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Tags), Qt::ItemDataRole::DisplayRole).toString(), "Comic,Documentary");

      // set again, skip unchange
      QCOMPARE(jtm.SetStudio(valid2Indexes, "Marvel"), 0);
      QCOMPARE(jtm.SetCastOrTags(valid2Indexes, JSON_KEY_E::Cast, "Chris Evans,Michael Fassbender"), 0);
      QCOMPARE(jtm.SetCastOrTags(valid2Indexes, JSON_KEY_E::Tags, "Documentary,Comic"), 0);

      // set to empty. also clear this field
      QCOMPARE(jtm.SetStudio(valid2Indexes, ""), 2);
      QCOMPARE(jtm.SetCastOrTags(valid2Indexes, JSON_KEY_E::Cast, ""), 2);
      QCOMPARE(jtm.SetCastOrTags(valid2Indexes, JSON_KEY_E::Tags, ""), 2);

      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Studio), Qt::ItemDataRole::DisplayRole).toString(), "");
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Cast), Qt::ItemDataRole::DisplayRole).toString(), "");
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Tags), Qt::ItemDataRole::DisplayRole).toString(), "");

      QCOMPARE(jtm.data(secondLineIndex.siblingAtColumn(JsonKey::Studio), Qt::ItemDataRole::DisplayRole).toString(), "");
      QCOMPARE(jtm.data(secondLineIndex.siblingAtColumn(JsonKey::Cast), Qt::ItemDataRole::DisplayRole).toString(), "");
      QCOMPARE(jtm.data(secondLineIndex.siblingAtColumn(JsonKey::Tags), Qt::ItemDataRole::DisplayRole).toString(), "");

      QCOMPARE(jtm.SetStudio(valid2Indexes, "Marvel"), 2);
    }

    {                                                                       // 3. AddCastOrTags succeed
      QCOMPARE(jtm.AddCastOrTags(valid2Indexes, JSON_KEY_E::Cast, ""), 0);  // empty skip

      QCOMPARE(jtm.AddCastOrTags(valid2Indexes, JSON_KEY_E::Cast, "Chris Evans,Michael Fassbender"), 2);
      QCOMPARE(jtm.AddCastOrTags(valid2Indexes, JSON_KEY_E::Tags, "Documentary,Comic"), 2);
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Cast), Qt::ItemDataRole::DisplayRole).toString(), "Chris Evans,Michael Fassbender");
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Tags), Qt::ItemDataRole::DisplayRole).toString(), "Comic,Documentary");

      // unchange
      QCOMPARE(jtm.AddCastOrTags(valid2Indexes, JSON_KEY_E::Cast, "Chris Evans,Michael Fassbender"), 0);  // affect none
      QCOMPARE(jtm.AddCastOrTags(valid2Indexes, JSON_KEY_E::Tags, "Documentary,Comic"), 0);

      // append
      QCOMPARE(jtm.AddCastOrTags(valid2Indexes, JSON_KEY_E::Cast, "Chris Evans,Michael Fassbender,Xander"), 2);  // affect 2 rows
      QCOMPARE(jtm.AddCastOrTags(valid2Indexes, JSON_KEY_E::Tags, "Documentary,Comic,Comedy"), 2);
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Cast), Qt::ItemDataRole::DisplayRole).toString(),
               "Chris Evans,Michael Fassbender,Xander");
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Tags), Qt::ItemDataRole::DisplayRole).toString(), "Comedy,Comic,Documentary");
    }

    mCachedJsonsBackup = jtm.mCachedJsons;  // save backup here
    {
      // rmv one element(full match) at one time ok.
      QCOMPARE(jtm.RmvCastOrTags(valid2Indexes, JSON_KEY_E::Cast, ""), 0);  // empty skip

      QCOMPARE(jtm.RmvCastOrTags(valid2Indexes, JSON_KEY_E::Cast, "Chris Evans,Michael Fassbender,Xander"), 0);
      QCOMPARE(jtm.RmvCastOrTags(valid2Indexes, JSON_KEY_E::Tags, "Documentary,Comic,Comedy"), 0);
      QCOMPARE(jtm.mCachedJsons, mCachedJsonsBackup);  // nothing changed

      QCOMPARE(jtm.RmvCastOrTags(valid2Indexes, JSON_KEY_E::Cast, "Chris Evans"), 2);  // affect 2 rows
      QCOMPARE(jtm.RmvCastOrTags(valid2Indexes, JSON_KEY_E::Tags, "Documentary"), 2);

      QCOMPARE(jtm.RmvCastOrTags(valid2Indexes, JSON_KEY_E::Cast, "Michael Fassbender"), 2);  // affect 2 rows
      QCOMPARE(jtm.RmvCastOrTags(valid2Indexes, JSON_KEY_E::Tags, "Comic"), 2);

      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Cast), Qt::ItemDataRole::DisplayRole).toString(), "Xander");
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Tags), Qt::ItemDataRole::DisplayRole).toString(), "Comedy");
    }

    {  // sync field will file name
      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Name), Qt::ItemDataRole::DisplayRole).toString(), "GameTurbo - A rank - GGG YYYYY");
      QCOMPARE(jtm.data(secondLineIndex.siblingAtColumn(JsonKey::Name), Qt::ItemDataRole::DisplayRole).toString(),
               "GameTurbo - B rank - XX YY and ZZ DD EE");
      jtm.SyncFieldNameByJsonBaseName(valid2Indexes);

      QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Name), Qt::ItemDataRole::DisplayRole).toString(), "a");   // a.json
      QCOMPARE(jtm.data(secondLineIndex.siblingAtColumn(JsonKey::Name), Qt::ItemDataRole::DisplayRole).toString(), "b");  // b.json
    }

    // above operation should not write into files
    {
      bool readAok = false;
      QCOMPARE(FileTool::TextReader(mTDir.itemPath("a.json"), &readAok), JSON_CONTENTS_A_RANK_IN_MODEL);
      QVERIFY(readAok);

      bool readBok = false;
      QCOMPARE(FileTool::TextReader(mTDir.itemPath("b.json"), &readBok), JSON_CONTENTS_B_RANK_IN_MODEL);
      QVERIFY(readBok);
    }

    // setData
    // Set/Init/Rmv Studio/Cast/Tags ok
    // Append
  }

  void append_cast_from_sentence_Ok() {
    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mTDir.path()), mJsonsFileCountInitial);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);
    QVERIFY(mJsonsFileCountInitial >= 2);

    decltype(jtm.mCachedJsons) mCachedJsonsBackup = jtm.mCachedJsons;  // save backup here

    QModelIndex firstLineIndex{jtm.index(0, JsonKey::Name)};

    QCOMPARE(jtm.AppendCastFromSentence(firstLineIndex, "Chris Evans & Jensen & Franko De Jong", false), 3);

    QCOMPARE(jtm.AppendCastFromSentence(firstLineIndex, "MARK LONG and CHRIS PINE", true), 2);

    const SortedUniqStrLst& sortUniqList = jtm.mCachedJsons[firstLineIndex.row()].m_Cast;
    const QStringList actualLst = sortUniqList.toSortedList();
    QCOMPARE(actualLst.contains("Chris Evans"), true);
    QCOMPARE(actualLst.contains("Jensen"), true);
    QCOMPARE(actualLst.contains("Franko De Jong"), true);
    QCOMPARE(actualLst.contains("Mark Long"), true);
    QCOMPARE(actualLst.contains("Chris Pine"), true);

    QVERIFY(jtm.mCachedJsons != mCachedJsonsBackup);
  }

  void hintCastStudio_singleWordActor_ok() {
    const char json0Contents[] = R"({
    "Cast": ["Not Correct"],
    "Name": "SingleWordStudio - scene name - actorA, actorB",
    "Studio": "NotCorrect"
})";
    const char json1Contents[] = R"({
    "Cast": [],
    "Name": "NotSingleWordStudio - other scene name - actor player1, actor player2, @actor",
    "Studio": "NotCorrect"
})";
    const char json2Contents[] = R"({
    "Cast": ["Not Correct"],
    "Name": "NotSingleWordStudio - scene name - actorA, actorB",
    "Studio": "NotCorrect"
})";
    // 厂商SingleWordStudio下的演员姓名仅有名
    // 厂商NotSingleWordStudio下的演员姓名完整
    QVERIFY(mTDir.touch("0.json", json0Contents));
    QVERIFY(mTDir.touch("1.json", json1Contents));
    QVERIFY(mTDir.touch("2.json", json2Contents));
    OnScopeExit {
      QVERIFY(QFile::remove(mTDir.itemPath("0.json")));
      QVERIFY(QFile::remove(mTDir.itemPath("1.json")));
      QVERIFY(QFile::remove(mTDir.itemPath("2.json")));
    };

    studioHelper.setFileContents(
        "not single word studio\tNotSingleWordStudio\n"
        "notsinglewordstudio\tNotSingleWordStudio\n"
        "single word studio\tSingleWordStudio\n"
        "singlewordstudio\tSingleWordStudio\n",
        "SingleWordStudio\n");

    actorHelper.setFileContents(
        "@actor\n"
        "actora\n"
        "actorb\n"
        "actor player1\n"
        "actor player2\n",
        "actora\n"
        "actorb\n");

    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mTDir.path()), mJsonsFileCountInitial + 3);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial + 3);

    QCOMPARE(jtm.mCachedJsons[0].m_Name, "SingleWordStudio - scene name - actorA, actorB");
    QCOMPARE(jtm.mCachedJsons[0].m_Studio, "NotCorrect");
    QCOMPARE(jtm.mCachedJsons[0].m_Cast.toSortedList(), (QStringList{"Not Correct"}));

    QCOMPARE(jtm.mCachedJsons[1].m_Name, "NotSingleWordStudio - other scene name - actor player1, actor player2, @actor");
    QCOMPARE(jtm.mCachedJsons[1].m_Studio, "NotCorrect");
    QCOMPARE(jtm.mCachedJsons[1].m_Cast.toSortedList(), (QStringList{}));

    QCOMPARE(jtm.mCachedJsons[2].m_Name, "NotSingleWordStudio - scene name - actorA, actorB");
    QCOMPARE(jtm.mCachedJsons[2].m_Studio, "NotCorrect");
    QCOMPARE(jtm.mCachedJsons[2].m_Cast.toSortedList(), (QStringList{"Not Correct"}));

    QModelIndex firstLineIndex{jtm.index(0, JsonKey::Name)};
    QModelIndex secondLineIndex{jtm.index(1, JsonKey::Name)};
    QModelIndex thirdLineIndex{jtm.index(2, JsonKey::Name)};
    QModelIndexList valid3Indexes{firstLineIndex, secondLineIndex, thirdLineIndex};
    jtm.HintCastAndStudio(valid3Indexes, "");

    // 单名的厂商, 演员可以一个单词
    QCOMPARE(jtm.mCachedJsons[0].m_Studio, "SingleWordStudio");
    QCOMPARE(jtm.mCachedJsons[0].m_Cast.toSortedList(), (QStringList{"Not Correct", "actorA", "actorB"}));
    // 姓+名完整的厂商, 演员>=2个单词, 或者单个单词, 但是含有特殊字符[0-9@_]
    QCOMPARE(jtm.mCachedJsons[1].m_Studio, "NotSingleWordStudio");
    QCOMPARE(jtm.mCachedJsons[1].m_Cast.toSortedList(), (QStringList{"@actor", "actor player1", "actor player2"}));
    QCOMPARE(jtm.mCachedJsons[2].m_Studio, "NotSingleWordStudio");
    QCOMPARE(jtm.mCachedJsons[2].m_Cast.toSortedList(), (QStringList{"Not Correct"}));
  }

  void hintCastStudio_Ok() {
    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mTDir.path()), mJsonsFileCountInitial);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);
    QVERIFY(mJsonsFileCountInitial >= 2);
    decltype(jtm.mCachedJsons) mCachedJsonsBackup = jtm.mCachedJsons;  // save backup here

    QCOMPARE(jtm.mCachedJsons[0].m_Name, "GameTurbo - A rank - GGG YYYYY");
    QCOMPARE(jtm.mCachedJsons[0].m_Studio, "Empty Studio A");
    QCOMPARE(jtm.mCachedJsons[0].m_Cast.toSortedList(), (QStringList{"Empty Cast A 1", "Empty Cast A 2"}));  // hintCast insert to m_Cast

    QCOMPARE(jtm.mCachedJsons[1].m_Name, "GameTurbo - B rank - XX YY and ZZ DD EE");
    QCOMPARE(jtm.mCachedJsons[1].m_Studio, "Empty Studio B");
    QCOMPARE(jtm.mCachedJsons[1].m_Cast.toSortedList(), (QStringList{"Empty Cast B 1", "Empty Cast B 2"}));  // hintCast insert to m_Cast

    actorHelper.setFileContents(
        "ggg yyyyy\n"
        "xx yy\n"
        "zz dd ee\n"
        "sentence name",
        "");
    studioHelper.setFileContents(
        "game turbo\tGameTurbo\n"
        "gameturbo\tGameTurbo\n",
        "");

    QModelIndex firstLineIndex{jtm.index(0, JsonKey::Name)};
    QModelIndex secondLineIndex{jtm.index(1, JsonKey::Name)};
    QModelIndexList valid2Indexes{firstLineIndex, secondLineIndex};
    QCOMPARE(jtm.SetStudio(valid2Indexes, ""), 2);
    QCOMPARE(jtm.SetCastOrTags(valid2Indexes, JSON_KEY_E::Cast, ""), 2);

    {
      // before color is not red
      const QVariant studioHintColorData = jtm.data(firstLineIndex.siblingAtColumn(JSON_KEY_E::Studio), Qt::ItemDataRole::ForegroundRole);
      const QVariant castHintColorData = jtm.data(firstLineIndex.siblingAtColumn(JSON_KEY_E::Cast), Qt::ItemDataRole::ForegroundRole);
      QCOMPARE(studioHintColorData.isNull(), true);
      QCOMPARE(castHintColorData.isNull(), true);
    }

    // 2 rows both cast and studio hint ok
    const QString selectedSentenceText = "hello SENTENCE FROM CAST there";
    QCOMPARE(jtm.HintCastAndStudio(valid2Indexes, selectedSentenceText), 4);
    QVERIFY(jtm.mCachedJsons != mCachedJsonsBackup);  // hint member changed

    QCOMPARE(jtm.mCachedJsons[0].hintStudio, "GameTurbo");
    QCOMPARE(jtm.mCachedJsons[0].m_Studio, "GameTurbo");  // hintStudio as the new m_studio
    QCOMPARE(jtm.mCachedJsons[0].hintCast, "GGG YYYYY");
    QCOMPARE(jtm.mCachedJsons[0].m_Cast.toSortedList(), (QStringList{"GGG YYYYY"}));  // hintCast insert to m_Cast

    QCOMPARE(jtm.mCachedJsons[1].hintStudio, "GameTurbo");
    QCOMPARE(jtm.mCachedJsons[1].hintCast.contains("XX YY"), true);
    QCOMPARE(jtm.mCachedJsons[1].hintCast.contains("ZZ DD EE"), true);
    QCOMPARE(jtm.mCachedJsons[1].m_Cast.toSortedList(), (QStringList{"XX YY", "ZZ DD EE"}));  // hintCast insert to m_Cast

    {
      const QVariant studioHintColorData = jtm.data(firstLineIndex.siblingAtColumn(JSON_KEY_E::Studio), Qt::ItemDataRole::ForegroundRole);
      const QVariant castHintColorData = jtm.data(firstLineIndex.siblingAtColumn(JSON_KEY_E::Cast), Qt::ItemDataRole::ForegroundRole);
      QCOMPARE(studioHintColorData.isValid(), true);
      QCOMPARE(castHintColorData.isValid(), true);
      QCOMPARE(studioHintColorData.canConvert<QColor>(), true);
      QCOMPARE(castHintColorData.canConvert<QColor>(), true);
      QCOMPARE(studioHintColorData.value<QColor>(), QColor(Qt::red));
      QCOMPARE(castHintColorData.value<QColor>(), QColor(Qt::red));
    }

    // hint again nothins update now
    QCOMPARE(jtm.HintCastAndStudio(valid2Indexes, "hello SENTENCE FROM CAST there"), 0);
  }

  void format_cast_sync_name_field_to_file_name_ok() {
    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mTDir.path()), mJsonsFileCountInitial);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);
    QVERIFY(mJsonsFileCountInitial >= 2);

    QModelIndex firstLineIndex{jtm.index(0, JsonKey::Name)};
    QModelIndex secondLineIndex{jtm.index(1, JsonKey::Name)};
    QModelIndexList valid2Indexes{firstLineIndex, secondLineIndex};

    decltype(jtm.mCachedJsons) mCachedJsonsBackup = jtm.mCachedJsons;  // save backup here
    QVERIFY(!jtm.m_modifiedRows.test(firstLineIndex.row()));
    QVERIFY(!jtm.m_modifiedRows.test(secondLineIndex.row()));

    QCOMPARE(jtm.setData(firstLineIndex.siblingAtColumn(JsonKey::Prepath), "any path cannot edit"), false);  // cannot edit prepath field
    QCOMPARE(jtm.setData(firstLineIndex.siblingAtColumn(JsonKey::Cast), "Xander,Chris Evans, Xander,Chris Evans"), true);
    QCOMPARE(jtm.setData(secondLineIndex.siblingAtColumn(JsonKey::Cast), "Xander,Chris Evans, Xander,Chris Evans"), true);

    // 1. format cast ok
    QCOMPARE(jtm.FormatCast(valid2Indexes), 2);
    QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Cast), Qt::ItemDataRole::DisplayRole).toString(), "Chris Evans,Xander");
    QVERIFY(jtm.mCachedJsons != mCachedJsonsBackup);

    // 2. SyncFieldNameByJsonBaseName ok
    QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Name), Qt::ItemDataRole::DisplayRole).toString(), "GameTurbo - A rank - GGG YYYYY");
    QCOMPARE(jtm.data(secondLineIndex.siblingAtColumn(JsonKey::Name), Qt::ItemDataRole::DisplayRole).toString(),
             "GameTurbo - B rank - XX YY and ZZ DD EE");
    QCOMPARE(jtm.SyncFieldNameByJsonBaseName(valid2Indexes), 2);
    QCOMPARE(jtm.data(firstLineIndex.siblingAtColumn(JsonKey::Name), Qt::ItemDataRole::DisplayRole).toString(), "a");
    QCOMPARE(jtm.data(secondLineIndex.siblingAtColumn(JsonKey::Name), Qt::ItemDataRole::DisplayRole).toString(), "b");
    QVERIFY(jtm.m_modifiedRows.test(firstLineIndex.row()));
    QVERIFY(jtm.m_modifiedRows.test(secondLineIndex.row()));
  }

  void save_changes_and_export_to_dict_correct() {
    // precondition 2
    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mTDir.path()), mJsonsFileCountInitial);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);
    QVERIFY(mJsonsFileCountInitial >= 2);
    QCOMPARE(jtm.m_modifiedRows.any(), false);

    QModelIndex firstLineIndex{jtm.index(0, JsonKey::Name)};
    QModelIndex secondLineIndex{jtm.index(1, JsonKey::Name)};
    QModelIndexList valid2Indexes{firstLineIndex, secondLineIndex};

    {
      // set a specified studio/cast
      QCOMPARE(jtm.SetStudio({firstLineIndex}, "GameTurboA"), 1);
      QCOMPARE(jtm.SetCastOrTags({firstLineIndex}, JSON_KEY_E::Cast, "Cast11,Cast12"), 1);
      QCOMPARE(jtm.SetStudio({secondLineIndex}, "GameTurboB"), 1);
      QCOMPARE(jtm.SetCastOrTags({secondLineIndex}, JSON_KEY_E::Cast, "Cast21,Cast22"), 1);
      // save changed to local file
      QCOMPARE(jtm.m_modifiedRows.any(), true);
      QCOMPARE(jtm.SaveCurrentChanges(valid2Indexes), 2);
      QCOMPARE(jtm.m_modifiedRows.any(), false);
      QCOMPARE(jtm.SaveCurrentChanges(valid2Indexes), 0);
      // above operation should write into files

      QVERIFY(mTDir.checkFileContents("a.json",                                         //
                                      QSet<QString>{"GameTurboA", "Cast11", "Cast12"},  //
                                      QSet<QString>{"GameTurboB", "Cast21", "Cast22"}));
      QVERIFY(mTDir.checkFileContents("b.json",                                         //
                                      QSet<QString>{"GameTurboB", "Cast21", "Cast22"},  //
                                      QSet<QString>{"GameTurboA", "Cast11", "Cast12"}));
    }

    {  // export cast/studio to dictionary
      QVERIFY(!studioMgr.ProStudioMap().contains("gameturboa"));
      QVERIFY(!studioMgr.ProStudioMap().contains("gameturboa"));

      QVERIFY(!actorMgr.CastSet().contains("cast11"));
      QVERIFY(!actorMgr.CastSet().contains("cast12"));
      QVERIFY(!actorMgr.CastSet().contains("cast21"));
      QVERIFY(!actorMgr.CastSet().contains("cast22"));

      jtm.ExportCastStudioToLocalDictionaryFile(valid2Indexes);

      QVERIFY(studioMgr.ProStudioMap().contains("gameturboa"));
      QVERIFY(studioMgr.ProStudioMap().contains("gameturboa"));
      QVERIFY(actorMgr.CastSet().contains("cast11"));
      QVERIFY(actorMgr.CastSet().contains("cast12"));
      QVERIFY(actorMgr.CastSet().contains("cast21"));
      QVERIFY(actorMgr.CastSet().contains("cast22"));
    }
  }

  void RenameJsonAndItsRelated_ok() {
    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mTDir.path()), mJsonsFileCountInitial);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);
    QVERIFY(mJsonsFileCountInitial >= 2);
    QCOMPARE(jtm.m_modifiedRows.any(), false);
    const auto rowElementsRmv = [&jtm](int beg, int end) { jtm.mCachedJsons.erase(jtm.mCachedJsons.begin() + beg, jtm.mCachedJsons.begin() + end); };
    {
      QModelIndex firstLineIndex{jtm.index(0, JsonKey::Name)};
      QSignalSpy rowsAboutToBeRemovedSig{&jtm, &JsonTableModel::rowsAboutToBeRemoved};
      QSignalSpy rowsRemovedSig{&jtm, &JsonTableModel::rowsRemoved};
      QCOMPARE(jtm.onRowsRemoved({firstLineIndex}, rowElementsRmv), 1);
      QCOMPARE(rowsAboutToBeRemovedSig.count(), 1);
      QCOMPARE(rowsRemovedSig.count(), 1);
      QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial - 1);
    }

    jtm.forceReloadPath();
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);

    {
      QModelIndex firstLineIndex{jtm.index(0, JsonKey::Name)};
      QModelIndex secondLineIndex{jtm.index(1, JsonKey::Name)};
      QCOMPARE(jtm.onRowsRemoved({firstLineIndex, secondLineIndex, firstLineIndex}, rowElementsRmv), 2);  // should remove duplicate
      QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial - 2);
    }

    jtm.forceReloadPath();
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);
  }

  void proxy_model_works() {
    QStandardItemModel sourceModel;
    ModelTestHelper::InitStdItemModel(sourceModel, "row%1-col%2", 3, 2);  // 3-by-2 matrix
    // rc00 rc01
    // rc10 rc11
    // rc20 rc21

    QSortFilterProxyModel proxyModel;
    proxyModel.setSourceModel(&sourceModel);
    proxyModel.setFilterKeyColumn(-1);  // all column

    proxyModel.setFilterFixedString("");  // empty filter. all pass
    QCOMPARE(proxyModel.rowCount(), 3);

    proxyModel.setFilterFixedString("row1");  // only one row contains row1
    QCOMPARE(proxyModel.rowCount(), 1);
    QModelIndex proxyIndex = proxyModel.index(0, 0);
    QString data = proxyModel.data(proxyIndex).toString();
    QVERIFY(data.startsWith("row1"));

    proxyModel.setFilterFixedString("col1");  // all 3 row contains col1 in first column
    QCOMPARE(proxyModel.rowCount(), 3);

    proxyModel.setFilterFixedString("ROW0");  // case sensitive
    QCOMPARE(proxyModel.rowCount(), 0);

    proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);  // case sensitive
    proxyModel.setFilterFixedString("ROW0");
    QCOMPARE(proxyModel.rowCount(), 1);

    proxyModel.setFilterCaseSensitivity(Qt::CaseSensitive);
    proxyModel.setFilterKeyColumn(1);         // only filter for second column
    proxyModel.setFilterFixedString("col0");  // col0 not in second column at all
    QCOMPARE(proxyModel.rowCount(), 0);       // no row match
  }
};

#include "JsonTableModelTest.moc"
REGISTER_TEST(JsonTableModelTest, false)
