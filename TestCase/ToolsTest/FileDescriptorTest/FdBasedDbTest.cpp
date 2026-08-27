#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "FdBasedDb.h"
#include "MovieDBModelField.h"
#include "JsonHelper.h"
#include "PublicMacro.h"
#include "QuickWhereClauseHelper.h"
#include "JsonFieldBoundary.h"
#include "TDir.h"
#include "FileTool.h"

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

#include "VideoTestPrecoditionTools.h"

class FdBasedDbTest : public PlainTestSuite {
  Q_OBJECT
 public:
  FdBasedDbTest() : PlainTestSuite{} {}
  const QString rootpath{VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH};
  TDir mTDir;
  const QString dbName = mTDir.itemPath("FD_MOVIE_DB_CONN.db");
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
    QVERIFY(mTDir.IsValid());
    QCOMPARE(mTDir.createEntries(mNodes), 6);

    QHash<QString, int> presetVidDurations{
        {mTDir.itemPath("path1/20s.mp4"), 20},   //
        {mTDir.itemPath("path1/40s.avi"), 40},   //
        {mTDir.itemPath("path2/60s.mkv"), 60},   //
        {mTDir.itemPath("path2/30s.mpeg"), 30},  //
        {mTDir.itemPath("path3/new name 30s.mpeg"), 30},
    };
    VideoDurationGetterMock::PresetVidsDuration(presetVidDurations);
  }

  void init() {  //
    GlobalMockObject::reset();
    if (mTDir.exists("ScanJsonsFolder")) {
      QVERIFY(QDir{mTDir.itemPath("ScanJsonsFolder")}.removeRecursively());
    }
  }

  void cleanup() {
    GlobalMockObject::verify();
    if (QFile{dbName}.exists()) {
      QFile{dbName}.remove();
    }
  }

  void createTable_DropTable_ok() {
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

  void GetSelectMovieByCastStatement_ok() {
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
      using namespace MovieDBModelField;
      query.bindValue(":" ENUM_2_STR(SampleMD5), fdByteArray);
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

  void ReadADirectory_invalid_ok() {
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

  void ReadADirectory_ok() {
    // Procedure
    FdBasedDb dbManager{dbName, connName};
    QVERIFY(dbManager.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created

    QCOMPARE(dbManager.ReadADirectory(tableName, mTDir.itemPath("path1")), 2);
    QCOMPARE(dbManager.CountRow(tableName), 2);
    QCOMPARE(dbManager.ReadADirectory(tableName, mTDir.itemPath("path2")), 2);
    QCOMPARE(dbManager.CountRow(tableName), 2 + 2);
    QCOMPARE(dbManager.ReadADirectory(tableName, mTDir.itemPath("path1")), 2);
    QCOMPARE(dbManager.ReadADirectory(tableName, mTDir.itemPath("path2")), 2);
    QCOMPARE(dbManager.CountRow(tableName), 4);

    using namespace MovieDBModelField;
    const QString qryWhereClause{QString(R"(`%1` like "%.mp4")").arg(ENUM_2_STR(Name))};
    QCOMPARE(dbManager.CountRow(tableName, qryWhereClause), 1);
    QCOMPARE(dbManager.DeleteByWhereClause(tableName, qryWhereClause), 1);
    QCOMPARE(dbManager.CountRow(tableName), 3);
  }

  void adt_ok() {
    using namespace MovieDBModelField;
    // precondition
    FdBasedDb dbManager{dbName, connName};
    QVERIFY(dbManager.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created

    // 1. Adt(empty table no path, path1)
    VolumeUpdateResult adtRet{0};
    adtRet.Init();
    QCOMPARE(dbManager.Adt(tableName, mTDir.itemPath("path1"), &adtRet), FD_OK);
    QCOMPARE(adtRet.insertCnt, 2);
    QCOMPARE(adtRet.deleteCnt, 0);
    QCOMPARE(adtRet.updateCnt, 0);
    // Also Check file name should only contain beforeMKVFileName
    QSet<QString> oldActualNames;
    QVERIFY(dbManager.QueryPK(tableName, ENUM_2_STR(Name), oldActualNames));
    QCOMPARE(oldActualNames, (QSet<QString>{"20s.mp4", "40s.avi"}));

    // 2. Adt(table contains path1, path2)
    adtRet.Init();
    QCOMPARE(dbManager.Adt(tableName, mTDir.itemPath("path2"), &adtRet), FD_OK);
    QCOMPARE(adtRet.insertCnt, 2);
    QCOMPARE(adtRet.deleteCnt, 2);
    QCOMPARE(adtRet.updateCnt, 0);
    QVERIFY(dbManager.QueryPK(tableName, ENUM_2_STR(Name), oldActualNames));
    QCOMPARE(oldActualNames, (QSet<QString>{"60s.mkv", "30s.mpeg"}));

    // 3. Adt(path2, path3)
    adtRet.Init();
    QCOMPARE(dbManager.Adt(tableName, mTDir.itemPath("path3"), &adtRet), FD_OK);
    QCOMPARE(adtRet.insertCnt, 0);
    QCOMPARE(adtRet.deleteCnt, 1);
    QCOMPARE(adtRet.updateCnt, 1);
    QVERIFY(dbManager.QueryPK(tableName, ENUM_2_STR(Name), oldActualNames));
    QCOMPARE(oldActualNames, (QSet<QString>{"new name 30s.mpeg"}));

    // 4. Adt(path2, path4)
    adtRet.Init();
    QCOMPARE(dbManager.Adt(tableName, mTDir.itemPath("path4"), &adtRet), FD_OK);
    QCOMPARE(adtRet.insertCnt, 0);
    QCOMPARE(adtRet.deleteCnt, 1);
    QCOMPARE(adtRet.updateCnt, 0);
    QVERIFY(dbManager.QueryPK(tableName, ENUM_2_STR(Name), oldActualNames));
    QCOMPARE(oldActualNames, (QSet<QString>()));
  }

  void SetDuration_ok() {
    // Procedure
    MOCKER(VideoDurationGetter::GetLengthQuickStatic)  //
        .stubs()                                       //
        .will(invoke(VideoDurationGetterMock::invokeGetLengthQuickStatic));

    FdBasedDb dbManager{dbName, connName};
    QVERIFY(dbManager.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created

    QCOMPARE(dbManager.ReadADirectory(tableName, mTDir.path()), 4);  // remeber: SampleMD5 conflict because file contents same!
    QCOMPARE(dbManager.SetDuration(tableName), 4);
    QCOMPARE(dbManager.SetDuration(tableName), 0);  // 0 duration need update

    QSet<int> actualDurations;
    using namespace MovieDBModelField;
    QVERIFY(dbManager.QueryPK(tableName, ENUM_2_STR(Duration), actualDurations));
    QCOMPARE(actualDurations, (QSet<int>{20, 40, 30, 60}));
  }

  void ReadADirectoryJson_ok() {
    const QList<FsNodeEntry> scanJsonsFolderNodes {
        // ignore videos
        {"ScanJsonsFolder/CristianoRonaldo.mp4", false, "Content: Kaka.mp4"},
        // Kaka, 6s, 1KiB, 01234567890123456789012345678MD5, inLocal=1, rate=10, need further process
        {"ScanJsonsFolder/Kaka.mp4", false, "Content: Kaka.mp4"},
        {"ScanJsonsFolder/Kaka.json", false, R"({
    "Cast": ["Person 1", "2 Person"],
    "Detail": "Release date: 1st June 2026",
    "Name": "Kaka",
    "Studio": "PrideKaka",
    "Tags": ["tags 0", "tags 1"],
    "MD5": "01234567890123456789012345678MD5",
    "VidName": "Kaka.mp4",
    "Size": 1024,
    "Duration": 6000,
    "Rate": 10
})"}, // RandomOne, 60s, 2KiB, MD501234567890123456789012345678, inLocal=0, rate=use default JsonFieldBoundary::RATE_MIN_UNINITIALIZED_V, need further process
        {"ScanJsonsFolder/RandomOne.json", false, R"({
    "Cast": ["Person 3", "4 Person"],
    "Detail": "Release date: 2nd June 2026",
    "Name": "RandomOne",
    "Studio": "PrideRandomOne",
    "Tags": ["tags 2", "tags 3"],
    "MD5": "MD501234567890123456789012345678",
    "VidName": "RandomOne.mp4",
    "Size": 2048,
    "Duration": 60000
})"}, // json with MD5 field empty but VidName not empty. cannot read
        {"ScanJsonsFolder/MD5EmptyVidNameNotEmpty.json", false, R"({
    "Cast": ["Person 5", "6 Person"],
    "Detail": "Release date: 2nd June 2026",
    "Name": "MD5EmptyVidNameNotEmpty",
    "Studio": "Pride",
    "Tags": ["tags 2", "tags 3"],
    "MD5": "",
    "VidName": "MD5EmptyVidNameNotEmpty.mp4",
    "Size": 2048,
    "Duration": 60000
})"}, // json with MD5 field empty And VidName empty. ignore
        {"ScanJsonsFolder/MD5EmptyAndVidNameEmpty.json", false, R"({
    "Cast": ["Person 5", "6 Person"],
    "Detail": "Release date: 2nd June 2026",
    "Name": "MD5EmptyAndVidNameEmpty",
    "Studio": "Pride",
    "Tags": ["tags 2", "tags 3"],
    "MD5": "",
    "VidName": "",
    "Size": 2048,
    "Duration": 60000
})"},

    };
    QCOMPARE(mTDir.createEntries(scanJsonsFolderNodes), scanJsonsFolderNodes.size());

    const QString dbName{mTDir.itemPath("FdBasedMovieReadJsons.db")};
    const QString connName{"FdBasedMovieReadJsonsConn"};
    const QString scanJsonTableName{"ABCDEF12_3456_7890_ABCDEF1234567890"};  // can be converted to guid

    FdBasedDb movieDb{dbName, connName};
    QVERIFY(movieDb.IsValid());
    QVERIFY(movieDb.CreateTable(scanJsonTableName, FdBasedDb::CREATE_TABLE_TEMPLATE));

    QCOMPARE(movieDb.ReadADirectoryJson(scanJsonTableName, mTDir.itemPath("ScanJsonsFolder")), FD_ERROR_CODE::FD_JSON_PARSED_INVALID);
    // delete the dismatch one
    QVERIFY(QDir{mTDir}.remove("ScanJsonsFolder/MD5EmptyVidNameNotEmpty.json"));
    QCOMPARE(movieDb.ReadADirectoryJson(scanJsonTableName, mTDir.itemPath("ScanJsonsFolder")), 2); // 2 json need further process

    FdBasedDb fdDb{dbName, connName};
    QCOMPARE(fdDb.CountRow(scanJsonTableName), 2); // only 2 json will be inserted into table

    const QString sqlSelectCmd{QString{"SELECT * FROM `%1`"}.arg(scanJsonTableName)};
    QList<QSqlRecord> records;
    QVERIFY(fdDb.QueryForTest(sqlSelectCmd, records));
    QCOMPARE(records.size(), 2);

    // "Kaka.mp4", "RandomOne.mp4"
    using namespace MovieDBModelField;
    std::sort(records.begin(), records.end(), [](const QSqlRecord& lhs, const QSqlRecord& rhs)->bool{
      return lhs.value(ENUM_2_STR(Name)).toString() < rhs.value(ENUM_2_STR(Name)).toString();
    });
    const QSqlRecord& kaka = records[0], &randomOne = records[1];
    QCOMPARE(kaka.value(ENUM_2_STR(Name)).toString(), "Kaka.mp4");
    QCOMPARE(kaka.value(ENUM_2_STR(SampleMD5)).toString(), "01234567890123456789012345678MD5");
    QCOMPARE(kaka.value(ENUM_2_STR(Size)).toLongLong(), 1024);
    QCOMPARE(kaka.value(ENUM_2_STR(Duration)).toInt(), 6000);
    QCOMPARE(kaka.value(ENUM_2_STR(Rate)).toInt(), 10);
    QCOMPARE(kaka.value(ENUM_2_STR(Studio)).toString(), "PrideKaka");
    QCOMPARE(kaka.value(ENUM_2_STR(Cast)).toString(), "Person 1,2 Person");
    QCOMPARE(kaka.value(ENUM_2_STR(Tags)).toString(), "tags 0,tags 1");
    QCOMPARE(kaka.value(ENUM_2_STR(Detail)).toString(), "Release date: 1st June 2026");
    QCOMPARE(kaka.value(ENUM_2_STR(InLocal)).toInt(), 1);

    QCOMPARE(randomOne.value(ENUM_2_STR(Name)).toString(), "RandomOne.mp4");
    QCOMPARE(randomOne.value(ENUM_2_STR(SampleMD5)).toString(), "MD501234567890123456789012345678");
    QCOMPARE(randomOne.value(ENUM_2_STR(Size)).toLongLong(), 2048);
    QCOMPARE(randomOne.value(ENUM_2_STR(Duration)).toInt(), 60000);
    QCOMPARE(randomOne.value(ENUM_2_STR(Rate)).toInt(), JsonFieldBoundary::RATE_MIN_UNINITIALIZED_V);
    QCOMPARE(randomOne.value(ENUM_2_STR(Studio)).toString(), "PrideRandomOne");
    QCOMPARE(randomOne.value(ENUM_2_STR(Cast)).toString(), "Person 3,4 Person");
    QCOMPARE(randomOne.value(ENUM_2_STR(Tags)).toString(), "tags 2,tags 3");
    QCOMPARE(randomOne.value(ENUM_2_STR(Detail)).toString(), "Release date: 2nd June 2026");
    QCOMPARE(randomOne.value(ENUM_2_STR(InLocal)).toInt(), 0);
  }

  void ExportToEfuFile_ok() {
    const QList<FsNodeEntry> scanJsonsFolderNodes {
      // No Kaka.mp4 found. InLocal=0
        {"ScanJsonsFolder/Kaka.json", false, R"({
    "Cast": ["Person 1"],
    "Detail": "Release date: 1st June 2026",
    "Name": "Kaka",
    "Studio": "PrideKaka",
    "Tags": ["tags 0"],
    "MD5": "01234567890123456789012345678MD5",
    "VidName": "Kaka.mp4",
    "Size": 1024,
    "Duration": 6000,
    "Rate": 10
})"}
    };
    QCOMPARE(mTDir.createEntries(scanJsonsFolderNodes), 1);

    QString exportEfuTableName{"EXPORT12_3456_7890_ABCDEF1234567890"};
    FdBasedDb dbManager{dbName, connName};
    QVERIFY(dbManager.CreateTable(exportEfuTableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created
    QCOMPARE(dbManager.ReadADirectoryJson(exportEfuTableName, mTDir.itemPath("ScanJsonsFolder")), 1);

    const QString exportToEfuPath{mTDir.itemPath("ScanJsonsFolder/EXPORT12_3456_7890_ABCDEF1234567890.efu")};
    QCOMPARE(dbManager.ExportToEfuFile("inexist table name", exportToEfuPath), FD_ERROR_CODE::FD_TABLE_INEXIST);
    QCOMPARE(dbManager.ExportToEfuFile(exportEfuTableName, "inexist/path/to/EfuFile.efu"), FD_ERROR_CODE::FD_EFU_FILE_INVALID);
    QCOMPARE(dbManager.ExportToEfuFile(exportEfuTableName, exportToEfuPath), 1);

    bool bReadOk{false};
    const QByteArray& actualContents = FileTool::ByteArrayReader(exportToEfuPath, &bReadOk);
    QVERIFY(bReadOk);
    QCOMPARE(actualContents.count('\n'), 2); // "title line\nrecord 1\n"
    const QList<QByteArray>& lines = actualContents.split('\n');
    QVERIFY(lines.back().isEmpty());
    QCOMPARE(lines.size(), 3);
    const QByteArray& titleLine = lines[0];
    const QByteArray& recordLine = lines[1];
    QCOMPARE(titleLine.count(','), recordLine.count(',')); // columns count must equal
    QVERIFY(recordLine.contains(mTDir.itemPath("ScanJsonsFolder/Kaka.mp4").toUtf8()));
    QVERIFY(recordLine.contains(QByteArray::number(1024))); // size
    QVERIFY(recordLine.contains("00:00:06.000")); // duration: 6000ms
    QVERIFY(recordLine.contains("Kaka.mp4")); // Name
    QVERIFY(recordLine.contains("PrideKaka")); // Studio
    QVERIFY(recordLine.contains("Person 1")); // Cast
    QVERIFY(recordLine.contains("tags 0")); // Tags
    QVERIFY(recordLine.contains(QByteArray::number(10))); // Rate
    QVERIFY(recordLine.contains("Release date: 1st June 2026")); // Detail
    QVERIFY(recordLine.contains(QByteArray::number(0))); // InLocal
    QVERIFY(recordLine.contains("01234567890123456789012345678MD5")); // SampleMD5
  }

  void ExportDurationStudioCastTagsToJson_ok() {
    MOCKER(FdBasedDb::IsTableVolumeOnline)
        .stubs()  //
        .will(returnValue(false))
        .then(returnValue(true));

    FdBasedDb dbManager{dbName, connName};
    QVERIFY(dbManager.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());  // should created
    QCOMPARE(dbManager.ReadADirectory(tableName, mTDir.itemPath("path1")), 2);

    // volume "tableName" is offline, no need export
    QCOMPARE(dbManager.ExportDurationStudioCastTagsToJson(tableName), FD_DISK_OFFLINE);

    // volume "tableName" is online. but duration, studio, cast, tags, are empty, no need export
    QCOMPARE(dbManager.ExportDurationStudioCastTagsToJson(tableName), 0);

    // 2 row update (Cast, Duration)
    using namespace MovieDBModelField;
    MOCKER(VideoDurationGetter::GetLengthQuickStatic)  //
        .stubs()                                       //
        .will(invoke(VideoDurationGetterMock::invokeGetLengthQuickStatic));
    QCOMPARE(dbManager.SetDuration(tableName), 2);
    QCOMPARE(dbManager.ExportDurationStudioCastTagsToJson(tableName), 2);
    QVERIFY(mTDir.exists("path1/20s.json"));
    QVERIFY(mTDir.exists("path1/40s.json"));

    // Attention: there is an extra space after comma between two cast.
    // we expect it will be kept.
    const QString updateCmd{QString{R"(UPDATE `%1` SET `%2` = "Henry Cavill, Chris Evans";)"}.arg(tableName).arg(ENUM_2_STR(Cast))};
    QCOMPARE(dbManager.UpdateForTest(updateCmd), 2);
    QCOMPARE(dbManager.ExportDurationStudioCastTagsToJson(tableName), 2);

    using namespace JsonHelper;
    const QStringList expectCastLst{"Henry Cavill", " Chris Evans"};  // Atension,  here we use ',' to seperate not ", "
    const QStringList notExpectCastLst{"Henry Cavill", "Chris Evans"};
    const auto& dict = MovieJsonLoader(mTDir.itemPath("path1/20s.json"));
    QCOMPARE(dict.value(ENUM_2_STR(Cast)).toStringList(), expectCastLst);
    QVERIFY(dict.value(ENUM_2_STR(Cast)).toStringList() != notExpectCastLst);
  }

  void UpdateStudioCastTagsByJson_ok() {
    using namespace JsonHelper;
    const QString path1 = mTDir.itemPath("path1");
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
    QFile::remove(mTDir.itemPath("path1/20s.json"));
    QFile::remove(mTDir.itemPath("path1/40s.json"));
    QVERIFY(!mTDir.exists("path1/20s.json"));
    QVERIFY(!mTDir.exists("path1/40s.json"));
    QCOMPARE(dbManager.UpdateStudioCastTagsByJson(tableName, path1), 0);

    // 前提: 只要json中的studio/performers/tags有一个字段值非空
    // 预期: json字段原封不动覆盖写入表中字段(若json字段为空, 则表字段会设置为空)
    using namespace MovieDBModelField;
    QVariantHash keyValueNotFull{{ENUM_2_STR(Studio), "Century"},    //
                                 {ENUM_2_STR(Cast), QStringList()},  //
                                 {ENUM_2_STR(Tags), QStringList()}};
    QVariantHash keyFull{{ENUM_2_STR(Studio), "Fox"},
                         {ENUM_2_STR(Cast), QStringList{"Chris Evans", "Henry Cavill"}},  //
                         {ENUM_2_STR(Tags), QStringList{"Action", "Science"}}};
    QVERIFY(DumpJsonDict(keyValueNotFull, mTDir.itemPath("path1/20s.json")));
    QVERIFY(DumpJsonDict(keyFull, mTDir.itemPath("path1/40s.json")));
    QVERIFY(mTDir.exists("path1/20s.json"));
    QVERIFY(mTDir.exists("path1/40s.json"));
    QCOMPARE(dbManager.UpdateStudioCastTagsByJson(tableName, path1), 2);

    const QString selectCentury{R"(SELECT * FROM `%1` WHERE `%2`="Century";)"};
    const QString selectFox{R"(SELECT * FROM `%1` WHERE `%2`="Fox";)"};

    QList<QSqlRecord> centuryList;
    QVERIFY(dbManager.QueryForTest(selectCentury.arg(tableName).arg(ENUM_2_STR(Studio)), centuryList));
    QCOMPARE(centuryList.size(), 1);
    const QSqlRecord& centuryRec = centuryList.front();
    QCOMPARE(centuryRec.value(MovieDBModelField::Studio).toString(), "Century");
    QCOMPARE(centuryRec.value(MovieDBModelField::Cast).toString(), "");
    QCOMPARE(centuryRec.value(MovieDBModelField::Tags).toString(), "");

    QList<QSqlRecord> foxList;
    QVERIFY(dbManager.QueryForTest(selectFox.arg(tableName).arg(ENUM_2_STR(Studio)), foxList));
    QCOMPARE(foxList.size(), 1);
    const QSqlRecord& foxRec = foxList.front();
    QCOMPARE(foxRec.value(MovieDBModelField::Studio).toString(), "Fox");
    QCOMPARE(foxRec.value(MovieDBModelField::Cast).toString(), "Chris Evans,Henry Cavill");  // sperated by comma only
    QCOMPARE(foxRec.value(MovieDBModelField::Tags).toString(), "Action,Science");            // sperated by comma only
  }
};

#include "FdBasedDbTest.moc"
REGISTER_TEST(FdBasedDbTest, false)
