#include <QCoreApplication>
#include <QtTest>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include "PlainTestSuite.h"
#include "TDir.h"
#include <QDebug>
// ok 3e435dc4_72d5_4709_8bea_94da297050a9
// nok 20ff7c6b_2d89_4fd0_aafe_8db6eb547e79
#define TABLE_NAME_IN_QUOTE "'20ff7c6b_2d89_4fd0_aafe_8db6eb547e79'"

class BasicSqliteTest : public PlainTestSuite {
  Q_OBJECT
public:
  BasicSqliteTest() : PlainTestSuite{} {}
  TDir mDir;
  const QString mWorkPath{mDir.path()};
  const QString mConnName{"BASIC_SQLITE_CONN"};
  const QString mDbName{"BASIC_SQLITE_TEST.db"};
  QSqlDatabase GetSqlDataBase(bool open = true) {
    QSqlDatabase db;
    if (QSqlDatabase::contains(mConnName)) {
      db = QSqlDatabase::database(mConnName, true);
    } else {
      db = QSqlDatabase::addDatabase("QSQLITE", mConnName);
      db.setDatabaseName(mWorkPath + '/' + mDbName);
    }
    if (open && !db.isOpen()) {
      if (!db.open()) {
        qDebug("Open failed[%s]", qPrintable(db.lastError().text()));
      }
    }
    return db;
  }

  void initTestCase() {
    auto db = GetSqlDataBase();
    QVERIFY(db.isValid());
  }

private slots:
  void test_create_table_ok() {
    const QString CREATE_SCORE_TABLE_TEMPLATE{"CREATE TABLE IF NOT EXISTS " TABLE_NAME_IN_QUOTE " ("
                                              "   `NAME` NCHAR(260) NOT NULL,"
                                              "   `MATH` integer DEFAULT 0,"
                                              "   `ENGLISH` integer DEFAULT 0,"
                                              "    PRIMARY KEY (NAME)"
                                              "    );"};
    auto db = GetSqlDataBase();
    QVERIFY(db.isValid());
    QVERIFY(db.isValid());
    QSqlQuery query{CREATE_SCORE_TABLE_TEMPLATE, db};
    bool createTblRet = query.exec();
    qDebug() << query.executedQuery();
    qDebug() << query.lastError();
    qDebug() << db.tables();
    QVERIFY(createTblRet);
  }
  void test_insert_into_table_ok() {
    auto db = GetSqlDataBase();
    QVERIFY(db.isValid());

    // 插入测试数据
    QSqlQuery query(db);
    QVERIFY(query.prepare("INSERT INTO " TABLE_NAME_IN_QUOTE " (`NAME`, `MATH`, `ENGLISH`) VALUES (?, ?, ?)"));

    query.addBindValue("Alice");
    query.addBindValue(90);
    query.addBindValue(85);
    QVERIFY(query.exec());

    query.addBindValue("Bob");
    query.addBindValue(75);
    query.addBindValue(80);
    QVERIFY(query.exec());

    // 验证数据是否插入成功
    QVERIFY(query.exec("SELECT COUNT(*) FROM " TABLE_NAME_IN_QUOTE " "));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 2);
  }
  void test_delete_where_table_ok() {
    auto db = GetSqlDataBase();
    QVERIFY(db.isValid());

    // 删除特定记录
    QSqlQuery query(db);
    QVERIFY(query.prepare("DELETE FROM " TABLE_NAME_IN_QUOTE " WHERE `NAME` = ?"));
    query.addBindValue("Alice");
    QVERIFY(query.exec());

    // 验证删除是否成功
    QVERIFY(query.exec("SELECT COUNT(*) FROM " TABLE_NAME_IN_QUOTE " WHERE `NAME` = 'Alice'"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);

    // 验证其他记录是否还存在
    QVERIFY(query.exec("SELECT COUNT(*) FROM " TABLE_NAME_IN_QUOTE " WHERE `NAME` = 'Bob'"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 1);
  }
  void test_drop_table_ok() {
    auto db = GetSqlDataBase();
    QVERIFY(db.isValid());

    // 删除表
    QSqlQuery query(db);
    QVERIFY(query.exec("DROP TABLE IF EXISTS " TABLE_NAME_IN_QUOTE " "));

    // 验证表是否已删除
    QVERIFY(query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name= " TABLE_NAME_IN_QUOTE " "));
    QVERIFY(!query.next());
  }
};

#include "BasicSqliteTest.moc"
REGISTER_TEST(BasicSqliteTest, false)
