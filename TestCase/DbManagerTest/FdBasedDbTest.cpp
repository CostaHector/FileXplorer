#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "GlbDataProtect.h"
#include "OnScopeExit.h"
#include "FdBasedDb.h"
#include "TableFields.h"
#include "JsonKey.h"
#include "JsonHelper.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "QuickWhereClauseHelper.h"
#include "TDir.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlDriver>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
#include "VideoDurationGetterMock.h"
USING_MOCKCPP_NS

const QString rootpath = TESTCASE_ROOT_PATH "/test/TestEnvVideosDurationGetter";

class FdBasedDbTest : public PlainTestSuite {
  Q_OBJECT
 public:
  FdBasedDbTest() : PlainTestSuite{} {}
  TDir tDir;
  const QString dbName = tDir.itemPath("FD_MOVIE_DB_CONN.db");
  const QString connName = "FD_MOVIE_DB_CONN";
  const QString tableName = "RANDOM_TABLE_NAME";
  const QList<FsNodeEntry> mNodes{
      {"path1/20s.mp4", false, "contents 20s"},
      {"path1/40s.avi", false, "contents 40s"},
      {"path2/60s.mkv", false, "contents 60s"},
      {"path2/30s.mpeg", false, "contents 30s"},
      {"path3/new name 30s.mpeg", false, "contents 30s"},  // adt(path2, path3)-> will update Name field(from 20s.mp4 to new name 20s.mp4)
      {"path4", false, ""},                                // adt(anything, path4)->will remove all
  };
 private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());
    QCOMPARE(tDir.createEntries(mNodes), 6);

    QHash<QString, int> presetVidDurations{
        {tDir.itemPath("path1/20s.mp4"), 20},   //
        {tDir.itemPath("path1/40s.avi"), 40},   //
        {tDir.itemPath("path2/60s.mkv"), 60},   //
        {tDir.itemPath("path2/30s.mpeg"), 30},  //
        {tDir.itemPath("path3/new name 30s.mpeg"), 30},
    };
    VideoDurationGetterMock::PresetVidsDuration(presetVidDurations);
  }
  void init() {  //
    GlobalMockObject::reset();
  }
  void cleanup() {
    GlobalMockObject::verify();
    if (QFile{dbName}.exists()) {
      QFile{dbName}.remove();
    }
  }

  void test_createTable_DropTable_ok() {
    FdBasedDb dbManager{dbName, connName};
    QVERIFY(dbManager.CreateDatabase());
    QVERIFY(dbManager.CreateTable("VOLUME_E", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(dbManager.CreateTable("AGED", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());
    QCOMPARE(dbManager.DropTable("VOLUME_E"), 1);  // by default, full match
    QCOMPARE(dbManager.DropTable("AGED"), 1);
    QCOMPARE(dbManager.DropTable("VOLUME_E"), 0);
    QCOMPARE(dbManager.DropTable("AGED"), 0);
    QVERIFY(QFile{dbName}.exists());
  }

  void test_GetSelectMovieByCastStatement_ok() {
    FdBasedDb dbManager{dbName, connName};
    QVERIFY(dbManager.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));

    QVERIFY(QFile{dbName}.exists());  // should created
    auto db = dbManager.GetDb();
    QVERIFY(db.isOpen());
    QSqlQuery query{db};
    QVERIFY(query.prepare(FdBasedDb::INSERT_MOVIE_RECORD_TEMPLATE.arg(tableName)));

    const QList<QStringList> items  //
        {
            // PrePathLeft, PrePathRight, Name, PathHash
            {"C:/", "home/to/Cpath/", "Kaka - Real Madrid"},     {"D:/", "home/to/Dpath/", "Kaka Leite - Real Madrid"},
            {"E:/", "home/to/Epath/", "Chris Evans - Captain"},  {"F:/", "home/to/Fpath/", "Captain America 2022"},
            {"G:/", "home/to/Gpath/", "Chris Hemsworth - Thor"},
        };

    const QSet<QString> expectMovieNameSet{
        "E:/home/to/Epath/Chris Evans - Captain",
        "F:/home/to/Fpath/Captain America 2022",
        "G:/home/to/Gpath/Chris Hemsworth - Thor",
    };

    qint64 fdVal = 0;
    QByteArray fdByteArray;
    for (const auto& item : items) {
      ++fdVal;
      fdByteArray = QString::number(fdVal).toUtf8();
      using namespace MOVIE_TABLE;
      query.bindValue(":" ENUM_2_STR(Fd), fdByteArray);
      query.bindValue(":" ENUM_2_STR(PrePathLeft), item[0]);
      query.bindValue(":" ENUM_2_STR(PrePathRight), item[1]);
      query.bindValue(":" ENUM_2_STR(Name), item[2]);
      query.bindValue(":" ENUM_2_STR(Size), 1024);
      query.bindValue(":" ENUM_2_STR(PathHash), JsonHelper::CalcFileHash(item[0] + item[1] + item[2]));
      const bool insertOk = query.exec();
      QVERIFY(insertOk);
    }

    using namespace QuickWhereClauseHelper;
    const QString selectStatement = GetSelectMovieByCastStatement("Chris Evans", "Captain|Chris", tableName);
    QVERIFY(selectStatement.contains("Chris Evans"));
    QVERIFY(selectStatement.contains("Captain"));
    QVERIFY(selectStatement.contains("Chris"));
    QVERIFY(selectStatement.contains(tableName));
    QVERIFY(query.exec(selectStatement));
    QSet<QString> actualMovieNameSet;
    if (db.driver()->hasFeature(QSqlDriver::QuerySize)) {
      QCOMPARE(query.size(), 3);
    }
    while (query.next()) {
      actualMovieNameSet.insert(GetMovieFullPathFromSqlQry(query));
    }
    QCOMPARE(actualMovieNameSet, expectMovieNameSet);
  }

  void test_ReadADirectory_invalid() {
    // precondition
    QVERIFY(!QFile{dbName}.exists());  // should created
    FdBasedDb dbManager{dbName, connName};

    // Procedure
    QVERIFY(dbManager.CreateTable("VOLUME_E", FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should table created ok

    // 1. path not exist
    const QString inexistPath{rootpath + "_not_exist_Path"};
    QVERIFY(!QFileInfo{inexistPath}.isDir());
    QCOMPARE(dbManager.ReadADirectory("VOLUME_E", inexistPath), FD_NOT_DIR);

    // 2. invalid table name
    QVERIFY(!QFileInfo{inexistPath}.isDir());
    QCOMPARE(dbManager.ReadADirectory("", rootpath), FD_TABLE_NAME_INVALID);

    // 3. inexist table
    const QString& inexistTableName{"InexistTable"};
    QVERIFY(!QFileInfo{inexistPath}.isDir());
    QCOMPARE(dbManager.ReadADirectory(inexistTableName, rootpath), FD_TABLE_INEXIST);
  }

  void test_ReadADirectory_incremental() {
    // Procedure
    FdBasedDb dbManager{dbName, connName};
    QVERIFY(dbManager.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created

    QCOMPARE(dbManager.ReadADirectory(tableName, tDir.itemPath("path1")), 2);
    QCOMPARE(dbManager.ReadADirectory(tableName, tDir.itemPath("path2")), 2);
    QCOMPARE(dbManager.ReadADirectory(tableName, tDir.itemPath("path1")), 0);
    QCOMPARE(dbManager.ReadADirectory(tableName, tDir.itemPath("path2")), 0);  // fd already in table

    using namespace MOVIE_TABLE;
    const QString qryWhereClause{QString(R"(`%1` like "%.mp4")").arg(ENUM_2_STR(Name))};
    QCOMPARE(dbManager.CountRow(tableName, qryWhereClause), 1);
    QCOMPARE(dbManager.DeleteByWhereClause(tableName, qryWhereClause), 1);
    QCOMPARE(dbManager.CountRow(tableName), 3);
  }

  void test_adt() {
    using namespace MOVIE_TABLE;
    // precondition
    FdBasedDb dbManager{dbName, connName};
    QVERIFY(dbManager.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created

    // 1. Adt(empty table no path, path1)
    VolumeUpdateResult adtRet{0};
    adtRet.Init();
    QCOMPARE(dbManager.Adt(tableName, tDir.itemPath("path1"), &adtRet), FD_OK);
    QCOMPARE(adtRet.insertCnt, 2);
    QCOMPARE(adtRet.deleteCnt, 0);
    QCOMPARE(adtRet.updateCnt, 0);
    // Also Check file name should only contain beforeMKVFileName
    QSet<QString> oldActualNames;
    QVERIFY(dbManager.QueryPK(tableName, ENUM_2_STR(Name), oldActualNames));
    QCOMPARE(oldActualNames, (QSet<QString>{"20s.mp4", "40s.avi"}));

    // 2. Adt(table contains path1, path2)
    adtRet.Init();
    QCOMPARE(dbManager.Adt(tableName, tDir.itemPath("path2"), &adtRet), FD_OK);
    QCOMPARE(adtRet.insertCnt, 2);
    QCOMPARE(adtRet.deleteCnt, 2);
    QCOMPARE(adtRet.updateCnt, 0);
    QVERIFY(dbManager.QueryPK(tableName, ENUM_2_STR(Name), oldActualNames));
    QCOMPARE(oldActualNames, (QSet<QString>{"60s.mkv", "30s.mpeg"}));

    // 3. Adt(path2, path3)
    adtRet.Init();
    QCOMPARE(dbManager.Adt(tableName, tDir.itemPath("path3"), &adtRet), FD_OK);
    QCOMPARE(adtRet.insertCnt, 0);
    QCOMPARE(adtRet.deleteCnt, 1);
    QCOMPARE(adtRet.updateCnt, 1);
    QVERIFY(dbManager.QueryPK(tableName, ENUM_2_STR(Name), oldActualNames));
    QCOMPARE(oldActualNames, (QSet<QString>{"new name 30s.mpeg"}));

    // 4. Adt(path2, path4)
    adtRet.Init();
    QCOMPARE(dbManager.Adt(tableName, tDir.itemPath("path4"), &adtRet), FD_OK);
    QCOMPARE(adtRet.insertCnt, 0);
    QCOMPARE(adtRet.deleteCnt, 1);
    QCOMPARE(adtRet.updateCnt, 0);
    QVERIFY(dbManager.QueryPK(tableName, ENUM_2_STR(Name), oldActualNames));
    QCOMPARE(oldActualNames, (QSet<QString>()));
  }

  void test_SetDuration() {
    // Procedure
    MOCKER(VideoDurationGetter::GetLengthQuickStatic)  //
        .stubs()                                       //
        .will(invoke(VideoDurationGetterMock::invokeGetLengthQuickStatic));

    FdBasedDb dbManager{dbName, connName};
    QVERIFY(dbManager.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created

    QCOMPARE(dbManager.ReadADirectory(tableName, tDir.path()), 4);  // remeber: Fd conflict because file contents same!
    QCOMPARE(dbManager.SetDuration(tableName), 4);
    QCOMPARE(dbManager.SetDuration(tableName), 0);  // 0 duration need update

    QSet<int> actualDurations;
    using namespace MOVIE_TABLE;
    QVERIFY(dbManager.QueryPK(tableName, ENUM_2_STR(Duration), actualDurations));
    QCOMPARE(actualDurations, (QSet<int>{20, 40, 30, 60}));
  }

  void test_ExportDurationStudioCastTagsToJson() {
    MOCKER(FdBasedDb::IsTableVolumeOnline)
        .stubs()  //
        .will(returnValue(false))
        .then(returnValue(true));

    FdBasedDb dbManager{dbName, connName};
    QVERIFY(dbManager.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created
    QCOMPARE(dbManager.ReadADirectory(tableName, tDir.itemPath("path1")), 2);

    // volume "tableName" is offline, no need export
    QCOMPARE(dbManager.ExportDurationStudioCastTagsToJson(tableName), FD_DISK_OFFLINE);

    // volume "tableName" is online. but duration, studio, cast, tags, are empty, no need export
    QCOMPARE(dbManager.ExportDurationStudioCastTagsToJson(tableName), 0);

    // 2 row update (Cast, Duration)
    using namespace MOVIE_TABLE;
    MOCKER(VideoDurationGetter::GetLengthQuickStatic)  //
        .stubs()                                       //
        .will(invoke(VideoDurationGetterMock::invokeGetLengthQuickStatic));
    QCOMPARE(dbManager.SetDuration(tableName), 2);
    QCOMPARE(dbManager.ExportDurationStudioCastTagsToJson(tableName), 2);
    QVERIFY(tDir.exists("path1/20s.json"));
    QVERIFY(tDir.exists("path1/40s.json"));

    // Attention: there is an extra space after comma between two cast.
    // we expect it will be kept.
    const QString updateCmd{QString{R"(UPDATE `%1` SET `%2` = "Henry Cavill, Chris Evans";)"}.arg(tableName).arg(ENUM_2_STR(Cast))};
    QCOMPARE(dbManager.UpdateForTest(updateCmd), 2);
    QCOMPARE(dbManager.ExportDurationStudioCastTagsToJson(tableName), 2);

    using namespace JsonHelper;
    const QStringList expectCastLst{"Henry Cavill", " Chris Evans"};  // Atension,  here we use ',' to seperate not ", "
    const QStringList notExpectCastLst{"Henry Cavill", "Chris Evans"};
    const auto& dict = MovieJsonLoader(tDir.itemPath("path1/20s.json"));
    QCOMPARE(dict.value(ENUM_2_STR(Cast)).toStringList(), expectCastLst);
    QVERIFY(dict.value(ENUM_2_STR(Cast)).toStringList() != notExpectCastLst);
  }

  void test_UpdateStudioCastTagsByJson() {
    using namespace JsonHelper;
    const QString path1 = tDir.itemPath("path1");
    MOCKER(FdBasedDb::IsTableVolumeOnline)
        .stubs()  //
        .will(returnValue(true));

    FdBasedDb dbManager{dbName, connName};
    QVERIFY(dbManager.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created
    QCOMPARE(dbManager.ReadADirectory(tableName, path1), 2);

    // 表中studio/performers/tags的字段值将被json字段覆盖
    const QString updateStudioCmd{QString{R"(UPDATE `%1` SET `%2` = "Hong Meng";)"}.arg(tableName).arg(ENUM_2_STR(Studio))};
    QCOMPARE(dbManager.UpdateForTest(updateStudioCmd), 2);
    const QString updateTagsCmd{QString{R"(UPDATE `%1` SET `%2` = "Adventure,Documentary";)"}.arg(tableName).arg(ENUM_2_STR(Tags))};
    QCOMPARE(dbManager.UpdateForTest(updateTagsCmd), 2);

    // no json exists not exist, skip
    QFile::remove(tDir.itemPath("path1/20s.json"));
    QFile::remove(tDir.itemPath("path1/40s.json"));
    QVERIFY(!tDir.exists("path1/20s.json"));
    QVERIFY(!tDir.exists("path1/40s.json"));
    QCOMPARE(dbManager.UpdateStudioCastTagsByJson(tableName, path1), 0);

    // 前提: 只要json中的studio/performers/tags有一个字段值非空
    // 预期: json字段原封不动覆盖写入表中字段(若json字段为空, 则表字段会设置为空)
    using namespace MOVIE_TABLE;
    QVariantHash keyValueNotFull{{ENUM_2_STR(Studio), "Century"},    //
                                 {ENUM_2_STR(Cast), QStringList()},  //
                                 {ENUM_2_STR(Tags), QStringList()}};
    QVariantHash keyFull{{ENUM_2_STR(Studio), "Fox"},
                         {ENUM_2_STR(Cast), QStringList{"Chris Evans", "Henry Cavill"}},  //
                         {ENUM_2_STR(Tags), QStringList{"Action", "Science"}}};
    QVERIFY(DumpJsonDict(keyValueNotFull, tDir.itemPath("path1/20s.json")));
    QVERIFY(DumpJsonDict(keyFull, tDir.itemPath("path1/40s.json")));
    QVERIFY(tDir.exists("path1/20s.json"));
    QVERIFY(tDir.exists("path1/40s.json"));
    QCOMPARE(dbManager.UpdateStudioCastTagsByJson(tableName, path1), 2);

    const QString selectCentury{R"(SELECT * FROM `%1` WHERE `%2`="Century";)"};
    const QString selectFox{R"(SELECT * FROM `%1` WHERE `%2`="Fox";)"};

    QList<QSqlRecord> centuryList;
    QVERIFY(dbManager.QueryForTest(selectCentury.arg(tableName).arg(ENUM_2_STR(Studio)), centuryList));
    QCOMPARE(centuryList.size(), 1);
    const QSqlRecord& centuryRec = centuryList.front();
    QCOMPARE(centuryRec.value(MOVIE_TABLE::Studio).toString(), "Century");
    QCOMPARE(centuryRec.value(MOVIE_TABLE::Cast).toString(), "");
    QCOMPARE(centuryRec.value(MOVIE_TABLE::Tags).toString(), "");

    QList<QSqlRecord> foxList;
    QVERIFY(dbManager.QueryForTest(selectFox.arg(tableName).arg(ENUM_2_STR(Studio)), foxList));
    QCOMPARE(foxList.size(), 1);
    const QSqlRecord& foxRec = foxList.front();
    QCOMPARE(foxRec.value(MOVIE_TABLE::Studio).toString(), "Fox");
    QCOMPARE(foxRec.value(MOVIE_TABLE::Cast).toString(), "Chris Evans,Henry Cavill");  // sperated by comma only
    QCOMPARE(foxRec.value(MOVIE_TABLE::Tags).toString(), "Action,Science");            // sperated by comma only
  }
};

#include "FdBasedDbTest.moc"
REGISTER_TEST(FdBasedDbTest, false)
