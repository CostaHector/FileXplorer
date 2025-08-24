#include <QCoreApplication>
#include <QtTest>
#include <QSqlRecord>
#include "MyTestSuite.h"
#include "CastBaseDb.h"
#include "PerformerJsonFileHelper.h"
#include "PublicVariable.h"

const QString rootpath = QFileInfo(__FILE__).absolutePath();
const QString imgHostPath = rootpath + "/PerfImgHost";
const QString dbName = rootpath + "/PERF.db";

class CastBaseDbTest : public MyTestSuite {
  Q_OBJECT
 public:
  CastBaseDbTest() : MyTestSuite{false} {}
  static QString GetPerfTextExample() {
    QString perfText;
    perfText += "Kaka|Ricardo Izecson dos Santos Leite";
    perfText += '\n';
    perfText += "Chris Evans,Cevans and Christopher Robert Evans";
    perfText += '\n';
    perfText += "Huge Jackman @ Wolverine";
    perfText += '\n';
    perfText += "Ricky Martin, Enrique Martin Morales";
    return perfText;
  }
  static constexpr int PERFS_ITEM_COUNT = 4;

 private slots:
  void cleanup() {
    // post-condition
    if (QFile{dbName}.exists()) {
      QFile{dbName}.remove();
    }
  }

  void test_InsertPerformers() {
    // precondition
    QVERIFY(!QFile::exists(dbName));
    const QString& perfText = GetPerfTextExample();
    CastBaseDb perfDb{dbName, "PERF_CONNECTION"};
    QVERIFY(perfDb.CreateDatabase());
    QVERIFY(perfDb.CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(QFile::exists(dbName));

    const QStringList perfs{"Kaka", "Chris Evans", "Huge Jackman", "Ricky Martin"};
    // procedure
    QCOMPARE(perfDb.InsertPerformers(perfs), PERFS_ITEM_COUNT);

    QList<QSqlRecord> records;
    QVERIFY(perfDb.QueryForTest("SELECT * from " + DB_TABLE::PERFORMERS, records));
    QCOMPARE(records.size(), PERFS_ITEM_COUNT);
    QSet<QString> actualNames;
    for (const auto& record : records) {
      actualNames << record.value(PERFORMER_DB_HEADER_KEY::Name).toString();
    }
    const QSet<QString> expectNames{perfs.cbegin(), perfs.cend()};
    QCOMPARE(actualNames, expectNames);

    // everything already exist, nothing inserted, UNIQUE constraint failed
    QCOMPARE(perfDb.InsertPerformers(perfs), FD_EXEC_FAILED);
    records.clear();
    QVERIFY(perfDb.QueryForTest("SELECT * from " + DB_TABLE::PERFORMERS, records));
    QCOMPARE(records.size(), PERFS_ITEM_COUNT);
  }

  void test_aka_name_split() {
    const auto& noPerfFromEmptyText = CastBaseDb::GetFreqName2AkaNames("");
    QVERIFY(noPerfFromEmptyText.isEmpty());

    const auto& noPerfFromUselessText = CastBaseDb::GetFreqName2AkaNames("\n\n\n");
    QVERIFY(noPerfFromUselessText.isEmpty());

    const QString& perfText = GetPerfTextExample();
    QVERIFY(!perfText.isEmpty());
    const auto& perfs = CastBaseDb::GetFreqName2AkaNames(perfText);
    QCOMPARE(perfs.size(), PERFS_ITEM_COUNT);
    QCOMPARE(perfs.value("Kaka"), "Ricardo Izecson dos Santos Leite");
    QCOMPARE(perfs.value("Chris Evans"), "Cevans,Christopher Robert Evans");
    QCOMPARE(perfs.value("Huge Jackman"), "Wolverine");
    QCOMPARE(perfs.value("Ricky Martin"), "Enrique Martin Morales");
  }

  void test_ReadFromUserInputSentence() {
    // precondition
    QVERIFY(!QFile::exists(dbName));
    const QString& perfText = GetPerfTextExample();
    CastBaseDb perfDb{dbName, "PERF_CONNECTION"};
    QVERIFY(perfDb.CreateDatabase());
    QVERIFY(perfDb.CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(QFile::exists(dbName));
    // procedure
    QCOMPARE(perfDb.ReadFromUserInputSentence(perfText), PERFS_ITEM_COUNT);

    QList<QSqlRecord> records;
    QVERIFY(perfDb.QueryForTest("SELECT * from " + DB_TABLE::PERFORMERS, records));
    QCOMPARE(records.size(), PERFS_ITEM_COUNT);
    QSet<QString> actualNames;
    QSet<QString> actualAkas;
    for (const auto& record : records) {
      actualNames << record.value(PERFORMER_DB_HEADER_KEY::Name).toString();
      actualAkas << record.value(PERFORMER_DB_HEADER_KEY::AKA).toString();
    }
    const QSet<QString> expectNames{"Kaka", "Chris Evans", "Huge Jackman", "Ricky Martin"};
    const QSet<QString> expectAkas{"Ricardo Izecson dos Santos Leite", "Cevans,Christopher Robert Evans", "Wolverine", "Enrique Martin Morales"};
    QCOMPARE(actualNames, expectNames);
    QCOMPARE(actualAkas, expectAkas);
  }

  void test_ReadFromImageHost() {
    // precondition
    QVERIFY(!QFile::exists(dbName));
    CastBaseDb perfDb{dbName, "PERF_CONNECTION"};
    QVERIFY(perfDb.CreateDatabase());
    QVERIFY(perfDb.CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(QFile::exists(dbName));
    // procedure
    QCOMPARE(perfDb.ReadFromImageHost(imgHostPath), PERFS_ITEM_COUNT);
    QList<QSqlRecord> records;
    QVERIFY(perfDb.QueryForTest("SELECT * from " + DB_TABLE::PERFORMERS, records));
    QCOMPARE(records.size(), PERFS_ITEM_COUNT);
    QSet<QString> actualNames;
    QSet<QString> actualOris;
    QSet<QString> actualImgs;
    for (const auto& record : records) {
      actualNames << record.value(PERFORMER_DB_HEADER_KEY::Name).toString();
      actualOris << record.value(PERFORMER_DB_HEADER_KEY::Orientation).toString();
      actualImgs << record.value(PERFORMER_DB_HEADER_KEY::Imgs).toString();
    }
    QSet<QString> expectNames{"Kaka", "Chris Evans", "Huge Jackman", "Ricky Martin"};
    QSet<QString> expectOris{"hetero", "hetero", "hetero", "gay"};
    QSet<QString> expectImgs{"Kaka.jpg", "Chris Evans.jpg\nChris Evans 2.jpg", "Huge Jackman.jpg", "Ricky Martin.jpg"};
    QCOMPARE(actualNames, expectNames);
    QCOMPARE(actualOris, expectOris);
    QCOMPARE(actualImgs, expectImgs);
  }

  void test_LoadFromPJsonFile() {
    // precondition
    QVERIFY(!QFile::exists(dbName));
    CastBaseDb perfDb{dbName, "PERF_CONNECTION"};
    QVERIFY(perfDb.CreateDatabase());
    QVERIFY(perfDb.CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(QFile::exists(dbName));
    // procedure
    QCOMPARE(perfDb.LoadFromPJsonFile(imgHostPath), PERFS_ITEM_COUNT);
    // load again, replace into will always ok
    QCOMPARE(perfDb.LoadFromPJsonFile(imgHostPath), PERFS_ITEM_COUNT);

    QSet<QString> pkNames;
    QVERIFY(perfDb.QueryPK(DB_TABLE::PERFORMERS, PERFORMER_DB_HEADER_KEY::Name, pkNames));
    QSet<QString> expectNames{"Kaka", "Chris Evans", "Huge Jackman", "Ricky Martin"};
    QCOMPARE(pkNames, expectNames);

    QList<QSqlRecord> records;
    QVERIFY(perfDb.QueryForTest("SELECT * from " + DB_TABLE::PERFORMERS, records));
    QCOMPARE(records.size(), PERFS_ITEM_COUNT);
    QSet<QString> actualNames;
    QSet<QString> actualOris;
    QSet<QString> actualImgs;
    QSet<int> actualRate;
    for (const auto& record : records) {
      actualNames << record.value(PERFORMER_DB_HEADER_KEY::Name).toString();
      actualOris << record.value(PERFORMER_DB_HEADER_KEY::Orientation).toString();
      actualImgs << record.value(PERFORMER_DB_HEADER_KEY::Imgs).toString();
      actualRate << record.value(PERFORMER_DB_HEADER_KEY::Rate).toInt();
    }
    QSet<QString> expectOris{"hetero", "hetero", "hetero", "gay"};
    QSet<QString> expectImgs{"Kaka.jpg", "Chris Evans.jpg\nChris Evans 2.jpg", "Huge Jackman.jpg", "Ricky Martin.jpg"};
    QSet<int> expectRate{10, 9, 8, 10};
    QCOMPARE(actualNames, expectNames);
    QCOMPARE(actualOris, expectOris);
    QCOMPARE(actualImgs, expectImgs);
    QCOMPARE(actualRate, expectRate);
  }
};
constexpr int CastBaseDbTest::PERFS_ITEM_COUNT;

CastBaseDbTest g_CastBaseDbTest;
#include "CastBaseDbTest.moc"
