#include <QCoreApplication>
#include <QtTest>
#include <QSqlRecord>
#include <QSqlField>
#include "PlainTestSuite.h"
#include "CastBaseDb.h"
#include "TDir.h"
#include "TableFields.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "StringTool.h"
#include "CastPsonFileHelper.h"
#include "SqlRecordTestHelper.h"
#include <QMap>

using namespace SqlRecordTestHelper;
using namespace CastPsonFileHelper;
struct CastStructureProperty {
  QString ori;
  QSet<QString> imgsStr;
  int rate;
};

class CastBaseDbTest : public PlainTestSuite {
  Q_OBJECT
public:
  CastBaseDbTest() : PlainTestSuite{} {}
  TDir mDir;
  QString dbName{mDir.itemPath("PERF.db")};
  QString imgHostPath{mDir.path()};

  /* file system structure
root: PerfImgHost, subitems as follow
Music
  Ricky Martin
    Ricky Martin.jpg
    Ricky Martin.pson 10
Movie
  Chris Evans
    Chris Evans.jpg
    Chris Evans 2.jpg
    Chris Evans.pson   9
  Huge Jackman
    Huge Jackman.jpg
    Huge Jackman.pson  8
  Kaka
    Kaka.jpg
    Kaka.pson          10
escape folder
  not allowed img.jpg
*/
  const QList<FsNodeEntry> mNodeEntries//
      {
       {"Music/Ricky Martin/Ricky Martin.jpg", false, ""},
       {"Music/Ricky Martin/Ricky Martin.pson", false, CastValues2PsonStr("Ricky Martin", 10, "", "", "Music", -1, -1, "", "", "Ricky Martin.jpg", "")},

       {"Movie/Chris Evans/Chris Evans.jpg", false, ""},
       {"Movie/Chris Evans/Chris Evans 2.jpg", false, ""},
       {"Movie/Chris Evans/Chris Evans.pson", false, CastValues2PsonStr("Chris Evans", 9, "", "", "Movie", -1, -1, "", "", "Chris Evans 2.jpg\nChris Evans.jpg", "")},

       {"Movie/Huge Jackman/Huge Jackman.jpg", false, ""},
       {"Movie/Huge Jackman/Huge Jackman.pson", false, CastValues2PsonStr("Huge Jackman", 8, "", "", "Movie", -1, -1, "", "", "Huge Jackman.jpg", "")},

       {"Movie/Kaka/Kaka.jpg", false, ""},
       {"Movie/Kaka/Kaka.pson", false, CastValues2PsonStr("Kaka", 10, "", "", "Movie", -1, -1, "", "", "Kaka.jpg", "")},

       {"escape folder/not allowed img.jpg", false, ""},
       };

  const QMap<QString, CastStructureProperty> gExpectName2Property //
      {
       {"Kaka", {"Movie", QSet<QString>{"Kaka.jpg"}, 10}},
       {"Chris Evans", {"Movie", QSet<QString>{"Chris Evans.jpg", "Chris Evans 2.jpg"}, 9}},
       {"Huge Jackman", {"Movie", QSet<QString>{"Huge Jackman.jpg"}, 8}},
       {"Ricky Martin", {"Music", QSet<QString>{"Ricky Martin.jpg"}, 10}},
       };

  const QString mPerfAkaMultiLineText {
      "Kaka|Ricardo Izecson dos Santos Leite\n"
      "Chris Evans,Cevans and Christopher Robert Evans\n"
      "Huge Jackman, Wolverine\n"
      "Ricky Martin, Enrique Martin Morales"
  };
  // itself
  const QMap<QString, QString> mPerf2AkaMaps // 4
      { //
       {"Kaka", "Ricardo Izecson dos Santos Leite"},
       {"Chris Evans", "Cevans,Christopher Robert Evans"},
       {"Huge Jackman", "Wolverine"},
       {"Ricky Martin", "Enrique Martin Morales"},
       };
  // others
  const QString mPerfOtherOrNoneAkaMultiLineText {
      "Chris Evans, Captain America\n"
      "Huge Jackman\n"
      "Ricky Martin, Enrique Martin Morales\n"
      "Jake Gyllenhaal,Jacob Benjamin Gyllenhaal\n"
      "Brad Pitt,William Bradley Pitt\n"
      "Frank Grillo,Crossbones"
  };
  const QMap<QString, QString> mPerf2OtherAkaMaps // 6
      { //
          {"Chris Evans", "Captain America"},
          {"Huge Jackman", ""},
          {"Ricky Martin", "Enrique Martin Morales"},
          {"Jake Gyllenhaal", "Jacob Benjamin Gyllenhaal"},
          {"Brad Pitt", "William Bradley Pitt"},
          {"Frank Grillo", "Crossbones"}
      };
private slots:
  void initTestCase() {
    QVERIFY(mDir.IsValid());
    QCOMPARE(mDir.createEntries(mNodeEntries), mNodeEntries.size());
  }

  void test_aka_name_split() {
    const auto& noPerfFromEmptyText = CastBaseDb::GetFreqName2AkaNames("");
    QVERIFY(noPerfFromEmptyText.isEmpty());

    const auto& noPerfFromUselessText = CastBaseDb::GetFreqName2AkaNames("\n\n\n");
    QVERIFY(noPerfFromUselessText.isEmpty());

    const QMap<QString, QString> actualPerf2AkaMaps = CastBaseDb::GetFreqName2AkaNames(mPerfAkaMultiLineText);
    QCOMPARE(actualPerf2AkaMaps, mPerf2AkaMaps);

    const QMap<QString, QString> actualPerfs2OtherAkaMaps = CastBaseDb::GetFreqName2AkaNames(mPerfOtherOrNoneAkaMultiLineText);
    QCOMPARE(actualPerfs2OtherAkaMaps, mPerf2OtherAkaMaps);
  }

  void test_ReadFromUserInputSentence() {
    // precondition
    if (QFile{dbName}.exists()) {
      QVERIFY(QFile{dbName}.remove());
    }

    CastBaseDb perfDb{dbName, "PERF_CONNECTION"};
    QVERIFY(perfDb.CreateDatabase());
    QVERIFY(perfDb.CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(QFile::exists(dbName));

    // procedure
    // 1. add from 4 lines text
    QCOMPARE(perfDb.AppendCastFromMultiLineInput(mPerfAkaMultiLineText), 4);
    // 2. add from other 6 lines text
    QCOMPARE(perfDb.AppendCastFromMultiLineInput(mPerfOtherOrNoneAkaMultiLineText), 6);
    // INSERT INTO ON CONFLICT SET used in AppendCastFromMultiLineInput when PRIMARY KEY CONFLICTS, aka get updated

    const auto& oldMap = mPerf2AkaMaps;
    const auto& newMap = mPerf2OtherAkaMaps;

    QList<QSqlRecord> records;
    QVERIFY(perfDb.QueryForTest("SELECT * from " + DB_TABLE::PERFORMERS, records));
    for (const auto& record : records) {
      const QString castName{record.value(PERFORMER_DB_HEADER_KEY::Name).toString()};
      const QString akaNames{record.value(PERFORMER_DB_HEADER_KEY::ALIAS).toString()};
      if (newMap.contains(castName)) {
        QCOMPARE(newMap[castName], akaNames);
      } else if (oldMap.contains(castName)) {  // only newMap not exists this one, we consider oldMap
        QCOMPARE(oldMap[castName], akaNames);
      } else {
        QVERIFY2(false, qPrintable(castName));
      }
    }
  }

  void test_ReadFromImageHost() { // from local file system structure
    // precondition
    if (QFile{dbName}.exists()) {
      QVERIFY(QFile{dbName}.remove());
    }
    CastBaseDb perfDb{dbName, "PERF_CONNECTION"};
    QVERIFY(perfDb.CreateDatabase());
    QVERIFY(perfDb.CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(QFile::exists(dbName));

    // procedure
    // "escape folder" should not in table
    QCOMPARE(perfDb.ReadFromImageHost(imgHostPath), 4);
    QList<QSqlRecord> records;
    QVERIFY(perfDb.QueryForTest("SELECT * from " + DB_TABLE::PERFORMERS, records));
    QCOMPARE(records.size(), 4);

    for (const auto& record : records) {
      const QString castName = record.value(PERFORMER_DB_HEADER_KEY::Name).toString();
      auto it = gExpectName2Property.find(castName);
      QVERIFY2(it != gExpectName2Property.cend(), qPrintable(castName));

      QCOMPARE(record.value(PERFORMER_DB_HEADER_KEY::Ori).toString(), it->ori);

      const QStringList actualImgsLst = record.value(PERFORMER_DB_HEADER_KEY::Imgs).toString().split(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR);
      QSet<QString> actualImgsSet{actualImgsLst.cbegin(), actualImgsLst.cend()};
      QCOMPARE(actualImgsSet, it->imgsStr);
    }
  }

  void test_LoadFromPJsonFile() { // from local file system pson file
    // precondition
    if (QFile{dbName}.exists()) {
      QVERIFY(QFile{dbName}.remove());
    }
    CastBaseDb perfDb{dbName, "PERF_CONNECTION"};
    QVERIFY(perfDb.CreateDatabase());
    QVERIFY(perfDb.CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(QFile::exists(dbName));

    // procedure
    QCOMPARE(perfDb.LoadFromPsonFile(imgHostPath), 4);
    // load again, replace into will always ok
    QCOMPARE(perfDb.LoadFromPsonFile(imgHostPath), 4);

    QSet<QString> pkNames;
    {
      using namespace PERFORMER_DB_HEADER_KEY;
      QVERIFY(perfDb.QueryPK(DB_TABLE::PERFORMERS, ENUM_2_STR(Name), pkNames));
    }
    QSet<QString> expectNames{"Kaka", "Chris Evans", "Huge Jackman", "Ricky Martin"};
    QCOMPARE(pkNames, expectNames);

    QList<QSqlRecord> records;
    QVERIFY(perfDb.QueryForTest("SELECT * from " + DB_TABLE::PERFORMERS, records));
    QCOMPARE(records.size(), 4);

    for (const auto& record : records) {
      const QString castName = record.value(PERFORMER_DB_HEADER_KEY::Name).toString();
      auto it = gExpectName2Property.find(castName);
      QVERIFY2(it != gExpectName2Property.cend(), qPrintable(castName));

      QCOMPARE(record.value(PERFORMER_DB_HEADER_KEY::Ori).toString(), it->ori);

      const QStringList actualImgsLst = record.value(PERFORMER_DB_HEADER_KEY::Imgs).toString().split(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR);
      QSet<QString> actualImgsSet{actualImgsLst.cbegin(), actualImgsLst.cend()};
      QCOMPARE(actualImgsSet, it->imgsStr);

      QCOMPARE(record.value(PERFORMER_DB_HEADER_KEY::Rate).toInt(), it->rate);
    }
  }

  void test_UpdateRecordImgsField() {
    const QString defaultImgs{"Chris Hemsworth 0.jpg\nChris Hemsworth 1.jpg"};

    using namespace PERFORMER_DB_HEADER_KEY;
    // 1. folder not exist should skip update
    QSqlRecord chrisHenmworthRecord = GetACastRecordLine("Chris Hemsworth", "Movie", defaultImgs);
    QCOMPARE(chrisHenmworthRecord.value(PERFORMER_DB_HEADER_KEY::Name).toString(), "Chris Hemsworth");
    QCOMPARE(chrisHenmworthRecord.value(PERFORMER_DB_HEADER_KEY::Ori).toString(), "Movie");
    QCOMPARE(chrisHenmworthRecord.value(ENUM_2_STR(Name)).toString(), "Chris Hemsworth");
    QCOMPARE(chrisHenmworthRecord.value(ENUM_2_STR(Ori)).toString(), "Movie");
    const QString castFolderPath{imgHostPath + "/Movie/Chris Hemsworth"};
    const QString castPsonPath{imgHostPath + "/Movie/Chris Hemsworth/Chris Hemsworth.pson"};
    QVERIFY2(!QDir{castFolderPath}.exists(), "Precondition: Chris Hemsworth path should not exist");
    QCOMPARE(CastBaseDb::GetCastPath(chrisHenmworthRecord, imgHostPath), castFolderPath);
    QCOMPARE(CastBaseDb::GetCastFilePath(chrisHenmworthRecord, imgHostPath), castPsonPath);
    QVERIFY(!CastBaseDb::UpdateRecordImgsField(chrisHenmworthRecord, imgHostPath)); // path no exist

    // 2. folder exist should update img fields only
    QSqlRecord chrisEvansRecord = GetACastRecordLine("Chris Evans", "Movie", defaultImgs);
    const QString& oldImgs = chrisEvansRecord.value(PERFORMER_DB_HEADER_KEY::Imgs).toString();
    QCOMPARE(oldImgs, defaultImgs);
    QVERIFY(CastBaseDb::UpdateRecordImgsField(chrisEvansRecord, imgHostPath));
    const QString& newImgs = chrisEvansRecord.value(PERFORMER_DB_HEADER_KEY::Imgs).toString();
    QVERIFY(newImgs != defaultImgs);
    QVERIFY(newImgs.contains("Chris Evans 2.jpg")); // 2 file "A.jpg\nB.jpg" in filesystem-depend sequence
    QVERIFY(newImgs.contains("Chris Evans.jpg"));
    QCOMPARE(newImgs.count('\n'), 1);
  }

  void test_WhenCastNameRenamed_skip() { // must last test here
    QCOMPARE(CastBaseDb::WhenCastNameRenamed("inexists path", "a", "b"), -1); // not exist

    QString imgHostPathOri = mDir.itemPath("Movie");
    QCOMPARE(CastBaseDb::WhenCastNameRenamed(imgHostPathOri, "Kaka", "Kaka"), 0); // skip
    QCOMPARE(CastBaseDb::WhenCastNameRenamed(imgHostPathOri, "Kaka", "<a|\">"), -1); // invalid chars
    QCOMPARE(CastBaseDb::WhenCastNameRenamed(imgHostPathOri, "Kaka", "Kaka/Football"), -1); // move files up or down

    QCOMPARE(CastBaseDb::WhenCastNameRenamed(imgHostPathOri, "Kaka", "Kaka Leite"), 3); // move files up or down
  }
};

#include "CastBaseDbTest.moc"
REGISTER_TEST(CastBaseDbTest, false)
