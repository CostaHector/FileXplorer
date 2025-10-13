#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "SqlTableModelPub.h"
#include "EndToExposePrivateMember.h"
#include <QSqlQuery>
#include <QSqlError>
#include "TDir.h"

#define RECOVERABLE_TEST_TABLE_NAME "INVALID_FILTER_RECOVERABLE_TEST_TBL"

class SqlTableModelPubTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir tDir;
  const QString mRecoverableFilterDB = tDir.itemPath("RobustSetFilter.db");
  const QString mRecoverableFilterConn = "RobustSetFilterConn";
private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());
  }

  void cleanupTestCase() {
    // Avoid QWARN log:
    // QSqlDatabasePrivate::removeDatabase: connection 'RobustSetFilterConn' is still in use, all queries will cease to work.
    if (QSqlDatabase::contains(mRecoverableFilterConn)) {
      QSqlDatabase::removeDatabase(mRecoverableFilterConn);
    }
  }

  void recovery_from_invalid_filter_set_before_ok() {
    // 1. 创建测试数据库/表/3条记录
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", mRecoverableFilterConn);
    db.setDatabaseName(mRecoverableFilterDB);
    QVERIFY(db.open());
    {
      QSqlQuery createQuery(db);
      QVERIFY(createQuery.exec("CREATE TABLE " RECOVERABLE_TEST_TABLE_NAME "(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT)"));
      QVERIFY(createQuery.exec("INSERT INTO " RECOVERABLE_TEST_TABLE_NAME " (name) VALUES('Alice'), ('Bob'), ('Charlie')"));
      createQuery.finish();
    }

    {
      // 2. 初始化模型
      SqlTableModelPub model(nullptr, db);
      model.setTable(RECOVERABLE_TEST_TABLE_NAME);
      QVERIFY(model.select());
      QCOMPARE(model.rowCount(), 3);

      // 3. 设置非法过滤器
      bool invalidResult1 = model.SetFilterAndSelect("INVALID SQL WHERE CLAUSE");
      QVERIFY(!invalidResult1);
      QVERIFY(!model.m_lastFilterSucceed);
      QCOMPARE(model.m_lastFilter, "INVALID SQL WHERE CLAUSE");
      QVERIFY(!model.lastError().text().isEmpty());  // 有错误信息
      QCOMPARE(model.rowCount(), 0);

      // 4. 设置重复的非法过滤器
      bool invalidResult2 = model.SetFilterAndSelect("INVALID SQL WHERE CLAUSE");
      QVERIFY(!invalidResult2);
      QVERIFY(!model.m_lastFilterSucceed);
      QCOMPARE(model.m_lastFilter, "INVALID SQL WHERE CLAUSE");
      QVERIFY(!model.lastError().text().isEmpty());  // 有错误信息
      QCOMPARE(model.rowCount(), 0);

      // 5. 设置不重复的非法过滤器
      bool invalidResult3 = model.SetFilterAndSelect("INVALID SQL FILTER NEW");
      QVERIFY(!invalidResult3);
      QVERIFY(!model.m_lastFilterSucceed);
      QCOMPARE(model.m_lastFilter, "INVALID SQL FILTER NEW");
      QVERIFY(!model.lastError().text().isEmpty());  // 有错误信息
      QCOMPARE(model.rowCount(), 0);

      // 6. 设置合法过滤器
      bool validResult = model.SetFilterAndSelect("name = 'Alice'");
      QVERIFY(validResult);
      QVERIFY(model.m_lastFilterSucceed);
      QCOMPARE(model.m_lastFilter, "name = 'Alice'");
      QVERIFY(model.lastError().text().isEmpty());  // 无错误
      QCOMPARE(model.rowCount(), 1);
      QCOMPARE(model.data(model.index(0, 1)).toString(), "Alice");
    }

    db.close();
  }
};

#include "SqlTableModelPubTest.moc"
REGISTER_TEST(SqlTableModelPubTest, false)
