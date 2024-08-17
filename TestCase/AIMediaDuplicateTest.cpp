#include <QCoreApplication>
#include <QtTest>

#include "TestCase/pub/BeginToExposePrivateMember.h"
#include "Tools/AIMediaDuplicate.h"
#include "TestCase/pub/EndToExposePrivateMember.h"

const QString AI_MEDIA_DUPLICATE_DIR_EMPTY = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_AIMediaDuplicate/empty");
const QString AI_MEDIA_DUPLICATE_DIR_FOLDER_1 = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_AIMediaDuplicate/folder_1");
const QString AI_MEDIA_DUPLICATE_DIR_NO_MEDIA = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_AIMediaDuplicate/no_media");

class AIMediaDuplicateTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}

  void init() {}
  void cleanup() {}

  void test_Basic() { QCOMPARE("123", GetEffectiveName("123")); }

  void test_SimplePath() { QCOMPARE("B/C.ext", GetEffectiveName("C:/A/B/C.ext")); }

  void test_VideosPath() { QCOMPARE("A/Videos/C.ext", GetEffectiveName("C:/A/Videos/C.ext")); }

  void test_VideoPath() { QCOMPARE("A/Video/C.ext", GetEffectiveName("C:/A/Video/C.ext")); }

  void test_VidPath() { QCOMPARE("A/Vid/C.ext", GetEffectiveName("C:/A/Vid/C.ext")); }

  void test_VIDEO_TSPath() { QCOMPARE("A/VIDEO_TS/C.ext", GetEffectiveName("C:/A/VIDEO_TS/C.ext")); }

  void test_ignoreCasePath() {
    QCOMPARE("A/VID/C.ext", GetEffectiveName("C:/A/VID/C.ext"));
    QCOMPARE("A/video_ts/C.ext", GetEffectiveName("C:/A/video_ts/C.ext"));
  }

  void test_C_DISK_DRIVER_NAME_2_TableName() {
    QCOMPARE(GetTableName("C:/DISK/LD2"), "DISK_LD2");
    QCOMPARE(GetTableName("C:/DISK/LDBKP"), "DISK_LDBKP");
    QCOMPARE(GetTableName("C:/DISK/F24BKP"), "DISK_F24BKP");
  }

  void test_DRIVER_LETTER_2_TableName() {
    QCOMPARE(GetTableName("C:"), "C_");
    QCOMPARE(GetTableName("C:/"), "C__");
    QCOMPARE(GetTableName("C:/A"), "A");
    QCOMPARE(GetTableName("E:/P/Hetero/jerk"), "P_Hetero_jerk");
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

  void test_ScanAPath_ok() {
    auto& aid = AIMediaDuplicate::GetInst();
    QVERIFY(aid.DropTables({}, true) != -1);
    bool scanRet = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_EMPTY, true);
    QVERIFY(scanRet);
    QCOMPARE(aid.GetTablesCnt(), 1);
  }

  void test_ScanPaths_ok() {
    auto& aid = AIMediaDuplicate::GetInst();
    QVERIFY(aid.DropTables({}, true) != -1);
    bool scanRet = aid.ScanALocation(AI_MEDIA_DUPLICATE_DIR_FOLDER_1, true);
    QVERIFY(scanRet);
    QCOMPARE(aid.GetTablesCnt(), 1);
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

//QTEST_MAIN(AIMediaDuplicateTest)
#include "AIMediaDuplicateTest.moc"
