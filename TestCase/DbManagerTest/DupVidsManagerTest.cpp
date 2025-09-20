#include <QCoreApplication>
#include <QtTest>

#include "BeginToExposePrivateMember.h"
#include "DupVidsManager.h"
#include "EndToExposePrivateMember.h"
#include "FileSystemTestSuite.h"

class DupVidsManagerTest : public FileSystemTestSuite {
  Q_OBJECT
 public:
  DupVidsManagerTest() : FileSystemTestSuite("TestEnv_AIMediaDuplicate", false) {}
  const FileSystemHelper m_rootHelper{mTestPath};
  const QString AI_MEDIA_DUPLICATE_DIR_EMPTY = mTestPath + "/empty";
  const QString AI_MEDIA_DUPLICATE_DIR_FOLDER_1 = mTestPath + "/folder_1";
  const QString AI_MEDIA_DUPLICATE_DIR_NO_MEDIA = mTestPath + "/no_media";

  const QString DUP_VID_DB{DupVidsManager::GetAiDupVidDbPath()};
  const QString DUP_VID_CONN = DupVidsManager::CONNECTION_NAME;
 private slots:
  void initTestCase() {
    QCOMPARE(DupVidsManager::DropDatabaseForTest(DUP_VID_DB, false), true);
    // empty
    // folder_1{movie 1 duplicate.mp4, movie 2 duplicate.mp4, movie 3 unique.mkv},
    // no_media{any text.txt}
    std::string size102Str(102, '0');
    QString s102{size102Str.c_str()};
    m_rootHelper << FileSystemNode{"empty"} << FileSystemNode{"folder_1"} << FileSystemNode{"no_media"};
    m_rootHelper.GetSubHelper("folder_1") << FileSystemNode{"movie 1 duplicate.mp4", false, "012345678901234567890123"}
                                          << FileSystemNode{"movie 2 duplicate.mp4", false, "012345678901234567890123"}
                                          << FileSystemNode{"movie 3 unique.mkv", false, s102};
    m_rootHelper.GetSubHelper("no_media").GetSubHelper("folder") << FileSystemNode{"any text.txt", false, ""};
  }

  void cleanupTestCase() {  //
    m_rootHelper.EraseFileSystemTree(true);
    DupVidsManager::DropDatabaseForTest(DUP_VID_DB, false);
  }

  void test_C_DISK_DRIVER_NAME_2_TableName() {
    QCOMPARE(GetTableName("C:/DISK/LD2"), "C__DISK_LD2");
    QCOMPARE(GetTableName("C:/DISK/LDBKP"), "C__DISK_LDBKP");
    QCOMPARE(GetTableName("C:/DISK/F24BKP"), "C__DISK_F24BKP");

#ifdef _WIN32
    QCOMPARE(TableName2Path("C__DISK_LD2"), "C:/DISK/LD2");
    QCOMPARE(TableName2Path("C__DISK_LDBKP"), "C:/DISK/LDBKP");
    QCOMPARE(TableName2Path("C__DISK_F24BKP"), "C:/DISK/F24BKP");
#endif
  }

  void test_DRIVER_LETTER_2_TableName() {
    QCOMPARE(GetTableName("C:"), "C_");
    QCOMPARE(GetTableName("C:/"), "C__");
    QCOMPARE(GetTableName("C:/A"), "C__A");
    QCOMPARE(GetTableName("E:/P/Hetero/jerk"), "E__P_Hetero_jerk");

    QCOMPARE(GetTableName("/home"), "_home");
    QCOMPARE(GetTableName("/home/costa/Document"), "_home_costa_Document");
  }

  void test_DropAllTables_ok() {
    DupVidsManager::DropAllTablesForTest(DUP_VID_CONN);
    auto& aid = DupVidsManager::GetInst();
    QCOMPARE(aid.GetTablesCnt(), 0);
  }

  void test_ScanAnEmptyPath_ok() {
    DupVidsManager::DropAllTablesForTest(DUP_VID_CONN);
    auto& aid = DupVidsManager::GetInst();
    bool scanRet = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_EMPTY);
    QVERIFY(scanRet);
    QCOMPARE(aid.GetTablesCnt(), 1);
  }

  void test_ScanAPaths_ok() {
    DupVidsManager::DropAllTablesForTest(DUP_VID_CONN);
    auto& aid = DupVidsManager::GetInst();
    bool scanRet = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_FOLDER_1);
    QVERIFY(scanRet);
    QCOMPARE(aid.GetTablesCnt(), 1);
  }

  void test_ScanPathContainDuplicates_ok() {
    DupVidsManager::DropAllTablesForTest(DUP_VID_CONN);
    auto& aid = DupVidsManager::GetInst();
    bool scanRet = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_FOLDER_1);
    QVERIFY(scanRet);
    QCOMPARE(aid.GetTablesCnt(), 1);
    const QString& tableName = GetTableName(AI_MEDIA_DUPLICATE_DIR_FOLDER_1);
    const int fillCnt = aid.FillHashFieldIfSizeConflict(AI_MEDIA_DUPLICATE_DIR_FOLDER_1);
    QCOMPARE(fillCnt, 2);

    QSqlDatabase db = QSqlDatabase::database(DupVidsManager::CONNECTION_NAME);
    QSqlQuery query(db);
    const bool qryRet = query.exec(QString("SELECT `FIRST_1024_HASH`, `SIZE` FROM `%1` WHERE `FIRST_1024_HASH` IS NOT NULL").arg(tableName));
    int cnt = 0;
    QSet<QString> md5s;
    // both are 24byte: movie 1 duplicate.mp4; movie 2 duplicate.mp4;
    while (query.next()) {
      md5s << query.value(0).toString();
      QCOMPARE(query.value(1).toLongLong(), 24);
      ++cnt;
    }
    QCOMPARE(cnt, 2);
    QCOMPARE(md5s.size(), 1);
  }

  void test_ScanNoMediaPaths_ok() {
    DupVidsManager::DropAllTablesForTest(DUP_VID_CONN);
    auto& aid = DupVidsManager::GetInst();
    bool scanRet = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_NO_MEDIA);
    QVERIFY(scanRet);
    QCOMPARE(aid.GetTablesCnt(), 1);
  }

  void test_ScanLocations_DropATable_ok() {
    DupVidsManager::DropAllTablesForTest(DUP_VID_CONN);
    auto& aid = DupVidsManager::GetInst();
    const int scansRet = aid.ScanLocations({AI_MEDIA_DUPLICATE_DIR_EMPTY, AI_MEDIA_DUPLICATE_DIR_FOLDER_1, AI_MEDIA_DUPLICATE_DIR_NO_MEDIA});
    QCOMPARE(scansRet, 3);
    QCOMPARE(aid.GetTablesCnt(), 3);

    const QString delTableName = GetTableName(AI_MEDIA_DUPLICATE_DIR_EMPTY);
    QCOMPARE(aid.DropTables({delTableName}), 1);
    QCOMPARE(aid.GetTablesCnt(), 2);
  }
};

#include "DupVidsManagerTest.moc"
REGISTER_TEST(DupVidsManagerTest, false)
