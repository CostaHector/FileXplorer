#include <QCoreApplication>
#include <QtTest>
#include <QSqlRecord>
#include "TestCase/pub/MyTestSuite.h"
#include "TestCase/PathRelatedTool.h"
#include "Tools/FileDescriptor/MovieBaseDb.h"
#include "Tools/FileDescriptor/TableFields.h"
#include "public/PublicVariable.h"
#include "public/PublicMacro.h"

const QString rootpath = QFileInfo(__FILE__).absolutePath();
const QString dbName = rootpath + "/MOVIE.db";
const QString readVidFromPath = TestCaseRootPath() + "/test/TestEnv_VideosDurationGetter";

class MovieBaseDbTest : public MyTestSuite {
  Q_OBJECT
 public:
  MovieBaseDbTest() : MyTestSuite{false} {}
  static constexpr int VIDS_CNT_UNDER_PATH = 8;
 private slots:
  void cleanup() {
    // post-condition
    if (QFile{dbName}.exists()) {
      QFile{dbName}.remove();
    }
  }

  void test_create_table_succeed_and_empty_record() {
    // precondition
    QVERIFY(!QFile{dbName}.exists());
    // procedure
    MovieBaseDb mDb{dbName, "CONN_MOVIE_DB_TEST"};
    QVERIFY(mDb.CreateDatabase());
    QVERIFY(mDb.CreateTable(DB_TABLE::MOVIES, MovieBaseDb::CREATE_MOVIE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());

    QList<QSqlRecord> records;
    QVERIFY(mDb.QueryForTest(QString("SELECT * FROM %1").arg(DB_TABLE::MOVIES), records));
    QVERIFY(records.isEmpty());

    QCOMPARE(mDb.CountRow(DB_TABLE::MOVIES), 0);
  }

  void test_ReadVideosFromAPath_8Videos() {
    // precondition
    QVERIFY(!QFile{dbName}.exists());
    // procedure
    MovieBaseDb mDb{dbName, "CONN_MOVIE_DB_TEST"};
    QVERIFY(mDb.CreateDatabase());
    QVERIFY(mDb.CreateTable(DB_TABLE::MOVIES, MovieBaseDb::CREATE_MOVIE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());

    QCOMPARE(mDb.ReadVideosFromAPath(readVidFromPath, DB_TABLE::MOVIES), VIDS_CNT_UNDER_PATH);

    QList<QSqlRecord> records;
    QVERIFY(mDb.QueryForTest(QString("SELECT * FROM %1").arg(DB_TABLE::MOVIES), records));
    QCOMPARE(records.size(), VIDS_CNT_UNDER_PATH);

    QCOMPARE(mDb.CountRow(DB_TABLE::MOVIES), VIDS_CNT_UNDER_PATH);

    // total count = 8
    // name contains "2008, The Blender Foundation": count = 3
    using namespace DB_HEADER_KEY;
    const QString qryWhereClause{QString(R"(`%1` like "%2008, The Blender Foundation%")").arg(VOLUME_ENUM_TO_STRING(Name))};
    QCOMPARE(mDb.CountRow(DB_TABLE::MOVIES, qryWhereClause), 3);
    // should remove all 3 records
    QCOMPARE(mDb.DeleteByWhereClause(DB_TABLE::MOVIES, qryWhereClause), 3);
    // now count now = 8 - 3 = 5
    QCOMPARE(mDb.CountRow(DB_TABLE::MOVIES), VIDS_CNT_UNDER_PATH - 3);
  }
};

constexpr int MovieBaseDbTest::VIDS_CNT_UNDER_PATH;

MovieBaseDbTest g_MovieBaseDbTest;
#include "MovieBaseDbTest.moc"
