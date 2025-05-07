#include <QCoreApplication>
#include <QtTest>
#include "TestCase/PathRelatedTool.h"
#include "TestCase/pub/MyTestSuite.h"
#include "TestCase/pub/GlbDataProtect.h"
#include "TestCase/pub/OnScopeExit.h"
#include "Tools/FileDescriptor/FdBasedDb.h"
#include "Tools/FileDescriptor/TableFields.h"
#include "Tools/JsonFileHelper.h"
#include "public/PublicMacro.h"
#include <QSqlRecord>

const QString rootpath = TestCaseRootPath() + "/test/TestEnv_VideosDurationGetter";
const QString dbName = TestCaseRootPath() + "/FD_MOVIE_DB_CONN.db";

class FdBasedDbTest : public MyTestSuite {
  Q_OBJECT
 public:
  FdBasedDbTest() : MyTestSuite{false} {}
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
    QCOMPARE(dbManager.DropTable("VOLUME_E"), 1);  // by default, full match
    QCOMPARE(dbManager.DropTable("AGED"), 1);
    QCOMPARE(dbManager.DropTable("VOLUME_E"), 0);
    QCOMPARE(dbManager.DropTable("AGED"), 0);
    QVERIFY(dbManager.DeleteDatabase());
    QVERIFY(!QFile{dbName}.exists());
  }

  void test_ReadADirectory_invalid() {
    // precondition
    QVERIFY(!QFile{dbName}.exists());  // should created
    FdBasedDb dbManager{dbName, "FD_MOVIE_DB_CONN"};

    // Procedure
    QVERIFY(dbManager.CreateTable("VOLUME_E", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should table created ok

    // 1. path not exist
    const QString inexistPath{rootpath + "_not_exist_Path"};
    QVERIFY(!QFileInfo{inexistPath}.isDir());
    QCOMPARE(dbManager.ReadADirectory("VOLUME_E", inexistPath), FD_NOT_DIR);

    // 2. invalid table name exist
    QVERIFY(!QFileInfo{inexistPath}.isDir());
    QCOMPARE(dbManager.ReadADirectory("", rootpath), FD_TABLE_NAME_INVALID);

    // 3. inexist table
    const QString& inexistTableName{"InexistTable"};
    QVERIFY(!QFileInfo{inexistPath}.isDir());
    QCOMPARE(dbManager.ReadADirectory(inexistTableName, rootpath), FD_TABLE_INEXIST);

    QVERIFY(dbManager.DeleteDatabase());
    QVERIFY(!QFile{dbName}.exists());
  }

  void test_ReadADirectory_incremental() {
    GlbDataProtect<decltype(FdBasedDb::VIDEOS_FILTER)> bkp{FdBasedDb::VIDEOS_FILTER};
    QStringList MP4_TYPES{"*.mp4"};
    QStringList MP4_MKV_TYPES{"*.mp4", "*.mkv"};

    // precondition
    QDir dir{rootpath, "", QDir::SortFlag::Name, QDir::Filter::Files};
    dir.setNameFilters(MP4_TYPES);
    const QStringList& vids = dir.entryList();
    QCOMPARE(vids.size(), 5);  // only 5 mp4 videos

    dir.setNameFilters(MP4_MKV_TYPES);
    const QStringList& vid2s = dir.entryList();
    QCOMPARE(vid2s.size(), 6);  // only 5 mp4 + 1 mkv videos
    QVERIFY(!QFile{dbName}.exists());

    // Procedure
    FdBasedDb dbManager{dbName, "FD_MOVIE_DB_CONN"};
    QVERIFY(dbManager.CreateTable("VOLUME_E", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created

    FdBasedDb::VIDEOS_FILTER = MP4_TYPES;
    QCOMPARE(dbManager.ReadADirectory("VOLUME_E", rootpath), 5);
    FdBasedDb::VIDEOS_FILTER = MP4_MKV_TYPES;
    QCOMPARE(dbManager.ReadADirectory("VOLUME_E", rootpath), 1);  // incremental

    // total count = 8
    using namespace MOVIE_TABLE;
    const QString qryWhereClause{QString(R"(`%1` like "%.mp4")").arg(ENUM_TO_STRING(Name))};
    QCOMPARE(dbManager.CountRow("VOLUME_E", qryWhereClause), 5);  // 5 *.mp4 removed
    QCOMPARE(dbManager.DeleteByWhereClause("VOLUME_E", qryWhereClause), 5);
    QCOMPARE(dbManager.CountRow("VOLUME_E"), 1);  // 1 *.mkv left

    QVERIFY(dbManager.DeleteDatabase());
    QVERIFY(!QFile{dbName}.exists());
  }

  void test_adt() {
    GlbDataProtect<decltype(FdBasedDb::VIDEOS_FILTER)> bkp{FdBasedDb::VIDEOS_FILTER};
    FdBasedDb::VIDEOS_FILTER = QStringList{"*.mkv"};  // 1 x mkv
    using namespace MOVIE_TABLE;

    // precondition
    QDir dir{rootpath, "", QDir::SortFlag::Name, QDir::Filter::Files};
    dir.setNameFilters(FdBasedDb::VIDEOS_FILTER);
    const QString beforeMKVFileName = "Big Buck Bunny SampleVideo_360x240_1mb 9s.mkv";
    const QString afterMKVFileName = "I am renamed Big Buck Bunny SampleVideo_360x240_1mb 9s.mkv";
    QVERIFY(dir.exists(beforeMKVFileName));
    QVERIFY(!dir.exists(afterMKVFileName));
    const QStringList& vids = dir.entryList();
    QCOMPARE(vids.size(), 1);  // only 1 mkv file
    QVERIFY(!QFile{dbName}.exists());

    FdBasedDb dbManager{dbName, "FD_MOVIE_DB_CONN"};
    QVERIFY(dbManager.CreateTable("VOLUME_E", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created

    // 1. insert only 1 mkv file
    VolumeUpdateResult adtRet{0};
    adtRet.Init();
    QCOMPARE(dbManager.Adt("VOLUME_E", rootpath, &adtRet), FD_OK);
    QCOMPARE(adtRet.insertCnt, 1);
    QCOMPARE(adtRet.deleteCnt, 0);
    QCOMPARE(adtRet.updateCnt, 0);
    // Also Check file name should only contain beforeMKVFileName
    QSet<QString> oldActualNames;
    QVERIFY(dbManager.QueryPK("VOLUME_E", ENUM_TO_STRING(Name), oldActualNames));
    QVERIFY(oldActualNames.contains(beforeMKVFileName));
    QVERIFY(!oldActualNames.contains(afterMKVFileName));

    // 2. 1 renamed mkv file and 5 x mp4 files
    QVERIFY(dir.rename(beforeMKVFileName, afterMKVFileName));
    ON_SCOPE_EXIT {
      if (dir.exists(afterMKVFileName)) {
        QVERIFY(dir.rename(afterMKVFileName, beforeMKVFileName));
      }
    };
    FdBasedDb::VIDEOS_FILTER = QStringList{"*.mp4", "*.mkv"};
    adtRet.Init();
    QCOMPARE(dbManager.Adt("VOLUME_E", rootpath, &adtRet), FD_OK);
    QCOMPARE(adtRet.insertCnt, 5);
    QCOMPARE(adtRet.deleteCnt, 0);
    QCOMPARE(adtRet.updateCnt, 1);
    QCOMPARE(dbManager.CountRow("VOLUME_E"), 6);
    // Also Check file name should also sync to new one afterMKVFileName
    QSet<QString> newActualNames;
    QVERIFY(dbManager.QueryPK("VOLUME_E", ENUM_TO_STRING(Name), newActualNames));
    QVERIFY(!newActualNames.contains(beforeMKVFileName));
    QVERIFY(newActualNames.contains(afterMKVFileName));

    // 3. 6 file removed
    FdBasedDb::VIDEOS_FILTER = QStringList{"NOTHING_NAME_CAN_MATCH"};
    adtRet.Init();
    QCOMPARE(dbManager.Adt("VOLUME_E", rootpath, &adtRet), FD_OK);
    QCOMPARE(adtRet.insertCnt, 0);
    QCOMPARE(adtRet.deleteCnt, 6);
    QCOMPARE(adtRet.updateCnt, 0);
    QCOMPARE(dbManager.CountRow("VOLUME_E"), 0);
  }

  void test_SetDuration() {
    GlbDataProtect<decltype(FdBasedDb::VIDEOS_FILTER)> bkp{FdBasedDb::VIDEOS_FILTER};
    QStringList MP4_TYPES{"*.mp4"};

    // precondition
    QDir dir{rootpath, "", QDir::SortFlag::Name, QDir::Filter::Files};
    dir.setNameFilters(MP4_TYPES);
    const QStringList& vids = dir.entryList();
    QCOMPARE(vids.size(), 5);  // only 5 mp4 videos

    // Procedure
    FdBasedDb dbManager{dbName, "FD_MOVIE_DB_CONN"};
    QVERIFY(dbManager.CreateTable("VOLUME_E", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created

    FdBasedDb::VIDEOS_FILTER = MP4_TYPES;
    QCOMPARE(dbManager.ReadADirectory("VOLUME_E", rootpath), vids.size());
    QCOMPARE(dbManager.SetDuration("VOLUME_E"), vids.size());
    QCOMPARE(dbManager.SetDuration("VOLUME_E"), 0);  // 0 duration need update

    QVERIFY(dbManager.DeleteDatabase());
    QVERIFY(!QFile{dbName}.exists());
  }

  void test_ExportDurationStudioCastTagsToJson() {
    GlbDataProtect<decltype(FdBasedDb::VIDEOS_FILTER)> bkp{FdBasedDb::VIDEOS_FILTER};
    GlbDataProtect<bool> checkBkp{FdBasedDb::CHECK_TABLE_VOLUME_ONLINE};
    FdBasedDb::VIDEOS_FILTER = QStringList{"*.mkv"};
    const QString MKV_FILENAME{"Big Buck Bunny SampleVideo_360x240_1mb 9s.mkv"};
    const QString JSON_FILENAME{"Big Buck Bunny SampleVideo_360x240_1mb 9s.json"};

    // precondition
    QDir dir{rootpath, "", QDir::SortFlag::Name, QDir::Filter::Files};
    dir.setNameFilters(FdBasedDb::VIDEOS_FILTER);
    const QStringList& vids = dir.entryList();
    QCOMPARE(vids.size(), 1);  // only 1 mkv videos
    QVERIFY(dir.exists(MKV_FILENAME));
    QVERIFY(!dir.exists(JSON_FILENAME));
    ON_SCOPE_EXIT {
      if (dir.exists(JSON_FILENAME)) {
        QVERIFY(dir.remove(JSON_FILENAME));
      }
    };

    // Procedure
    FdBasedDb dbManager{dbName, "FD_MOVIE_DB_CONN"};
    QVERIFY(dbManager.CreateTable("VOLUME_E", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created
    QCOMPARE(dbManager.ReadADirectory("VOLUME_E", rootpath), vids.size());

    // volume "VOLUME_E" is offline
    FdBasedDb::CHECK_TABLE_VOLUME_ONLINE = true;
    QCOMPARE(dbManager.ExportDurationStudioCastTagsToJson("VOLUME_E"), FD_DISK_OFFLINE);

    // no need export, because duration, studio, cast, tags, are empty
    FdBasedDb::CHECK_TABLE_VOLUME_ONLINE = false;
    QCOMPARE(dbManager.ExportDurationStudioCastTagsToJson("VOLUME_E"), 0);
    QVERIFY(dir.exists(MKV_FILENAME));
    QVERIFY(!dir.exists(JSON_FILENAME));

    // need export 1 json(because of Cast key valid) by 1 mkv
    using namespace MOVIE_TABLE;
    const QString updateCmd{                                                                              //
                            QString{R"(UPDATE %1 SET `%2` = "Henry Cavill, Chris Evans", `%3` = 6000;)"}  // Atension, here we write ", " in purpose
                                .arg("VOLUME_E")                                                          //
                                .arg(ENUM_TO_STRING(Cast))                                         //
                                .arg(ENUM_TO_STRING(Duration))};
    QCOMPARE(dbManager.UpdateForTest(updateCmd), vids.size());
    QCOMPARE(dbManager.ExportDurationStudioCastTagsToJson("VOLUME_E"), vids.size());
    QVERIFY(dir.exists(MKV_FILENAME));
    QVERIFY(dir.exists(JSON_FILENAME));

    using namespace JsonFileHelper;
    const QStringList expectCastLst{"Henry Cavill", " Chris Evans"};  // Atension,  here we use ',' to seperate not ", "
    const QStringList notExpectCastLst{"Henry Cavill", "Chris Evans"};
    const auto& dict = MovieJsonLoader(dir.absoluteFilePath(JSON_FILENAME));
    QCOMPARE(dict.value(ENUM_TO_STRING(Cast)).toStringList(), expectCastLst);
    QVERIFY(dict.value(ENUM_TO_STRING(Cast)).toStringList() != notExpectCastLst);
  }

  void test_UpdateStudioCastTagsByJson() {
    GlbDataProtect<decltype(FdBasedDb::VIDEOS_FILTER)> bkp{FdBasedDb::VIDEOS_FILTER};
    GlbDataProtect<bool> checkBkp{FdBasedDb::CHECK_TABLE_VOLUME_ONLINE};
    FdBasedDb::VIDEOS_FILTER = QStringList{"*.mkv"};
    const QString MKV_FILENAME{"Big Buck Bunny SampleVideo_360x240_1mb 9s.mkv"};
    const QString JSON_FILE_NAME{"Big Buck Bunny SampleVideo_360x240_1mb 9s.json"};
    using namespace JsonFileHelper;

    // precondition
    QDir dir{rootpath, "", QDir::SortFlag::Name, QDir::Filter::Files};
    dir.setNameFilters(FdBasedDb::VIDEOS_FILTER);
    const QString JSON_ABS_PATH{dir.absoluteFilePath(JSON_FILE_NAME)};
    const QStringList& vids = dir.entryList();
    QCOMPARE(vids.size(), 1);  // only 1 mkv videos
    QVERIFY(dir.exists(MKV_FILENAME));
    QVERIFY(!dir.exists(JSON_FILE_NAME));
    ON_SCOPE_EXIT {
      if (dir.exists(JSON_FILE_NAME)) {
        QVERIFY(dir.remove(JSON_FILE_NAME));
      }
    };

    // Procedure
    FdBasedDb dbManager{dbName, "FD_MOVIE_DB_CONN"};
    QVERIFY(dbManager.CreateTable("VOLUME_E", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created
    QCOMPARE(dbManager.ReadADirectory("VOLUME_E", rootpath), vids.size());
    FdBasedDb::CHECK_TABLE_VOLUME_ONLINE = false;

    // json not exist, skip
    QVERIFY(!dir.exists(JSON_FILE_NAME));
    QCOMPARE(dbManager.UpdateStudioCastTagsByJson("VOLUME_E", rootpath), 0);

    // json studio not exits, but performers/tags not exist
    QVariantHash keyValueNotFull{{ENUM_TO_STRING(Studio), ""},                                             //
                                 {ENUM_TO_STRING(Cast), QStringList{"Chris Pine", "Henry Cavill"}},  //
                                 {ENUM_TO_STRING(Tags), QStringList{"Action", "Science"}}};
    QVERIFY(DumpJsonDict(keyValueNotFull, JSON_ABS_PATH));
    QVERIFY(dir.exists(JSON_FILE_NAME));
    QCOMPARE(dbManager.UpdateStudioCastTagsByJson("VOLUME_E", rootpath), 0);

    // json studio/performers/tags all exists
    QVariantHash keyFull{{ENUM_TO_STRING(Studio), "HongMeng"},
                         {ENUM_TO_STRING(Cast), QStringList{"Chris Evans", "Henry Cavill"}},  //
                         {ENUM_TO_STRING(Tags), QStringList{"Action", "Science"}}};
    QVERIFY(DumpJsonDict(keyFull, JSON_ABS_PATH));
    QVERIFY(dir.exists(JSON_FILE_NAME));
    QCOMPARE(dbManager.UpdateStudioCastTagsByJson("VOLUME_E", rootpath), 1);

    const QString selectAllTemp{"SELECT * FROM %1"};
    QList<QSqlRecord> ansList;
    QVERIFY(dbManager.QueryForTest(selectAllTemp.arg("VOLUME_E"), ansList));
    QCOMPARE(ansList.size(), 1);  // 1 mkv file -> 1 record
    const QSqlRecord& firstRecord = ansList.front();
    QCOMPARE(firstRecord.value(MOVIE_TABLE::Studio).toString(), "HongMeng");
    QCOMPARE(firstRecord.value(MOVIE_TABLE::Cast).toString(), "Chris Evans,Henry Cavill");  // sperated by comma only
    QCOMPARE(firstRecord.value(MOVIE_TABLE::Tags).toString(), "Action,Science");            // sperated by comma only
  }
};

FdBasedDbTest g_FdBasedDbTest;
#include "FdBasedDbTest.moc"
