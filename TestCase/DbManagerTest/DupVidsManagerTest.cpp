#include <QCoreApplication>
#include <QtTest>

#include "BeginToExposePrivateMember.h"
#include "DupVidsManager.h"
#include "EndToExposePrivateMember.h"
#include "FileSystemTestSuite.h"

#include "PlainTestSuite.h"
#include "TDir.h"
#include "VideoTestPrecoditionTools.h"
#include <QSqlRecord>

class DupVidsManagerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  QDir mDir{tDir.path()};
  const QString AI_MEDIA_DUPLICATE_DIR_EMPTY = tDir.itemPath("/empty");
  const QString AI_MEDIA_DUPLICATE_DIR_FOLDER_1 = tDir.itemPath("/folder_1");
  const QString AI_MEDIA_DUPLICATE_DIR_NO_MEDIA = tDir.itemPath("/no_media");

  const QString tb1{GetTableName(AI_MEDIA_DUPLICATE_DIR_EMPTY)};
  const QString tb2{GetTableName(AI_MEDIA_DUPLICATE_DIR_FOLDER_1)};
  const QString tb3{GetTableName(AI_MEDIA_DUPLICATE_DIR_NO_MEDIA)};

  const VideoTestPrecoditionTools& tool{VideoTestPrecoditionTools::getInst()};
 private slots:
  void initTestCase() {
    QCOMPARE(DupVidsManager::DropDatabaseForTest(tool.DUP_VID_DB, false), true);
    // empty
    // folder_1{movie 1 duplicate.mp4, movie 2 duplicate.mp4, movie 3 unique.mkv},
    // no_media{any text.txt}
    const QString s24{"012345678901234567890123"};

    std::string size102Str(102, '0');
    const QString s102{size102Str.c_str()};

    QList<FsNodeEntry> nodes{
        {"empty", true, ""},
        {"folder_1/movie 1 duplicate.mp4", false, s24.toUtf8().constData()},
        {"folder_1/movie 2 duplicate.mp4", false, s24.toUtf8().constData()},
        {"folder_1/movie 3 unique.mkv", false, s102.toUtf8().constData()},
        {"no_media/folder/any text.txt", false, ""},
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());
  }

  void cleanupTestCase() {  //
    DupVidsManager::DropDatabaseForTest(tool.DUP_VID_DB, false);
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

  void test_ScanPath_DropTable_ok() {
    DupVidsManager::DropAllTablesForTest(tool.DUP_VID_CONN);

    DupVidsManager aid;
    aid.setSkipGetVideosDuration(true);
    QCOMPARE(aid.GetTablesCnt(), 0);  // no tables

    {
      DupVidMetaInfoList vidInfoListWith2RandomElement{{}, {}};
      QCOMPARE(vidInfoListWith2RandomElement.size(), 2);
      aid.ReadSpecifiedTables2List({}, vidInfoListWith2RandomElement);
      QCOMPARE(vidInfoListWith2RandomElement.size(), 0);
    }

    // 0. non exist path, will not create a table
    bool scan0Ret = aid.ScanALocation("NonExists path");
    QCOMPARE(scan0Ret, false);        // no tables
    QCOMPARE(aid.GetTablesCnt(), 0);  // no tables

    // 1. Scan Tables ok, table exists ok
    bool scan1Ret = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_EMPTY);  // empty path, will also create a table
    QVERIFY(scan1Ret);
    QCOMPARE(aid.GetTablesCnt(), 1);
    QCOMPARE(aid.IsTableExist(tb1), true);
    QCOMPARE(aid.IsTableExist(tb2), false);
    QCOMPARE(aid.IsTableExist(tb3), false);

    bool scan2Ret = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_FOLDER_1);
    QVERIFY(scan2Ret);
    QCOMPARE(aid.GetTablesCnt(), 2);

    bool scan3Ret = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_FOLDER_1);  // skip again, table already exist
    QVERIFY(scan3Ret);
    QCOMPARE(aid.GetTablesCnt(), 2);

    bool scan4Ret = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_NO_MEDIA);  // path contains no videos, will also create a table
    QVERIFY(scan4Ret);
    QCOMPARE(aid.GetTablesCnt(), 3);

    const int scans3PathRet = aid.ScanLocations({AI_MEDIA_DUPLICATE_DIR_EMPTY,     // 3 paths already in table
                                                 AI_MEDIA_DUPLICATE_DIR_FOLDER_1,  //
                                                 AI_MEDIA_DUPLICATE_DIR_NO_MEDIA});
    QCOMPARE(scans3PathRet, 3);
    QCOMPARE(aid.GetTablesCnt(), 3);
    QCOMPARE(aid.IsTableExist(tb1), true);
    QCOMPARE(aid.IsTableExist(tb2), true);
    QCOMPARE(aid.IsTableExist(tb3), true);

    // 2 Tables Name to videos count ok
    DupVidTableName2RecordCountList tableName2VideosCnt = aid.TableName2Cnt();
    std::sort(tableName2VideosCnt.begin(), tableName2VideosCnt.end());
    DupVidTableName2RecordCountList expectTableName2VideosCnt{{tb1, 0}, {tb2, 3}, {tb3, 0}};
    std::sort(expectTableName2VideosCnt.begin(), expectTableName2VideosCnt.end());  //
    QCOMPARE(tableName2VideosCnt, expectTableName2VideosCnt);

    // 3 Drop Tables ok
    QCOMPARE(aid.DropTables({tb1}), 1);  // drop 1 table ok
    QCOMPARE(aid.GetTablesCnt(), 2);

    QCOMPARE(aid.DropTables({tb2, tb3}), 2);  // drop 2 tables ok
    QCOMPARE(aid.GetTablesCnt(), 0);

    {  // bidirectional conversion between table names and file paths ok
      const QString tableNameTsPath = GetTableName(tool.TS_FILE_MERGER_SAMPLE_PATH);
      const QString pathFromTable = TableName2Path(tableNameTsPath);
      QCOMPARE(pathFromTable, tool.TS_FILE_MERGER_SAMPLE_PATH);

      const QString qryAll = QString{"SELECT * from `%1`;"}.arg(tableNameTsPath);
      bool bScanTs = aid.ScanALocation(tool.TS_FILE_MERGER_SAMPLE_PATH);
      QCOMPARE(bScanTs, true);

      QList<QSqlRecord> expectAllRecords;
      QCOMPARE(aid.QueryForTest(qryAll, expectAllRecords), true);
      // 4. rebuild ok
      {
        const QStringList rebTables{tableNameTsPath};
        QCOMPARE(aid.RebuildTables(rebTables), rebTables.size());

        QList<QSqlRecord> allRecordsAfterRebuild;
        QCOMPARE(aid.QueryForTest(qryAll, allRecordsAfterRebuild), true);
        QCOMPARE(allRecordsAfterRebuild, expectAllRecords);

        DupVidMetaInfoList vidInfoList;
        aid.ReadSpecifiedTables2List(rebTables, vidInfoList);
        QCOMPARE(vidInfoList.size(), expectAllRecords.size());
      }

      {  // 5. AuditTables ok. videos not exist will be removed from database
        // 1 file get renamed, audit repair 1 one (delete this record in table directly)
        QCOMPARE(QDir{tool.TS_FILE_MERGER_SAMPLE_PATH}.exists("File need to merge seg-1-v1-a1.ts"), true);
        AutoRollbackRename oneVideoGetRenamed{tool.TS_FILE_MERGER_SAMPLE_PATH, "File need to merge seg-1-v1-a1.ts",
                                              "File need to merge seg-1-v1-a1 renamed to.ts"};
        QCOMPARE(oneVideoGetRenamed.Execute(), true);
        QCOMPARE(QDir{tool.TS_FILE_MERGER_SAMPLE_PATH}.exists("File need to merge seg-1-v1-a1.ts"), false);
        QCOMPARE(aid.AuditTables({tableNameTsPath}), 1);

        QList<QSqlRecord> allRecordsAfterAudit;
        QCOMPARE(aid.QueryForTest(qryAll, allRecordsAfterAudit), true);
        QCOMPARE(allRecordsAfterAudit.size(), expectAllRecords.size() - 1);  // get delete one record
      }
    }
    QCOMPARE(QDir{tool.TS_FILE_MERGER_SAMPLE_PATH}.exists("File need to merge seg-1-v1-a1.ts"), true);
  }

  void test_ScanPathContainDuplicates_ok() {
    DupVidsManager::DropAllTablesForTest(tool.DUP_VID_CONN);

    DupVidsManager aid;
    aid.setSkipGetVideosDuration(true);
    bool scanRet = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_FOLDER_1);
    QVERIFY(scanRet);
    QCOMPARE(aid.GetTablesCnt(), 1);
    const QString& tableName = GetTableName(AI_MEDIA_DUPLICATE_DIR_FOLDER_1);

    QCOMPARE(aid.isSkipGetVideosDuration(), true);
    {  // 3 videos in table and all duration is zero
      QCOMPARE(aid.CountRow(tableName), 3);

      const QString qryDurationAllZeroCmd = QString("SELECT `DURATION` FROM `%1`").arg(tableName);
      QList<QSqlRecord> durationAllZeroRecords;
      QVERIFY(aid.QueryForTest(qryDurationAllZeroCmd, durationAllZeroRecords));
      QCOMPARE(durationAllZeroRecords.size(), 3);
      for (const auto& record : durationAllZeroRecords) {
        QCOMPARE(record.value(0).toInt(), 0);
      }
    }

    {                                                                                      // 2 videos records has hash field filled. and size=24bytes
      QCOMPARE(aid.FillHashFieldIfSizeConflict("RandomPathThatHasNoTableInDatabase"), 0);  // 0 record `FIRST_1024_HASH` field get filled
      QCOMPARE(aid.FillHashFieldIfSizeConflict(AI_MEDIA_DUPLICATE_DIR_FOLDER_1), 2);       // 2 record `FIRST_1024_HASH` field get filled
      const QString qryhashNotNullCmd = QString("SELECT `FIRST_1024_HASH`, `SIZE` FROM `%1` WHERE `FIRST_1024_HASH` IS NOT NULL").arg(tableName);

      QList<QSqlRecord> hasNotNullrecords;
      QVERIFY(aid.QueryForTest(qryhashNotNullCmd, hasNotNullrecords));
      QCOMPARE(hasNotNullrecords.size(), 2);  // two record and 1 md5
      QSet<QString> md5s;
      // 24 bytes files: movie 1 duplicate.mp4; movie 2 duplicate.mp4; => will fill FIRST_1024_HASH;
      // 102 bytes files: movie 3 unique.mkv; => will not fill FIRST_1024_HASH
      for (const auto& record : hasNotNullrecords) {
        md5s << record.value(0).toString();
        QCOMPARE(record.value(1).toLongLong(), 24);
      }
      QCOMPARE(md5s.size(), 1);
    }
  }
};

#include "DupVidsManagerTest.moc"
REGISTER_TEST(DupVidsManagerTest, false)
