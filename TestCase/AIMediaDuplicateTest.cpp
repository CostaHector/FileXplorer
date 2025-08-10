#include <QCoreApplication>
#include <QtTest>

#include "BeginToExposePrivateMember.h"
#include "AIMediaDuplicate.h"
#include "EndToExposePrivateMember.h"
#include "FileSystemTestSuite.h"

class AIMediaDuplicateTest : public FileSystemTestSuite {
  Q_OBJECT
 public:
  AIMediaDuplicateTest() : FileSystemTestSuite("TestEnv_AIMediaDuplicate", false, false) {}
  const FileSystemHelper m_rootHelper{mTestPath};
  const QString AI_MEDIA_DUPLICATE_DIR_EMPTY = mTestPath + "/empty";
  const QString AI_MEDIA_DUPLICATE_DIR_FOLDER_1 = mTestPath + "/folder_1";
  const QString AI_MEDIA_DUPLICATE_DIR_NO_MEDIA = mTestPath + "/no_media";

 private slots:
  void cleanupTestCase() {  //
    m_rootHelper.EraseFileSystemTree(true);
  }

  void initTestCase() {
    // empty, folder_1{movie 1 duplicate.mp4, movie 2 duplicate.mp4, movie 3 unique.mkv}, no_media{any text.txt}
    std::string size102Str(102, '0');
    QString s102{size102Str.c_str()};
    m_rootHelper << FileSystemNode{"empty"} << FileSystemNode{"folder_1"} << FileSystemNode{"no_media"};
    m_rootHelper.GetSubHelper("folder_1") << FileSystemNode{"movie 1 duplicate.mp4", false, "012345678901234567890123"} << FileSystemNode{"movie 2 duplicate.mp4", false, "012345678901234567890123"}
                                          << FileSystemNode{"movie 3 unique.mkv", false, s102};
    m_rootHelper.GetSubHelper("no_media").GetSubHelper("folder") << FileSystemNode{"any text.txt", false, ""};
  }
  void init() {
    AIMediaDuplicate::SKIP_GETTER_DURATION = true;
    AIMediaDuplicate::IS_TEST = true;
  }

  void test_Basic() {  //
    QCOMPARE("123", GetEffectiveName("123"));
  }

  void test_SimplePath() {  //
    QCOMPARE("B/C.ext", GetEffectiveName("C:/A/B/C.ext"));
  }

  void test_VideosPath() {  //
    QCOMPARE("A/Videos/C.ext", GetEffectiveName("C:/A/Videos/C.ext"));
  }

  void test_VideoPath() {  //
    QCOMPARE("A/Video/C.ext", GetEffectiveName("C:/A/Video/C.ext"));
  }

  void test_VidPath() {  //
    QCOMPARE("A/Vid/C.ext", GetEffectiveName("C:/A/Vid/C.ext"));
  }

  void test_VIDEO_TSPath() {  //
    QCOMPARE("A/VIDEO_TS/C.ext", GetEffectiveName("C:/A/VIDEO_TS/C.ext"));
  }

  void test_ignoreCasePath() {
    QCOMPARE("A/VID/C.ext", GetEffectiveName("C:/A/VID/C.ext"));
    QCOMPARE("A/video_ts/C.ext", GetEffectiveName("C:/A/video_ts/C.ext"));
  }

  void test_C_DISK_DRIVER_NAME_2_TableName() {
    QCOMPARE(GetTableName("C:/DISK/LD2"), "C__DISK_LD2");
    QCOMPARE(GetTableName("C:/DISK/LDBKP"), "C__DISK_LDBKP");
    QCOMPARE(GetTableName("C:/DISK/F24BKP"), "C__DISK_F24BKP");

    QCOMPARE(TableName2Path("C__DISK_LD2"), "C:/DISK/LD2");
    QCOMPARE(TableName2Path("C__DISK_LDBKP"), "C:/DISK/LDBKP");
    QCOMPARE(TableName2Path("C__DISK_F24BKP"), "C:/DISK/F24BKP");
  }

  void test_DRIVER_LETTER_2_TableName() {
    QCOMPARE(GetTableName("C:"), "C_");
    QCOMPARE(GetTableName("C:/"), "C__");
    QCOMPARE(GetTableName("C:/A"), "C__A");
    QCOMPARE(GetTableName("E:/P/Hetero/jerk"), "E__P_Hetero_jerk");
  }

  void test_noDriverLetter_TableName() {
    QCOMPARE(GetTableName("/home"), "_home");
    QCOMPARE(GetTableName("/home/costa/Document"), "_home_costa_Document");
  }

  void test_DropAllTables_ok() {
    auto& aid = AIMediaDuplicate::GetInst();
    QVERIFY(aid.DropTables({}, true) != -1);
    QCOMPARE(aid.GetTablesCnt(), 0);
  }

  void test_ScanAnEmptyPath_ok() {
    auto& aid = AIMediaDuplicate::GetInst();
    QVERIFY(aid.DropTables({}, true) != -1);
    bool scanRet = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_EMPTY, true);
    QVERIFY(scanRet);
    QCOMPARE(aid.GetTablesCnt(), 1);
  }

  void test_ScanAPaths_ok() {
    auto& aid = AIMediaDuplicate::GetInst();
    QVERIFY(aid.DropTables({}, true) != -1);
    bool scanRet = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_FOLDER_1, true);
    QVERIFY(scanRet);
    QCOMPARE(aid.GetTablesCnt(), 1);
  }

  void test_ScanPathContainDuplicates_ok() {
    auto& aid = AIMediaDuplicate::GetInst();
    QVERIFY(aid.DropTables({}, true) != -1);
    bool scanRet = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_FOLDER_1, true);
    QVERIFY(scanRet);
    QCOMPARE(aid.GetTablesCnt(), 1);
    const QString& tableName = GetTableName(AI_MEDIA_DUPLICATE_DIR_FOLDER_1);
    const int fillCnt = aid.FillHashFieldIfSizeConflict(AI_MEDIA_DUPLICATE_DIR_FOLDER_1);
    QCOMPARE(fillCnt, 2);

    QSqlDatabase db = QSqlDatabase::database(AIMediaDuplicate::CONNECTION_NAME);
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
    auto& aid = AIMediaDuplicate::GetInst();
    QVERIFY(aid.DropTables({}, true) != -1);
    bool scanRet = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_NO_MEDIA, true);
    QVERIFY(scanRet);
    QCOMPARE(aid.GetTablesCnt(), 1);
  }

  void test_ScanLocations_DropATable_ok() {
    auto& aid = AIMediaDuplicate::GetInst();
    QVERIFY(aid.DropTables({}, true) != -1);
    const int scansRet = aid.ScanLocations({AI_MEDIA_DUPLICATE_DIR_EMPTY, AI_MEDIA_DUPLICATE_DIR_FOLDER_1, AI_MEDIA_DUPLICATE_DIR_NO_MEDIA}, true);
    QCOMPARE(scansRet, 3);
    QCOMPARE(aid.GetTablesCnt(), 3);

    const QString delTableName = GetTableName(AI_MEDIA_DUPLICATE_DIR_EMPTY);
    QCOMPARE(aid.DropTables({delTableName}, false), 1);
    QCOMPARE(aid.GetTablesCnt(), 2);
  }
};

#include "AIMediaDuplicateTest.moc"
AIMediaDuplicateTest g_AIMediaDuplicateTest;
