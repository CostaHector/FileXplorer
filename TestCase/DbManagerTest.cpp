#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
#include "Tools/FileDescriptor/DbManager.h"

const QString rootpath = QFileInfo(__FILE__).absolutePath() + "/test/TestEnv_VideosDurationGetter";
const QString dbName = rootpath + "/FD_MOVIE_DB_CONN.db";

class DbManagerTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void cleanup() {
    if (QFile{dbName}.exists()) {
      QFile{dbName}.remove();
    }
  }

  void test_createDatabaseAndTable() {
    FdBasedDb dbManager{dbName, "FD_MOVIE_DB_CONN"};
    QVERIFY(dbManager.CreateDatabase());
    QVERIFY(dbManager.CreateTable("VOLUME_E", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(dbManager.CreateTable("AGED", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());
    QCOMPARE(dbManager.DropTable("^VOLUME_E$"), 1);
    QCOMPARE(dbManager.DropTable("^AGED$"), 1);
    QCOMPARE(dbManager.DropTable("^VOLUME_E$"), 0);
    QCOMPARE(dbManager.DropTable("^AGED$"), 0);
    QVERIFY(dbManager.DeleteDatabase());
    QVERIFY(!QFile{dbName}.exists());
  }

  void test_ReadADirectory_invalid() {
    FdBasedDb dbManager{dbName, "FD_MOVIE_DB_CONN"};
    // no such table
    QCOMPARE(dbManager.ReadADirectory(rootpath, "VOLUME_E"), FdBasedDb::FD_PREPARE_FAILED);

    QVERIFY(dbManager.CreateTable("VOLUME_E", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created

    // path not exist
    const QString inexistPath{rootpath + "_not_exist_Path"};
    QVERIFY(!QFileInfo{inexistPath}.isDir());

    QCOMPARE(dbManager.ReadADirectory(inexistPath, "VOLUME_E"), FdBasedDb::FD_NOT_DIR);

    QVERIFY(dbManager.DeleteDatabase());
    QVERIFY(!QFile{dbName}.exists());
  }

  void test_ReadADirectory() {
    // precondition
    QDir dir{rootpath, "", QDir::SortFlag::Name, QDir::Filter::Files};
    dir.setNameFilters({"*.mp4"});
    const QStringList& vids = dir.entryList();
    QCOMPARE(vids.size(), 5);
    QVERIFY(!QFile{dbName}.exists());

    FdBasedDb dbManager{dbName, "FD_MOVIE_DB_CONN"};
    QVERIFY(dbManager.CreateTable("VOLUME_E", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created
    QCOMPARE(dbManager.ReadADirectory(rootpath, "VOLUME_E"), vids.size());

    QVERIFY(dbManager.DeleteDatabase());
    QVERIFY(!QFile{dbName}.exists());
  }
};

DbManagerTest g_DbManagerTest;
#include "DbManagerTest.moc"
