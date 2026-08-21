#include <QtTest/QtTest>
#include "PlainTestSuite.h"


#include "BeginToExposePrivateMember.h"
#include "FdBasedDb.h"
#include "FdBasedDbModel.h"
#include "EndToExposePrivateMember.h"
#include "MovieDBModelField.h"
#include "TDir.h"
#include "SqlTableTestPreconditionTool.h"
#include "JsonFieldBoundary.h"

#include "VideoDurationGetter.h"
#include "VideoDurationGetterMock.h"
#include <QSqlRecord>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
using namespace SqlTableTestPreconditionTool;

class FdBasedDbModelTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
private slots:

  void initTestCase() {  //
    QVERIFY(mTDir.IsValid());
  }

  void init() {  //
    mTDir.ClearAll();
  }

  void default_constructor_ok() {
    FdBasedDbModel fdModel;
    // call member should not leading crash down

    QModelIndex invalidIndex;
    QVERIFY(fdModel.data(invalidIndex, Qt::ItemDataRole::DisplayRole).isNull());
    QVERIFY(fdModel.absolutePath(invalidIndex).isEmpty());
    QVERIFY(fdModel.fileName(invalidIndex).isEmpty());
    QVERIFY(fdModel.filePath(invalidIndex).isEmpty());
    QVERIFY(fdModel.fileInfo(invalidIndex).filePath().isEmpty());
    QVERIFY(fdModel.fullInfo(invalidIndex).isEmpty());

    QModelIndexList emptyList;
    fdModel.SetStudio(emptyList, "Test Studio");
    fdModel.SetCastOrTags(emptyList, "Test Cast");
    fdModel.AddCastOrTags(emptyList, "Test Cast");
    fdModel.RmvCastOrTags(emptyList, "Test Cast");

    // 验证默认值
    QCOMPARE(fdModel.rootPath(), QString());
    QCOMPARE(fdModel.GUID(), QString());

    // 额外测试：非法section调用headerData
    QCOMPARE(fdModel.headerData(1, Qt::Horizontal).toInt(), 2);
    QCOMPARE(fdModel.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 0 + 1);
    QCOMPARE(fdModel.headerData(1, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 1 + 1);
    QCOMPARE(fdModel.headerData(1000, Qt::Vertical, Qt::TextAlignmentRole).toInt(), (int)Qt::AlignRight);

    // 测试无效索引调用siblingAtColumn
    QModelIndex sibling = invalidIndex.siblingAtColumn(0);
    QVERIFY(!sibling.isValid());

    QCOMPARE(fdModel.GetSelectionFileSizes({}), (QList<qint64>{}));
    QCOMPARE(fdModel.GetSelectionDurations({}), (QList<int>{}));
  }

  void data_retrieve_ok() {
    const QString dbName{mTDir.itemPath("FdBasedMovieTest.db")};
    const QString connName{"FdBasedMovieTestConn"};
    const QString tableName{"ABCDEF12_3456_7890_ABCDEF1234567890"};  // can be converted to guid

    const QList<FsNodeEntry> nodes{
                                   {"Chris Evans.mp4", false, "Chris Evans"},                // 11
                                   {"Chris Hemsworth.mp4", false, "Chris Hemsworth"},        // 5+1+9
                                   {"Chris Pine.mp4", false, "Chris Pine"},                  // 10
                                   {"Michael Fassbender.mp4", false, "Michael Fassbender"},  // 7+1+4+6
                                   {"Cristiano Ronaldo.jpg", false, "Cristiano Ronaldo"},
                                   };
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());
    QSet<QString> movieNames{"Chris Evans.mp4", "Chris Hemsworth.mp4", "Chris Pine.mp4", "Michael Fassbender.mp4"};
    QSet<QString> movieSizes{"0'0'0'11", "0'0'0'15", "0'0'0'10", "0'0'0'18"};
    QSet<QString> absolutePathSet{mTDir.path(), mTDir.path(), mTDir.path(), mTDir.path()};

    FdBasedDb movieDb{dbName, connName};
    QVERIFY(movieDb.IsValid());
    QVERIFY(movieDb.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(movieDb.IsTableExist(tableName));
    QVERIFY(movieDb.IsTableEmpty(tableName));

    FdBasedDbModel movieModel{nullptr, movieDb.GetDb()};
    movieModel.setTable(tableName);
    movieModel.select();
    QCOMPARE(movieModel.GUID(), "ABCDEF12-3456-7890-ABCDEF1234567890");
    QCOMPARE(movieModel.rowCount(), 0);
    {
      // .jpg is not videos. ignored
      QCOMPARE(movieDb.ReadADirectory(tableName, mTDir.path()), 4);
      movieModel.select();
      QCOMPARE(movieModel.rowCount(), 4);

      QSet<QString> actualMovieNames;
      QSet<QString> actualMovieSizes;
      QSet<QString> actualAbsolutePathSet;
      QSet<QString> actualFileNameSet;
      for (int i = 0; i < movieModel.rowCount(); ++i) {
        actualMovieNames.insert(movieModel.data(movieModel.index(i, MovieDBModelField::Name), Qt::DisplayRole).toString());
        actualMovieSizes.insert(movieModel.data(movieModel.index(i, MovieDBModelField::Size), Qt::DisplayRole).toString());
        actualAbsolutePathSet.insert(movieModel.absolutePath(movieModel.index(i, MovieDBModelField::Name)));
        actualFileNameSet.insert(movieModel.fileName(movieModel.index(i, MovieDBModelField::Name)));
      }
      QCOMPARE(actualMovieNames, movieNames);
      QCOMPARE(actualMovieSizes, movieSizes);
      QCOMPARE(actualAbsolutePathSet, absolutePathSet);
      QCOMPARE(actualFileNameSet, movieNames);
    }

    QList<FsNodeEntry> nodesExtra {
        {"Raphael Varane.mp4", false, "Raphael Varane"},  // 14
        {"Alvaro Morata.mp4", false, "Alvaro Morata"},    // 13
    };
    QCOMPARE(mTDir.createEntries(nodesExtra), 2);
    QCOMPARE(movieDb.ReadADirectory(tableName, mTDir.path()), 6);
    movieModel.select();

    // 2.1 Studio in table modified
    {
      QModelIndex index4Studio{movieModel.index(4, MovieDBModelField::Studio)};
      QModelIndex index5Studio{movieModel.index(5, MovieDBModelField::Studio)};
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Studio, index5Studio},  //
                                                 QStringList{"", ""}));                     // todo: use GetIndexessAtOneRow
      movieModel.SetStudio({index4Studio, index5Studio}, "Marvel");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Studio, index5Studio},  //
                                                 QStringList{"Marvel", "Marvel"}));
      movieModel.SetStudio({index4Studio}, "Century");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Studio, index5Studio},  //
                                                 QStringList{"Century", "Marvel"}));
      movieModel.SetStudio({index5Studio}, "");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Studio, index5Studio},  //
                                                 QStringList{"Century", ""}));
    }

    // 2.2 Cast modified
    {
      QModelIndex index4Cast{movieModel.index(4, MovieDBModelField::Cast)};
      QModelIndex index5Cast{movieModel.index(5, MovieDBModelField::Cast)};
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 QStringList{"", ""}));
      movieModel.SetCastOrTags({index4Cast, index5Cast}, "Cristiano Ronaldo&Kaka");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 QStringList{"Cristiano Ronaldo,Kaka", "Cristiano Ronaldo,Kaka"}));
      movieModel.SetCastOrTags({index4Cast}, "Kaka");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 QStringList{"Cristiano Ronaldo,Kaka", "Kaka"}));
      movieModel.SetCastOrTags({index5Cast}, "");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 QStringList{"", "Kaka"}));

      movieModel.AddCastOrTags({index4Cast, index5Cast}, "");  // add empty nothing happend
      movieModel.AddCastOrTags({index4Cast, index5Cast}, "Levi,Michael Fassbender");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 QStringList{"Levi,Michael Fassbender", "Kaka,Levi,Michael Fassbender"}));

      movieModel.AddCastOrTags({index4Cast, index5Cast}, "Levi,Michael");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  // again no change
                                                 QStringList{"Levi,Michael,Michael Fassbender", "Kaka,Levi,Michael,Michael Fassbender"}));

      movieModel.RmvCastOrTags({index4Cast, index5Cast}, "");                           // remove empty nothing changed
      movieModel.RmvCastOrTags({index4Cast, index5Cast}, "Ronaldo");                    // not exists at all
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 QStringList{"Levi,Michael,Michael Fassbender", "Kaka,Levi,Michael,Michael Fassbender"}));

      movieModel.RmvCastOrTags({index4Cast, index5Cast}, "Michael");                    // remove full match. not exists at all
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 QStringList{"Levi,Michael Fassbender", "Kaka,Levi,Michael Fassbender"}));
    }
    // 2.3 Tags modified
    {
      QModelIndex index4Tags{movieModel.index(4, MovieDBModelField::Tags)};
      QModelIndex index5Tags{movieModel.index(5, MovieDBModelField::Tags)};

      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 QStringList{"", ""}));
      movieModel.SetCastOrTags({index4Tags, index5Tags}, "Cristiano Ronaldo&Kaka");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 QStringList{"Cristiano Ronaldo,Kaka", "Cristiano Ronaldo,Kaka"}));
      movieModel.SetCastOrTags({index4Tags}, "Kaka");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 QStringList{"Cristiano Ronaldo,Kaka", "Kaka"}));
      movieModel.SetCastOrTags({index5Tags}, "");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 QStringList{"", "Kaka"}));

      movieModel.AddCastOrTags({index4Tags, index5Tags}, "");  // add empty nothing happend
      movieModel.AddCastOrTags({index4Tags, index5Tags}, "Levi,Michael Fassbender");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 QStringList{"Levi,Michael Fassbender", "Kaka,Levi,Michael Fassbender"}));

      movieModel.AddCastOrTags({index4Tags, index5Tags}, "Levi,Michael");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  // again no change
                                                 QStringList{"Levi,Michael,Michael Fassbender", "Kaka,Levi,Michael,Michael Fassbender"}));

      movieModel.RmvCastOrTags({index4Tags, index5Tags}, "");                           // remove empty nothing changed
      movieModel.RmvCastOrTags({index4Tags, index5Tags}, "Ronaldo");                    // not exists at all
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 QStringList{"Levi,Michael,Michael Fassbender", "Kaka,Levi,Michael,Michael Fassbender"}));

      movieModel.RmvCastOrTags({index4Tags, index5Tags}, "Michael");                    // remove full match. not exists at all
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 QStringList{"Levi,Michael Fassbender", "Kaka,Levi,Michael Fassbender"}));
    }
  }

  void get_durations_file_size_ok() {
    const QString dbName{mTDir.itemPath("DurationFileSizeTest.db")};
    const QString connName{"DurationFileSizeTestConn"};
    const QString tableName{"DURATION_3456_7890_ABCDEF1234567890"};  // can be converted to guid
    const QList<FsNodeEntry> nodes{
        {"Chris Evans.mp4", false, "Chris Evans"},          // 5+1+5=11Bytes
        {"Chris Hemsworth.mp4", false, "Chris Hemsworth"},  // 5+1+9=15Bytes
    };
    QCOMPARE(mTDir.createEntries(nodes), 2);

    FdBasedDb movieDb{dbName, connName};
    QVERIFY(movieDb.IsValid());
    QVERIFY(movieDb.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(movieDb.IsTableExist(tableName));
    QVERIFY(movieDb.IsTableEmpty(tableName));

    FdBasedDbModel movieModel{nullptr, movieDb.GetDb()};
    movieModel.setTable(tableName);
    movieModel.select();
    QCOMPARE(movieModel.GUID(), "DURATION-3456-7890-ABCDEF1234567890");
    QCOMPARE(movieModel.rowCount(), 0);

    MOCKER(VideoDurationGetter::GetLengthQuickStatic)  //
        .stubs()                                       //
        .will(invoke(VideoDurationGetterMock::invokeGetLengthQuickStatic));
    QHash<QString, int> vidPath2Duration{
        {mTDir.itemPath("Chris Evans.mp4"), 10 * 1000},      // 10s
        {mTDir.itemPath("Chris Hemsworth.mp4"), 70 * 1000},  // 70s
    };
    VideoDurationGetterMock::PresetVidsDuration(vidPath2Duration);

    QCOMPARE(movieDb.ReadADirectory(tableName, mTDir.path()), 2);
    QCOMPARE(movieDb.SetDuration(tableName), 2);

    movieModel.sort(MovieDBModelField::Name, Qt::AscendingOrder);
    movieModel.select();
    QCOMPARE(movieModel.rowCount(), 2);

    QCOMPARE(movieModel.QSqlTableModel::data(movieModel.index(0, MovieDBModelField::Name), Qt::DisplayRole).toString(), "Chris Evans.mp4");
    QCOMPARE(movieModel.QSqlTableModel::data(movieModel.index(1, MovieDBModelField::Name), Qt::DisplayRole).toString(), "Chris Hemsworth.mp4");
    QCOMPARE(movieModel.QSqlTableModel::data(movieModel.index(0, MovieDBModelField::Size), Qt::DisplayRole).toLongLong(), 11);
    QCOMPARE(movieModel.QSqlTableModel::data(movieModel.index(1, MovieDBModelField::Size), Qt::DisplayRole).toLongLong(), 15);
    QCOMPARE(movieModel.QSqlTableModel::data(movieModel.index(0, MovieDBModelField::Duration), Qt::DisplayRole).toInt(), 10000);
    QCOMPARE(movieModel.QSqlTableModel::data(movieModel.index(1, MovieDBModelField::Duration), Qt::DisplayRole).toInt(), 70000);

    const QModelIndexList indexes{movieModel.index(0, MovieDBModelField::Name), movieModel.index(1, MovieDBModelField::Name)};

    QCOMPARE(movieModel.GetSelectionFileSizes(indexes), (QList<qint64>{11, 15}));
    QCOMPARE(movieModel.GetSelectionDurations(indexes), (QList<int>{10000, 70000}));
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
    const QString tableName{"ABCDEF12_3456_7890_ABCDEF1234567890"};  // can be converted to guid

    FdBasedDb movieDb{dbName, connName};
    QVERIFY(movieDb.IsValid());
    QVERIFY(movieDb.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));

    QCOMPARE(movieDb.ReadADirectoryJson(tableName, mTDir.itemPath("ScanJsonsFolder")), FD_ERROR_CODE::FD_JSON_PARSED_INVALID);
    // delete the dismatch one
    QVERIFY(QDir{mTDir}.remove("ScanJsonsFolder/MD5EmptyVidNameNotEmpty.json"));
    QCOMPARE(movieDb.ReadADirectoryJson(tableName, mTDir.itemPath("ScanJsonsFolder")), 2); // 2 json need further process

    FdBasedDb fdDb{dbName, connName};
    QCOMPARE(fdDb.CountRow(tableName), 2); // only 2 json will be inserted into table

    const QString sqlSelectCmd{QString{"SELECT * FROM `%1`"}.arg(tableName)};
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
};

#include "FdBasedDbModelTest.moc"
REGISTER_TEST(FdBasedDbModelTest, false)
