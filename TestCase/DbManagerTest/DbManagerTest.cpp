#include <QtTest/QtTest>
#include "OnScopeExit.h"
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "DbManager.h"
#include "EndToExposePrivateMember.h"
#include <QSqlRecord>
#include "TDir.h"
#include <QSqlQuery>
#include <QSqlError>
#include "DbManagerHelper.h"
#include "PublicTool.h"
#include "FileToolMock.h"
#include "MountHelper.h"
#include "MountPathTableNameMapperMock.h"
#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class DbManagerTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir tDir;
  DbManager mDBMgr{tDir.itemPath("DB_MANAGER_TEST.db"), "DB_MANAGER_TEST_CONNECTION"};
  static const QString DB_MANAGER_TEST_TBL_NAME; //
  static const QString DB_MANGER_TBL_DEFINITION;
  static const QString INSERT_RECORDS_STR;

  bool CreateATableForTest() { //
    return mDBMgr.CreateTable(DB_MANAGER_TEST_TBL_NAME, DB_MANGER_TBL_DEFINITION);
  }

  int InsertTestData() {
    const QString insertCmd = INSERT_RECORDS_STR.arg(DB_MANAGER_TEST_TBL_NAME);
    return mDBMgr.UpdateForTest(insertCmd);
  }

private slots:
  void initTestCase() {
    GlobalMockObject::reset();
        using namespace FileToolMock;
    MOCKER(FileTool::OpenLocalFileUsingDesktopService).stubs().will(invoke(invokeOpenLocalFileUsingDesktopService));
    
    using namespace MountPathTableNameMapper;
    using namespace MountPathTableNameMapperMock;
    MOCKER(toMountPath).stubs().will(invoke(invokeToMountPath));
    MOCKER(toTableName).stubs().will(invoke(invokeToTableName));

    QVERIFY(tDir.IsValid());
    QVERIFY(mDBMgr.IsValid());

    QVERIFY(mDBMgr.GetCfgDebug().contains("DB_MANAGER_TEST.db"));
    QVERIFY(mDBMgr.GetCfgDebug().contains("DB_MANAGER_TEST_CONNECTION"));
  }

  void cleanupTestCase() {
    GlobalMockObject::verify();

    QCOMPARE(InsertTestData(), 4);
    QCOMPARE(mDBMgr.IsTableExist(DB_MANAGER_TEST_TBL_NAME), true);
    mDBMgr.ReleaseConnection();

    // below is the operation when db invalid. should not affect database
    {
      auto invalidDb = mDBMgr.GetDb();
      QCOMPARE(invalidDb.isValid(), false);
      QCOMPARE(mDBMgr.CreateTable("ANY_RANDOM_TABLE", DB_MANGER_TBL_DEFINITION), false);
      QCOMPARE(mDBMgr.IsTableExist(DB_MANAGER_TEST_TBL_NAME), false);

      QList<QSqlRecord> records;
      QCOMPARE(mDBMgr.QueryForTest(QString{"SELECT * FROM ``"}.arg(DB_MANAGER_TEST_TBL_NAME), records), false);
      QCOMPARE(records.size(), 0);

      QSet<QString> namesPrimaryKey;
      QCOMPARE(mDBMgr.QueryPK(DB_MANAGER_TEST_TBL_NAME, "name", namesPrimaryKey), false);
      QCOMPARE(namesPrimaryKey.size(), 0);

      QCOMPARE(mDBMgr.CountRow(DB_MANAGER_TEST_TBL_NAME), FD_DB_OPEN_FAILED);

      QCOMPARE(mDBMgr.DropTable(DB_MANAGER_TEST_TBL_NAME), FD_DB_OPEN_FAILED);
      QCOMPARE(mDBMgr.ClearTable(DB_MANAGER_TEST_TBL_NAME), FD_DB_OPEN_FAILED);
      QCOMPARE(mDBMgr.UpdateForTest(QString{"UPDATE `%1` SET `grade`='A'"}.arg(DB_MANAGER_TEST_TBL_NAME)), FD_DB_INVALID);
    }
  }

  void cleanup() { //
    QVERIFY(mDBMgr.ClearTable(DB_MANAGER_TEST_TBL_NAME) >= 0);
  }

  void initalize_ok() {
    QVERIFY(mDBMgr.IsValid());
    QCOMPARE(mDBMgr.mDbName, tDir.itemPath("DB_MANAGER_TEST.db"));
    QCOMPARE(mDBMgr.mConnName, "DB_MANAGER_TEST_CONNECTION");

    // database exist, table exist
    QVERIFY(CreateATableForTest());
    QVERIFY(QFile::exists(mDBMgr.mDbName));
    QVERIFY(mDBMgr.IsTableExist(DB_MANAGER_TEST_TBL_NAME));
  }

  void CreateTable_success() {
    const QString testTable = "RANDOM_TEST_TABLE";
    const QString definition = R"(CREATE TABLE `%1` (id INTEGER PRIMARY KEY, data TEXT))";
    QVERIFY(mDBMgr.CreateTable(testTable, definition));
    ON_SCOPE_EXIT {
      QCOMPARE(mDBMgr.DropTable(testTable), 1);
    };

    auto db = mDBMgr.GetDb();
    QVERIFY(db.tables().contains(testTable));
    QCOMPARE(mDBMgr.DropTable(testTable), 1);           // 1 table been dropped ok
    QVERIFY(mDBMgr.CreateTable(testTable, definition)); // create again skip
    QCOMPARE(mDBMgr.onShowInFileSystemView(), true);

    {
      // invalid table template definition
      QVERIFY(!mDBMgr.CreateTable("INVALID_TABLE", "INVALID SQL"));

      // empty DB_MANAGER_TEST_TBL_NAME
      QVERIFY(!mDBMgr.CreateTable("", DB_MANGER_TBL_DEFINITION));

      // invalid connection
      DbManager invalidMgr("", "");
      QCOMPARE(invalidMgr.IsValid(), false);
      QVERIFY(!invalidMgr.CreateTable(DB_MANAGER_TEST_TBL_NAME, DB_MANGER_TBL_DEFINITION));
      QCOMPARE(invalidMgr.onShowInFileSystemView(), false);
    }
  }

  void insertAndQuery_ok() {
    QVERIFY(mDBMgr.IsTableExist(DB_MANAGER_TEST_TBL_NAME));
    QCOMPARE(mDBMgr.CountRow(DB_MANAGER_TEST_TBL_NAME), 0);
    QCOMPARE(InsertTestData(), 4);
    QCOMPARE(mDBMgr.CountRow(DB_MANAGER_TEST_TBL_NAME), 4);

    QList<QSqlRecord> records;
    QVERIFY(mDBMgr.QueryForTest(QString("SELECT * FROM %1").arg(DB_MANAGER_TEST_TBL_NAME), records));
    QCOMPARE(records.size(), 4);
    QCOMPARE(records[0].value("name").toString(), "Alice");
    QCOMPARE(records[0].value("grade").toString(), "A");

    { // exception scenario, invalid sql where clause
      QList<QSqlRecord> records;
      QVERIFY(!mDBMgr.QueryForTest("INVALID SQL", records));
      QCOMPARE(records.isEmpty(), true);

      QVERIFY(!mDBMgr.QueryForTest("SELECT * FROM", records));
      QCOMPARE(records.isEmpty(), true);
    }

    { // count where clause invalid
      QCOMPARE(mDBMgr.CountRow("invalid where clause command"), FD_EXEC_FAILED);
    }
  }

  void QueryPK_ok() {
    QCOMPARE(InsertTestData(), 4);

    // primary key
    QSet<QString> names;
    QVERIFY(mDBMgr.QueryPK(DB_MANAGER_TEST_TBL_NAME, "name", names));
    QCOMPARE(names, QSet<QString>({"Alice", "Bob", "Charlie", "David"}));

    // not exist column
    QSet<int> invalid;
    QVERIFY(!mDBMgr.QueryPK(DB_MANAGER_TEST_TBL_NAME, "invalid_column", invalid));
    QCOMPARE(invalid.isEmpty(), true);
  }

  void UpdateData_ok() {
    QVERIFY(InsertTestData());

    QSqlQuery simpleCountGetQry{QString("SELECT COUNT(*) FROM `%1` WHERE `name`='Bob'").arg(DB_MANAGER_TEST_TBL_NAME), mDBMgr.GetDb()};
    QCOMPARE(simpleCountGetQry.lastError().type(), QSqlError::NoError);
    QVERIFY(simpleCountGetQry.next());
    QCOMPARE(simpleCountGetQry.value(0).toInt(), 1); // only one record where name = Bob
    // score upgrade
    const QString updateCmd = QString("UPDATE `%1` SET `grade`='A' WHERE `name`='Bob'").arg(DB_MANAGER_TEST_TBL_NAME);
    QCOMPARE(mDBMgr.UpdateForTest(updateCmd), 1);

    QList<QSqlRecord> records;
    mDBMgr.QueryForTest(QString("SELECT `grade` FROM %1 WHERE `name`='Bob'").arg(DB_MANAGER_TEST_TBL_NAME), records);
    QCOMPARE(records.size(), 1);
    QCOMPARE(records[0].value("grade").toString(), "A");

    { // bounder test
      QVERIFY(mDBMgr.UpdateForTest(
          QString("INSERT INTO %1 (name, student_id, grade) VALUES ('Eve', 'S1005', 'A')").arg(DB_MANAGER_TEST_TBL_NAME)));

      // primary key `name` conflict
      int result = mDBMgr.UpdateForTest(
          QString("INSERT INTO %1 (name, student_id, grade) VALUES ('Eve', 'S1006', 'B')").arg(DB_MANAGER_TEST_TBL_NAME));
      QVERIFY(result < FD_OK); // error code here

      // unique key `student_id` conflict
      result = mDBMgr.UpdateForTest(
          QString("INSERT INTO %1 (name, student_id, grade) VALUES ('Frank', 'S1005', 'C')").arg(DB_MANAGER_TEST_TBL_NAME));
      QVERIFY(result < FD_OK);

      // range key `grade` violation
      result = mDBMgr.UpdateForTest(
          QString("INSERT INTO %1 (name, student_id, grade) VALUES ('Grace', 'S1007', 'F')").arg(DB_MANAGER_TEST_TBL_NAME));
      QVERIFY(result < FD_OK);
    }
    { // invalid update command
      QCOMPARE(mDBMgr.UpdateForTest("INVALID SQL"), FD_EXEC_FAILED);
    }
  }

  void test_DeleteByWhereClause() {
    QVERIFY(InsertTestData());

    // grade under expectation
    QSqlQuery simpleCountGetQry{QString("SELECT COUNT(*) FROM `%1` WHERE `grade`='D'").arg(DB_MANAGER_TEST_TBL_NAME), mDBMgr.GetDb()};
    QCOMPARE(simpleCountGetQry.lastError().type(), QSqlError::NoError);
    QVERIFY(simpleCountGetQry.next());
    QCOMPARE(simpleCountGetQry.value(0).toInt(), 1); // only one record where grade = D

    QCOMPARE(mDBMgr.CountRow(DB_MANAGER_TEST_TBL_NAME), 4);
    int deleteGradeDCount = mDBMgr.DeleteByWhereClause(DB_MANAGER_TEST_TBL_NAME, "grade = 'D'");
    QCOMPARE(deleteGradeDCount, 1);
    QCOMPARE(mDBMgr.CountRow(DB_MANAGER_TEST_TBL_NAME), 4 - 1);

    int alreadyDeletedRecordAgain = mDBMgr.DeleteByWhereClause(DB_MANAGER_TEST_TBL_NAME, "grade = 'D'");
    QCOMPARE(alreadyDeletedRecordAgain, 0);
    QCOMPARE(mDBMgr.CountRow(DB_MANAGER_TEST_TBL_NAME), 4 - 1); // unchage

    int deleteAllCount = mDBMgr.DeleteByWhereClause(DB_MANAGER_TEST_TBL_NAME, ""); // empty where clause
    QCOMPARE(deleteAllCount, 3);

    { // delete command error
      // no column field
      int invalidField = mDBMgr.DeleteByWhereClause(DB_MANAGER_TEST_TBL_NAME, "invalid_column = 1");
      QCOMPARE(invalidField, FD_EXEC_FAILED);

      // invalid clause
      int invalidWhereClause = mDBMgr.DeleteByWhereClause(DB_MANAGER_TEST_TBL_NAME, "invalid_column |<>= 1");
      QCOMPARE(invalidWhereClause, FD_EXEC_FAILED);
    }
  }

  void clearTable_ok() {
    QVERIFY(InsertTestData());

    // existed table
    QVERIFY(mDBMgr.IsTableExist(DB_MANAGER_TEST_TBL_NAME));
    QCOMPARE(mDBMgr.ClearTable(DB_MANAGER_TEST_TBL_NAME), 1); // 1 get deleted
    QVERIFY(mDBMgr.IsTableEmpty(DB_MANAGER_TEST_TBL_NAME));

    {
      // table already deleted or not exist at all
      QVERIFY(!mDBMgr.IsTableExist("NON_EXISTENT_TABLE"));
      QCOMPARE(mDBMgr.ClearTable("NON_EXISTENT_TABLE"), 0);
    }
  }

  void dropTable_ok() {
    const QString tempTable = "TEMP_TABLE";
    QVERIFY(mDBMgr.CreateTable(tempTable, DB_MANGER_TBL_DEFINITION));
    QCOMPARE(mDBMgr.DropTable(tempTable), 1);
    QVERIFY(!mDBMgr.IsTableExist(tempTable));

    {
      // already not exist table
      QVERIFY(!mDBMgr.IsTableExist("NON_EXISTENT_TABLE"));
      QCOMPARE(mDBMgr.DropTable("NON_EXISTENT_TABLE"), 0);
    }
  }

  void test_RmvTableWithRegex() {
    QCOMPARE(mDBMgr.IsTableExist(DB_MANAGER_TEST_TBL_NAME), true);
    QCOMPARE(mDBMgr.DropTable(DB_MANAGER_TEST_TBL_NAME), true);
    ON_SCOPE_EXIT {
      QCOMPARE(CreateATableForTest(), true);
    };

    QVERIFY(mDBMgr.CreateTable("logs_2023", DB_MANGER_TBL_DEFINITION));
    QVERIFY(mDBMgr.CreateTable("logs_2024", DB_MANGER_TBL_DEFINITION));
    QVERIFY(mDBMgr.CreateTable("users", DB_MANGER_TBL_DEFINITION));
    auto db = mDBMgr.GetDb();
    QCOMPARE(db.tables(), (QStringList{"logs_2023", "logs_2024", "users"}));

    int tableAlreadyNotExists = mDBMgr.RmvTable("logs_.*", DbManagerHelper::DropOrDeleteE::DROP);
    QCOMPARE(tableAlreadyNotExists, 0);
    QCOMPARE(db.tables(), (QStringList{"logs_2023", "logs_2024", "users"}));

    int oneTableDropped = mDBMgr.RmvTable("logs_2023", DbManagerHelper::DropOrDeleteE::DROP);
    QCOMPARE(db.tables(), (QStringList{"logs_2024", "users"}));
    QCOMPARE(oneTableDropped, 1);
  }
};

const QString DbManagerTest::DB_MANAGER_TEST_TBL_NAME{"STUDENTS_SCORE"}; //
const QString DbManagerTest::DB_MANGER_TBL_DEFINITION{
    R"(CREATE TABLE IF NOT EXISTS `%1` (
`name` TEXT,
`student_id` TEXT NOT NULL UNIQUE,
`grade` CHAR(1) CHECK(grade IN ('A','B','C','D','E')),
PRIMARY KEY (`name`));)"};
const QString DbManagerTest::INSERT_RECORDS_STR{
    R"(INSERT INTO `%1` (`name`, `student_id`, `grade`) VALUES
        ('Alice', 'S1001', 'A'),
        ('Bob', 'S1002', 'B'),
        ('Charlie', 'S1003', 'C'),
        ('David', 'S1004', 'D')
)"};

#include "DbManagerTest.moc"
REGISTER_TEST(DbManagerTest, false)
