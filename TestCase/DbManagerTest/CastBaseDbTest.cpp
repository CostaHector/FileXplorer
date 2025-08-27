#include <QCoreApplication>
#include <QtTest>
#include <QSqlRecord>
#include <QSqlField>
#include "MyTestSuite.h"
#include "CastBaseDb.h"
#include "TableFields.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "TableFields.h"
#include "StringTool.h"
#include <QDir>
#include <QMap>

QSqlRecord GetACastRecordLine(const QString& castName, const QString& ori, const QString& imgs) {
  QSqlRecord rec;
  using namespace PERFORMER_DB_HEADER_KEY;
  rec.append(QSqlField(ENUM_2_STR(Name)  , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Rate)  , QVariant::Int));
  rec.append(QSqlField(ENUM_2_STR(AKA)   , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Tags)  , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Ori)   , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Vids)  , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Imgs)  , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Detail), QVariant::String));

  rec.setValue(Name, castName);
  rec.setValue(Rate, 9);
  rec.setValue(AKA, "Thor");
  rec.setValue(Tags, "Man");
  rec.setValue(Ori, ori);
  rec.setValue(Vids, "The Avengers (2012)\nThor: The Dark World (2013)\nAvengers: Age of Ultron (2015)");
  rec.setValue(Imgs, imgs);
  rec.setValue(Detail, "The Australian actor first appeared as Thor in 2011 and has since reprised the role in numerous MCU films");
  return rec;
}

const QString rootpath = QFileInfo(__FILE__).absolutePath();
const QString imgHostPath = rootpath + "/PerfImgHost";
const QString dbName = rootpath + "/PERF.db";

/* file system structure
root: PerfImgHost, subitems as follow
gay
  Ricky Martin
    Ricky Martin.jpg
    Ricky Martin.pson
hetero
  Chris Evans
    Chris Evans.jpg
    Chris Evans 2.jpg
    Chris Evans.pson
  Huge Jackman
    Huge Jackman.jpg
    Huge Jackman.pson
  Kaka
    Kaka.jpg
    Kaka.pson
*/

struct CastStructureProperty {
  QString ori;
  QSet<QString> imgsStr;
  int rate;
};

class CastBaseDbTest : public MyTestSuite {
  Q_OBJECT
public:
  CastBaseDbTest() : MyTestSuite{false} {}

  // itself
  QString GetPerfAkaMultiLineText() const { // 4
    static const QString perfAndItsAkaText {
        "Kaka|Ricardo Izecson dos Santos Leite\n"
        "Chris Evans,Cevans and Christopher Robert Evans\n"
        "Huge Jackman @ Wolverine\n"
        "Ricky Martin, Enrique Martin Morales"
    };
    return perfAndItsAkaText;
  }
  const QMap<QString, QString> mPerf2AkaMaps // 4
      { //
       {"Kaka", "Ricardo Izecson dos Santos Leite"},
       {"Chris Evans", "Cevans,Christopher Robert Evans"},
       {"Huge Jackman", "Wolverine"},
       {"Ricky Martin", "Enrique Martin Morales"},
       };
  // others
  QString GetPerfOtherOrNoneAkaMultiLineText() const { // 6
    static const QString perfAndOtherAkaText {
        "Chris Evans, Captain America\n"
        "Huge Jackman\n"
        "Ricky Martin, Enrique Martin Morales\n"
        "Jake Gyllenhaal,Jacob Benjamin Gyllenhaal\n"
        "Brad Pitt,William Bradley Pitt\n"
        "Frank Grillo,Crossbones"
    };
    return perfAndOtherAkaText;
  }
  const QMap<QString, QString> mPerf2OtherAkaMaps // 6
      { //
          {"Chris Evans", "Captain America"},
          {"Huge Jackman", ""},
          {"Ricky Martin", "Enrique Martin Morales"},
          {"Jake Gyllenhaal", "Jacob Benjamin Gyllenhaal"},
          {"Brad Pitt", "William Bradley Pitt"},
          {"Frank Grillo", "Crossbones"}
      };

  const QMap<QString, CastStructureProperty> gExpectName2Property //
      {
       {"Kaka", {"hetero", QSet<QString>{"Kaka.jpg"}, 10}},
       {"Chris Evans", {"hetero", QSet<QString>{"Chris Evans.jpg", "Chris Evans 2.jpg"}, 9}},
       {"Huge Jackman", {"hetero", QSet<QString>{"Huge Jackman.jpg"}, 8}},
       {"Ricky Martin", {"gay", QSet<QString>{"Ricky Martin.jpg"}, 10}},
       };

private slots:
  void cleanup() {
    // post-condition
    if (QFile{dbName}.exists()) {
      QFile{dbName}.remove();
    }
  }

  void test_aka_name_split() {
    const auto& noPerfFromEmptyText = CastBaseDb::GetFreqName2AkaNames("");
    QVERIFY(noPerfFromEmptyText.isEmpty());

    const auto& noPerfFromUselessText = CastBaseDb::GetFreqName2AkaNames("\n\n\n");
    QVERIFY(noPerfFromUselessText.isEmpty());

    const QMap<QString, QString> actualPerf2AkaMaps = CastBaseDb::GetFreqName2AkaNames(GetPerfAkaMultiLineText());
    QCOMPARE(actualPerf2AkaMaps, mPerf2AkaMaps);

    const QMap<QString, QString> actualPerfs2OtherAkaMaps = CastBaseDb::GetFreqName2AkaNames(GetPerfOtherOrNoneAkaMultiLineText());
    QCOMPARE(actualPerfs2OtherAkaMaps, mPerf2OtherAkaMaps);
  }

  void test_ReadFromUserInputSentence() {
    // precondition
    QVERIFY(!QFile::exists(dbName));

    CastBaseDb perfDb{dbName, "PERF_CONNECTION"};
    QVERIFY(perfDb.CreateDatabase());
    QVERIFY(perfDb.CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(QFile::exists(dbName));

    // procedure
    // 1. add from 4 lines text
    QCOMPARE(perfDb.AppendCastFromMultiLineInput(GetPerfAkaMultiLineText()), 4);
    // 2. add from other 6 lines text
    QCOMPARE(perfDb.AppendCastFromMultiLineInput(GetPerfOtherOrNoneAkaMultiLineText()), 6);
    // INSERT INTO ON CONFLICT SET used in AppendCastFromMultiLineInput when PRIMARY KEY CONFLICTS, aka get updated

    const auto& oldMap = mPerf2AkaMaps;
    const auto& newMap = mPerf2OtherAkaMaps;

    QList<QSqlRecord> records;
    QVERIFY(perfDb.QueryForTest("SELECT * from " + DB_TABLE::PERFORMERS, records));
    for (const auto& record : records) {
      const QString castName{record.value(PERFORMER_DB_HEADER_KEY::Name).toString()};
      const QString akaNames{record.value(PERFORMER_DB_HEADER_KEY::AKA).toString()};
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
    QVERIFY(!QFile::exists(dbName));
    CastBaseDb perfDb{dbName, "PERF_CONNECTION"};
    QVERIFY(perfDb.CreateDatabase());
    QVERIFY(perfDb.CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(QFile::exists(dbName));

    // procedure
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
    QVERIFY(!QFile::exists(dbName));
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
    QSqlRecord chrisHenmworthRecord = GetACastRecordLine("Chris Hemsworth", "Hetero", defaultImgs);
    QCOMPARE(chrisHenmworthRecord.value(PERFORMER_DB_HEADER_KEY::Name).toString(), "Chris Hemsworth");
    QCOMPARE(chrisHenmworthRecord.value(PERFORMER_DB_HEADER_KEY::Ori).toString(), "Hetero");
    QCOMPARE(chrisHenmworthRecord.value(ENUM_2_STR(Name)).toString(), "Chris Hemsworth");
    QCOMPARE(chrisHenmworthRecord.value(ENUM_2_STR(Ori)).toString(), "Hetero");
    const QString castFolderPath{imgHostPath + "/Hetero/Chris Hemsworth"};
    const QString castPsonPath{imgHostPath + "/Hetero/Chris Hemsworth/Chris Hemsworth.pson"};
    QVERIFY2(!QDir{castFolderPath}.exists(), "Precondition: Chris Hemsworth path should not exist");
    QCOMPARE(CastBaseDb::GetCastPath(chrisHenmworthRecord, imgHostPath), castFolderPath);
    QCOMPARE(CastBaseDb::GetCastFilePath(chrisHenmworthRecord, imgHostPath), castPsonPath);
    QVERIFY(!CastBaseDb::UpdateRecordImgsField(chrisHenmworthRecord, imgHostPath)); // path no exist

    // 2. folder exist should update img fields only
    QSqlRecord chrisEvansRecord = GetACastRecordLine("Chris Evans", "Hetero", defaultImgs);
    const QString& oldImgs = chrisEvansRecord.value(PERFORMER_DB_HEADER_KEY::Imgs).toString();
    QCOMPARE(oldImgs, defaultImgs);
    QVERIFY(CastBaseDb::UpdateRecordImgsField(chrisEvansRecord, imgHostPath));
    const QString& newImgs = chrisEvansRecord.value(PERFORMER_DB_HEADER_KEY::Imgs).toString();
    QVERIFY(newImgs != defaultImgs);
    QVERIFY(newImgs.contains("Chris Evans 2.jpg")); // 2 file "A.jpg\nB.jpg" in filesystem-depend sequence
    QVERIFY(newImgs.contains("Chris Evans.jpg"));
    QCOMPARE(newImgs.count('\n'), 1);
  }
};

CastBaseDbTest g_CastBaseDbTest;
#include "CastBaseDbTest.moc"
