#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "JsonTableModel.h"
#include "CastManager.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "JsonTestPrecoditionTools.h"
#include "PublicVariable.h"
#include "PublicTool.h"
#include "StudiosManager.h"
#include "ModelTestHelper.h"

using namespace JsonTestPrecoditionTools;

class JsonTableModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  const QString mWorkPath{tDir.path()};
  QDir mDir{mWorkPath, "", QDir::SortFlag::Name, QDir::Filter::Files};
  int mJsonsFileCountInitial = 0;

 private slots:
  void initTestCase() {
    const QList<FsNodeEntry> nodes{
        //
        {"a.json", false, JSON_CONTENTS_A_RANK_IN_MODEL},               //
        {"b.json", false, JSON_CONTENTS_B_RANK_IN_MODEL},               //
        {"jazz .txt", false, "don't care me here"},                     //
        {"a.mp4", false, "only care me in rename json related files"},  //
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());
    QVERIFY(tDir.IsValid());

    mDir.setNameFilters(TYPE_FILTER::JSON_TYPE_SET);
    mJsonsFileCountInitial = mDir.entryList().size();
    QVERIFY(mJsonsFileCountInitial >= 2);  // at least two json file exist here
  }

  void CheckModelData(JsonTableModel& model) {
    QCOMPARE(model.data(model.index(0, JsonKey::Name), Qt::DisplayRole).toString(), "GameTurbo - A rank - GGG YYYYY");
    QCOMPARE(model.data(model.index(0, JsonKey::Cast), Qt::DisplayRole).toString(), "Empty Cast A 1,Empty Cast A 2");
    QCOMPARE(model.data(model.index(0, JsonKey::Studio), Qt::DisplayRole).toString(), "Empty Studio A");
    QCOMPARE(model.data(model.index(0, JsonKey::Tags), Qt::DisplayRole).toString(), "Empty Tag A");
    QCOMPARE(model.data(model.index(0, JsonKey::Detail), Qt::DisplayRole).toString(), "This is just a json example.");
    QCOMPARE(model.data(model.index(0, JsonKey::Duration), Qt::DisplayRole).toInt(), 36000);

    QCOMPARE(model.data(model.index(1, JsonKey::Name), Qt::DisplayRole).toString(), "GameTurbo - B rank - XX YY and ZZ DD EE");
    QCOMPARE(model.data(model.index(1, JsonKey::Cast), Qt::DisplayRole).toString(), "Empty Cast B 1,Empty Cast B 2");
    QCOMPARE(model.data(model.index(1, JsonKey::Studio), Qt::DisplayRole).toString(), "Empty Studio B");
    QCOMPARE(model.data(model.index(1, JsonKey::Tags), Qt::DisplayRole).toString(), "Empty Tag B");
    QCOMPARE(model.data(model.index(1, JsonKey::Detail), Qt::DisplayRole).toString(), "This is just b json example.");
    QCOMPARE(model.data(model.index(1, JsonKey::Duration), Qt::DisplayRole).toInt(), 3600);
  }

  void initalize_ok() {
    JsonTableModel jtm;
    QCOMPARE(jtm.rowCount(), 0);
    QCOMPARE(jtm.columnCount(), JsonKey::JSON_TABLE_HEADERS_COUNT);
    QVERIFY(1 <= JsonKey::JSON_TABLE_HEADERS_COUNT && JsonKey::JSON_TABLE_HEADERS_COUNT <= 999);
    QCOMPARE(jtm.headerData(0, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(), JsonKey::JSON_TABLE_HEADERS[0]);
    QCOMPARE(jtm.headerData(999, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toInt(), 999 + 1);
    QCOMPARE(jtm.headerData(0, Qt::Horizontal, Qt::ItemDataRole::TextAlignmentRole).isNull(), true);

    QCOMPARE(jtm.headerData(3, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 3 + 1);  // not depend on array, never out of
    QCOMPARE(jtm.headerData(0, Qt::Vertical, Qt::ItemDataRole::TextAlignmentRole).toInt(), ((int)Qt::AlignRight));
    QCOMPARE(jtm.headerData(102400000, Qt::Orientation::Vertical, Qt::ItemDataRole::ForegroundRole).isNull(), true);

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
      QCOMPARE(jtm.SetCastOrTags(invalidIndexes, JSON_KEY_E::Cast, "learn from this sentence"), 0);
      QCOMPARE(jtm.AddCastOrTags(invalidIndexes, JSON_KEY_E::Cast, "learn from this sentence"), 0);
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
      QCOMPARE(jtm.RenameJsonAndItsRelated(invalidIndex, "correct name.json"), 0);
      QCOMPARE(jtm.SaveCurrentChanges(invalidIndexes), 0);
      QCOMPARE(jtm.ExportCastStudioToLocalDictionaryFile(invalidIndexes), (std::pair<int, int>{-1, -1}));
      QCOMPARE(jtm.ExportCastStudioToLocalDictionaryFile({}), (std::pair<int, int>{0, 0}));
      QCOMPARE(jtm.AppendCastFromSentence(invalidIndex, "world AAA BB world", true), -1);
      QCOMPARE(jtm.AppendCastFromSentence(invalidIndex, "hello Aaa Bb world", false), -1);
    }
  }

  void setRootPath_force_reload_ok() {
    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mWorkPath), mJsonsFileCountInitial);
    QCOMPARE(jtm.mRootPath, mWorkPath);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);  // 2 file
    QCOMPARE(jtm.mCachedJsons.size(), jtm.rowCount());
    CheckModelData(jtm);
    decltype(jtm.mCachedJsons) mCachedJsonsBackup = jtm.mCachedJsons;  // save backup here

    {
      // 1.1 modify json file here
      AutoRollbackFileContentModify autoRollbackContentMod(tDir.itemPath("a.json"),            //
                                                           "GameTurbo - A rank - GGG YYYYY",   //
                                                           "GAMETURBO - A RANK - GGG YYYYY");  //
      QCOMPARE(autoRollbackContentMod.Execute(), true);

      // 1.2 path unchange, skip directly, will not reload from file
      QCOMPARE(jtm.setRootPath(mWorkPath), 0);
      QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);  // unchange at all
      QCOMPARE(jtm.mCachedJsons, mCachedJsonsBackup);    // unchange at all

      // 1.3 force reload will reload from file and update model data
      QCOMPARE(jtm.forceReloadPath(), mJsonsFileCountInitial);
      QCOMPARE(jtm.mRootPath, mWorkPath);
      QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);  // still 2 file
      QVERIFY(jtm.mCachedJsons != mCachedJsonsBackup);   // will not same as initial

      QCOMPARE(jtm.data(jtm.index(0, JsonKey::Name), Qt::DisplayRole).toString(), "GAMETURBO - A RANK - GGG YYYYY");
      // 1.4 json contents rollback
    }

    {
      // 2.1 create a new json file
      const QList<FsNodeEntry> extraNode{
          //
          {"c new added.json", false, JSON_CONTENTS_A_RANK_IN_MODEL},  //
      };
      QCOMPARE(tDir.createEntries(extraNode), extraNode.size());
      ON_SCOPE_EXIT {
        QVERIFY(mDir.remove("c new added.json"));
      };
      QCOMPARE(jtm.mRootPath, mWorkPath);
      QCOMPARE(jtm.forceReloadPath(), mJsonsFileCountInitial + 1);
      QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial + 1);               // still 2 file
      QVERIFY(jtm.mCachedJsons.size() == mCachedJsonsBackup.size() + 1);  // will not same as initial
    }

    QCOMPARE(jtm.m_modifiedRows.any(), false);  // force reload can be regard as model reset
  }

  void jsonFileProperty_retrieve_correct() {
    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mWorkPath), mJsonsFileCountInitial);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);

    QVERIFY(mJsonsFileCountInitial >= 2);
    QModelIndex firstLineIndex{jtm.index(0, 0)};

    QFileInfo firstFi = jtm.fileInfo(firstLineIndex);
    QCOMPARE(firstFi.isFile(), true);
    QCOMPARE(firstFi.absoluteFilePath(), tDir.itemPath("a.json"));
    QCOMPARE(jtm.filePath(firstLineIndex), tDir.itemPath("a.json"));
    QCOMPARE(jtm.fileName(firstLineIndex), "a.json");
    QCOMPARE(jtm.fileBaseName(firstLineIndex), "a");
    QCOMPARE(jtm.absolutePath(firstLineIndex), tDir.path());
    const QString fullInfoStr = jtm.fullInfo(firstLineIndex);
    QCOMPARE(fullInfoStr.contains("GameTurbo - A rank - GGG YYYYY"), true);
    QCOMPARE(fullInfoStr.contains("This is just a json example."), true);
  }

  void dataField_modify_correct() {
    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mWorkPath), mJsonsFileCountInitial);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);
    QVERIFY(mJsonsFileCountInitial >= 2);

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

      QCOMPARE(jtm.InitCastAndStudio(valid2Indexes), 0);  // get skipped
      QCOMPARE(jtm.mCachedJsons, mCachedJsonsBackup);
      QCOMPARE(jtm.m_modifiedRows.any(), false);

      // clear this field
      QCOMPARE(jtm.SetStudio(valid2Indexes, ""), 2);
      QCOMPARE(jtm.SetCastOrTags(valid2Indexes, JSON_KEY_E::Cast, ""), 2);
      QCOMPARE(jtm.SetCastOrTags(valid2Indexes, JSON_KEY_E::Tags, ""), 2);

      // here 2 row affected. but we don't assume what value it has been changed to. because it depends on CastManager, StudiosManager Behavior
      QCOMPARE(jtm.InitCastAndStudio(valid2Indexes), 2);
      QVERIFY(jtm.mCachedJsons != mCachedJsonsBackup);
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
      QCOMPARE(FileTool::TextReader(tDir.itemPath("a.json"), &readAok), JSON_CONTENTS_A_RANK_IN_MODEL);
      QVERIFY(readAok);

      bool readBok = false;
      QCOMPARE(FileTool::TextReader(tDir.itemPath("b.json"), &readBok), JSON_CONTENTS_B_RANK_IN_MODEL);
      QVERIFY(readBok);
    }

    // setData
    // Set/Init/Rmv Studio/Cast/Tags ok
    // Append
  }

  void append_cast_from_sentence_Ok() {
    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mWorkPath), mJsonsFileCountInitial);
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

  void hintCastStudioOk() {
    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mWorkPath), mJsonsFileCountInitial);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);
    QVERIFY(mJsonsFileCountInitial >= 2);

    QModelIndex firstLineIndex{jtm.index(0, JsonKey::Name)};
    QModelIndex secondLineIndex{jtm.index(1, JsonKey::Name)};
    QModelIndexList valid2Indexes{firstLineIndex, secondLineIndex};

    decltype(jtm.mCachedJsons) mCachedJsonsBackup = jtm.mCachedJsons;  // save backup here

    auto& castInst = CastManager::getInst();
    const CAST_MGR_DATA_T castDataBkp = castInst.CastSet();
    castInst.CastSet() = CAST_MGR_DATA_T{"ggg yyyyy", "xx yy", "zz dd ee", "sentence name"};
    ON_SCOPE_EXIT {
      castInst.CastSet() = castDataBkp;
    };

    QCOMPARE(jtm.SetStudio(valid2Indexes, ""), 2);
    QCOMPARE(jtm.SetCastOrTags(valid2Indexes, JSON_KEY_E::Cast, ""), 2);

    {  // before color is not red
      const QVariant studioHintColorData = jtm.data(firstLineIndex.siblingAtColumn(JSON_KEY_E::Studio), Qt::ItemDataRole::ForegroundRole);
      const QVariant castHintColorData = jtm.data(firstLineIndex.siblingAtColumn(JSON_KEY_E::Cast), Qt::ItemDataRole::ForegroundRole);
      QCOMPARE(studioHintColorData.isNull(), true);
      QCOMPARE(castHintColorData.isNull(), true);
    }

    QCOMPARE(jtm.HintCastAndStudio(valid2Indexes, "hello SENTENCE FROM CAST there"), 4);  // 2 row both cast and studio hint ok

    QVERIFY(jtm.mCachedJsons != mCachedJsonsBackup);  // hint member changed

    QCOMPARE(jtm.mCachedJsons[0].hintStudio, "GameTurbo");
    QCOMPARE(jtm.mCachedJsons[1].hintStudio, "GameTurbo");
    QCOMPARE(jtm.mCachedJsons[0].m_Studio, "GameTurbo");  // hintStudio as the new m_studio

    QCOMPARE(jtm.mCachedJsons[0].hintCast, "GGG YYYYY");
    QCOMPARE(jtm.mCachedJsons[0].m_Cast.toSortedList(), (QStringList{"GGG YYYYY"}));  // hintCast insert to m_Cast

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

    QCOMPARE(jtm.HintCastAndStudio(valid2Indexes, "hello SENTENCE FROM CAST there"), 0);  // nothins update now
  }

  void format_cast_sync_name_field_to_file_name_ok() {
    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mWorkPath), mJsonsFileCountInitial);
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
    QCOMPARE(jtm.setRootPath(mWorkPath), mJsonsFileCountInitial);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);
    QVERIFY(mJsonsFileCountInitial >= 2);
    QCOMPARE(jtm.m_modifiedRows.any(), false);

    QModelIndex firstLineIndex{jtm.index(0, JsonKey::Name)};
    QModelIndex secondLineIndex{jtm.index(1, JsonKey::Name)};
    QModelIndexList valid2Indexes{firstLineIndex, secondLineIndex};

    {
      // set a specified studio/cast
      QCOMPARE(jtm.SetStudio({firstLineIndex}, "GameTurboA"), 1);
      QCOMPARE(jtm.SetStudio({secondLineIndex}, "GameTurboB"), 1);
      QCOMPARE(jtm.SetCastOrTags({firstLineIndex}, JSON_KEY_E::Cast, "Cast11,Cast12"), 1);
      QCOMPARE(jtm.SetCastOrTags({secondLineIndex}, JSON_KEY_E::Cast, "Cast21,Cast22"), 1);
      // save changed to local file
      QCOMPARE(jtm.m_modifiedRows.any(), true);
      QCOMPARE(jtm.SaveCurrentChanges(valid2Indexes), 2);
      QCOMPARE(jtm.m_modifiedRows.any(), false);
      // above operation should write into files

      bool readAok = false;
      QVERIFY(FileTool::TextReader(tDir.itemPath("a.json"), &readAok) != JSON_CONTENTS_A_RANK_IN_MODEL);
      QVERIFY(readAok);

      bool readBok = false;
      QVERIFY(FileTool::TextReader(tDir.itemPath("b.json"), &readBok) != JSON_CONTENTS_B_RANK_IN_MODEL);
      QVERIFY(readBok);
    }

    {  // export cast/studio to dictionary
      const StudiosManager& studioInst = StudiosManager::getInst();
      QVERIFY(!studioInst.ProStudioMap().contains("gameturboa"));
      QVERIFY(!studioInst.ProStudioMap().contains("gameturboa"));

      const CastManager& castInst = CastManager::getInst();
      QVERIFY(!castInst.CastSet().contains("cast11"));
      QVERIFY(!castInst.CastSet().contains("cast12"));
      QVERIFY(!castInst.CastSet().contains("cast21"));
      QVERIFY(!castInst.CastSet().contains("cast22"));

      jtm.ExportCastStudioToLocalDictionaryFile(valid2Indexes);

      QVERIFY(studioInst.ProStudioMap().contains("gameturboa"));
      QVERIFY(studioInst.ProStudioMap().contains("gameturboa"));
      QVERIFY(castInst.CastSet().contains("cast11"));
      QVERIFY(castInst.CastSet().contains("cast12"));
      QVERIFY(castInst.CastSet().contains("cast21"));
      QVERIFY(castInst.CastSet().contains("cast22"));
    }
  }

  void RenameJsonAndItsRelated_ok() {
    JsonTableModel jtm;
    QCOMPARE(jtm.setRootPath(mWorkPath), mJsonsFileCountInitial);
    QCOMPARE(jtm.rowCount(), mJsonsFileCountInitial);
    QVERIFY(mJsonsFileCountInitial >= 2);
    QCOMPARE(jtm.m_modifiedRows.any(), false);

    QModelIndex firstLineIndex{jtm.index(0, JsonKey::Name)};
    QCOMPARE(jtm.RenameJsonAndItsRelated(firstLineIndex, "GameTurbo - A rank - GGG YYYYY"), 2);  // two file renamed

    const QStringList actualFileList = tDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name);
    const QStringList expectFileList = {"GameTurbo - A rank - GGG YYYYY.json", "GameTurbo - A rank - GGG YYYYY.mp4", "b.json", "jazz .txt"};
    QCOMPARE(expectFileList, actualFileList);
  }

  void proxy_model_works() {
    QStandardItemModel sourceModel;
    ModelTestHelper::InitStdItemModel(sourceModel, "row%1-col%2", 3, 2); // 3-by-2 matrix
    // rc00 rc01
    // rc10 rc11
    // rc20 rc21

    QSortFilterProxyModel proxyModel;
    proxyModel.setSourceModel(&sourceModel);
    proxyModel.setFilterKeyColumn(-1); // all column

    proxyModel.setFilterFixedString(""); // empty filter. all pass
    QCOMPARE(proxyModel.rowCount(), 3);

    proxyModel.setFilterFixedString("row1"); // only one row contains row1
    QCOMPARE(proxyModel.rowCount(), 1);
    QModelIndex proxyIndex = proxyModel.index(0, 0);
    QString data = proxyModel.data(proxyIndex).toString();
    QVERIFY(data.startsWith("row1"));

    proxyModel.setFilterFixedString("col1"); // all 3 row contains col1 in first column
    QCOMPARE(proxyModel.rowCount(), 3);

    proxyModel.setFilterFixedString("ROW0"); // case sensitive
    QCOMPARE(proxyModel.rowCount(), 0);

    proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive); // case sensitive
    proxyModel.setFilterFixedString("ROW0");
    QCOMPARE(proxyModel.rowCount(), 1);

    proxyModel.setFilterCaseSensitivity(Qt::CaseSensitive);
    proxyModel.setFilterKeyColumn(1); // only filter for second column
    proxyModel.setFilterFixedString("col0"); // col0 not in second column at all
    QCOMPARE(proxyModel.rowCount(), 0); // no row match
  }
};

#include "JsonTableModelTest.moc"
REGISTER_TEST(JsonTableModelTest, false)
