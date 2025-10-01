#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "CastDbModel.h"
#include "CastBaseDb.h"
#include "FdBasedDb.h"

#include "EndToExposePrivateMember.h"
#include "TableFields.h"
#include "CastPsonFileHelper.h"
#include "PublicVariable.h"
#include "TDir.h"
#include "SqlTableTestPreconditionTool.h"

using namespace SqlTableTestPreconditionTool;

class CastDbModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void default_initialized_ok() {
    Configuration().setValue(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, "Path/to/inexists/path");
    CastDbModel castModel;
    {  // precondition
      QVERIFY(!QFileInfo("Path/to/inexists/path").isDir());
      QVERIFY(!castModel.database().isValid());
      QVERIFY(!castModel.database().isOpen());
      QVERIFY(!CastDbModel::isDbValidAndOpened(castModel.database()));
    }

    {  // basic setting and property
      QCOMPARE(castModel.editStrategy(), QSqlTableModel::EditStrategy::OnManualSubmit);
      QCOMPARE(castModel.m_imageHostPath, "Path/to/inexists/path");
      QCOMPARE(castModel.rootPath(), "Path/to/inexists/path");
      QCOMPARE(castModel.rowCount(), 0);
      QCOMPARE(castModel.columnCount(), 0);
      QVERIFY(castModel.GetAllRowsIndexes().isEmpty());
      QVERIFY(!castModel.isDirty());
    }

    {  // header access
      QCOMPARE(castModel.headerData(1, Qt::Horizontal).toInt(), 2);
      QCOMPARE(castModel.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 0 + 1);
      QCOMPARE(castModel.headerData(1, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 1 + 1);
      QCOMPARE(castModel.headerData(1000, Qt::Vertical, Qt::TextAlignmentRole).toInt(), (int)Qt::AlignRight);
    }

    {  // data retrieve
      QModelIndex invalidIndex;
      QVERIFY(!invalidIndex.isValid());
      QVERIFY(castModel.data(invalidIndex, Qt::ItemDataRole::DisplayRole).isNull());
      QVERIFY(!castModel.setData(invalidIndex, "new value", Qt::ItemDataRole::EditRole));

      QCOMPARE(castModel.rootPath(), "Path/to/inexists/path");
      QCOMPARE(castModel.fileName(invalidIndex), "");
      QCOMPARE(castModel.filePath(invalidIndex), "");
      QCOMPARE(castModel.fileInfo(invalidIndex).absoluteFilePath(), "");
      QCOMPARE(castModel.oriPath(invalidIndex), "");
      QCOMPARE(castModel.psonFilePath(invalidIndex), "");
    }

    {  // batch modification
      QModelIndexList emptyIndexes;
      QItemSelection emptySelectionList;
      QCOMPARE(castModel.SyncImageFieldsFromImageHost(emptyIndexes), 0);
      QCOMPARE(castModel.DumpRecordsIntoPsonFile(emptyIndexes), 0);
      QCOMPARE(castModel.DeleteSelectionRange(emptySelectionList), 0);
      QSqlDatabase emptyMovieDb;
      QCOMPARE(castModel.RefreshVidsForRecords(emptyIndexes, emptyMovieDb), 0);
      QCOMPARE(castModel.MigrateCastsTo(emptyIndexes, QFileInfo(__FILE__).absolutePath()), 0);
    }

    {  // property
      QVERIFY(!castModel.isDirty());
      QVERIFY(castModel.submitSaveAllChanges());
    }
    {  // call me in the view, not in the model itself
      QVERIFY(!castModel.repopulate());
      QVERIFY(!castModel.onRevert());  // no need
    }
  }

  void data_retrieve_ok() {
    TDir tDir;
    QVERIFY(tDir.IsValid());

    const QString dbName{tDir.itemPath("CaseDbModelTest.db")};
    const QString connName{"CaseDbModelTestConn"};
    const QString tableName{DB_TABLE::PERFORMERS};

    CastBaseDb castDb{dbName, connName};
    QVERIFY(castDb.IsValid());
    {  // database exist but table not exist
      CastDbModel castModelNoTable{nullptr, castDb.GetDb()};
      QVERIFY(castModelNoTable.database().isValid());
      QVERIFY(castModelNoTable.database().isOpen());
      QVERIFY(castModelNoTable.tableName().isEmpty());
      QCOMPARE(castModelNoTable.rowCount(), 0);
    }
    // now table exist
    QVERIFY(castDb.CreateTable(tableName, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(castDb.IsTableExist(tableName));
    QVERIFY(castDb.IsTableEmpty(tableName));

    Configuration().setValue(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, tDir.path());
    CastDbModel castModel(nullptr, castDb.GetDb());
    QVERIFY(castModel.database().isValid());
    QVERIFY(castModel.database().isOpen());
    QCOMPARE(castModel.tableName(), tableName);
    QCOMPARE(castModel.m_imageHostPath, tDir.path());

    // prepare folder precondition
    QVERIFY(CreateFileStructure(tDir));

    QCOMPARE(castDb.ReadFromImageHost(tDir.path()), 5);  // 5 cast in total

    QCOMPARE(castModel.rowCount(), 0);
    castModel.select();
    QVERIFY(castModel.repopulate());
    QCOMPARE(castModel.rowCount(), 5);
    QCOMPARE(castModel.GetAllRowsIndexes().size(), 5);

    using namespace PERFORMER_DB_HEADER_KEY;
    {  // data retrieve and setData ok
      QModelIndexList indexesNames{GetIndexessAtOneRow(castModel, 0, 5, PERFORMER_DB_HEADER_KEY::Name)};
      QVERIFY(
          CheckIndexesDisplayRoleIgnoreOrder(castModel, indexesNames,  //
                                             QStringList{"Chris Evans", "Chris Hemsworth", "Chris Pine", "Cristiano Ronaldo", "Michael Fassbender"}));

      QModelIndexList indexesOris{GetIndexessAtOneRow(castModel, 0, 5, PERFORMER_DB_HEADER_KEY::Ori)};
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(castModel, indexesOris,  //
                                                 QStringList{"SuperHero", "SuperHero", "SuperHero", "X-MEN", "Football"}));

      QModelIndexList indexesRates{GetIndexessAtOneRow(castModel, 0, 5, PERFORMER_DB_HEADER_KEY::Rate)};
      QVERIFY(CastPsonFileHelper::DEFAULT_RATE != 99);
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(castModel, indexesRates,                      //
                                                 QList<int>{CastPsonFileHelper::DEFAULT_RATE,  //
                                                            CastPsonFileHelper::DEFAULT_RATE,  //
                                                            CastPsonFileHelper::DEFAULT_RATE,  //
                                                            CastPsonFileHelper::DEFAULT_RATE,  //
                                                            CastPsonFileHelper::DEFAULT_RATE}));
      // Rate DecorationRole correct
      const QVariant pixmapVar = castModel.data(castModel.index(0, PERFORMER_DB_HEADER_KEY::Rate), Qt::DecorationRole);
      QVERIFY(pixmapVar.isValid());
      QVERIFY(pixmapVar.canConvert<QPixmap>());
      const QPixmap pixmapVarRate = pixmapVar.value<QPixmap>();
      QVERIFY(!pixmapVarRate.isNull());

      // Rate setData correct
      QVERIFY(castModel.flags(castModel.index(0, PERFORMER_DB_HEADER_KEY::Rate)).testFlag(Qt::ItemIsEditable));
      QCOMPARE(castModel.fieldIndex(ENUM_2_STR(Rate)), PERFORMER_DB_HEADER_KEY::Rate);
      QVERIFY(castModel.setData(castModel.index(0, PERFORMER_DB_HEADER_KEY::Rate), 99, Qt::EditRole));  // rate: 99
      QVERIFY(castModel.setData(castModel.index(0, PERFORMER_DB_HEADER_KEY::Rate), 99, Qt::EditRole));  // already 99, set 99 will return true
      QVERIFY(castModel.setData(castModel.index(4, PERFORMER_DB_HEADER_KEY::Rate), 5, Qt::EditRole));   // rate: 5
      QVERIFY(castModel.isDirty());
      QVERIFY(castModel.submitSaveAllChanges());
      QVERIFY(!castModel.isDirty());
      QVERIFY(castModel.setData(castModel.index(4, PERFORMER_DB_HEADER_KEY::Rate), 79, Qt::EditRole));  // rate: 79
      QVERIFY(castModel.isDirty());
      QVERIFY(castModel.onRevert());  // onRevert ok
      QVERIFY(!castModel.isDirty());
      QModelIndexList indexesRatesEditRole{GetIndexessAtOneRow(castModel, 0, 5, PERFORMER_DB_HEADER_KEY::Rate)};
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(castModel, indexesRatesEditRole,  //
                                                 QList<int>{0, 0, 0, 5, 99}, Qt::EditRole));
    }

    {  // rename ok => leads to cast_name/cast_name.pson,cast_name.img also get renamed
      castModel.sort(PERFORMER_DB_HEADER_KEY::Name, Qt::AscendingOrder);
      const QModelIndex firstEleNameIndex = castModel.index(0, PERFORMER_DB_HEADER_KEY::Name);

      QVERIFY(tDir.exists("SuperHero/Chris Evans"));
      QCOMPARE(tDir.SnapshotAtPath(tDir.itemPath("SuperHero/Chris Evans")), (QSet<QString>{"Chris Evans 1.jpg", "Chris Evans 2.jpg"}));
      QCOMPARE(castModel.data(firstEleNameIndex, Qt::DisplayRole).toString(), "Chris Evans");
      QVERIFY(castModel.setData(firstEleNameIndex, "Kaka", Qt::EditRole));  // rename cast name and its related files
      QCOMPARE(castModel.data(firstEleNameIndex, Qt::DisplayRole).toString(), "Kaka");
      QVERIFY(!tDir.exists("SuperHero/Chris Evans"));
      QVERIFY(tDir.exists("SuperHero/Kaka"));
      QCOMPARE(tDir.SnapshotAtPath(tDir.itemPath("SuperHero/Kaka")), (QSet<QString>{"Kaka 1.jpg", "Kaka 2.jpg"}));
      QVERIFY(!castModel.setData(firstEleNameIndex, "Kaka", Qt::EditRole));        // already kaka
      QVERIFY(castModel.setData(firstEleNameIndex, "Chris Evans", Qt::EditRole));  // rename back to "Chris Evans"

      QCOMPARE(castModel.fileName(firstEleNameIndex), "Chris Evans");
      QCOMPARE(castModel.filePath(firstEleNameIndex), tDir.itemPath("SuperHero/Chris Evans"));
      QCOMPARE(castModel.fileInfo(firstEleNameIndex).absoluteFilePath(), tDir.itemPath("SuperHero/Chris Evans"));
      QCOMPARE(castModel.oriPath(firstEleNameIndex), tDir.itemPath("SuperHero"));
      QCOMPARE(castModel.psonFilePath(firstEleNameIndex), tDir.itemPath("SuperHero/Chris Evans/Chris Evans.pson"));

      QVERIFY(!castModel.setData(firstEleNameIndex, "Chris Pine", Qt::EditRole));        // Chris Pine is already occupied
      QVERIFY(!castModel.setData(firstEleNameIndex, "/Chris/../ Evans", Qt::EditRole));  // invalid new Name
      QCOMPARE(castModel.fileName(firstEleNameIndex), "Chris Evans");                    // not change
    }

    {  // SyncImageFieldsFromImageHost. sync 2 records that need sync only
      castModel.sort(PERFORMER_DB_HEADER_KEY::Name, Qt::AscendingOrder);
      const QModelIndex firstEleImgsIndex = castModel.index(0, PERFORMER_DB_HEADER_KEY::Imgs);
      const QModelIndex secondEleImgsIndex = castModel.index(1, PERFORMER_DB_HEADER_KEY::Imgs);
      const QModelIndex thirdEleImgsIndex = castModel.index(2, PERFORMER_DB_HEADER_KEY::Imgs);

      QCOMPARE(castModel.data(firstEleImgsIndex, Qt::DisplayRole).toString(), "Chris Evans 1.jpg\nChris Evans 2.jpg");
      QVERIFY(castModel.setData(firstEleImgsIndex, "Chris Evans 2.jpg", Qt::EditRole));
      QCOMPARE(castModel.data(firstEleImgsIndex, Qt::DisplayRole).toString(), "Chris Evans 2.jpg");

      QCOMPARE(castModel.data(secondEleImgsIndex, Qt::DisplayRole).toString(), "Chris Hemsworth.jpg");

      QCOMPARE(castModel.data(thirdEleImgsIndex, Qt::DisplayRole).toString(), "Chris Pine.jpg");
      QVERIFY(castModel.setData(thirdEleImgsIndex, "Chris  Pine invalid.jpg", Qt::EditRole));
      QCOMPARE(castModel.data(thirdEleImgsIndex, Qt::DisplayRole).toString(), "Chris  Pine invalid.jpg");

      QCOMPARE(castModel.SyncImageFieldsFromImageHost({firstEleImgsIndex, secondEleImgsIndex, thirdEleImgsIndex}), 3 - 1);  // only 2/3 record changed

      QCOMPARE(castModel.data(firstEleImgsIndex, Qt::DisplayRole).toString(), "Chris Evans 1.jpg\nChris Evans 2.jpg");
      QCOMPARE(castModel.data(secondEleImgsIndex, Qt::DisplayRole).toString(), "Chris Hemsworth.jpg");
      QCOMPARE(castModel.data(thirdEleImgsIndex, Qt::DisplayRole).toString(), "Chris Pine.jpg");

      QCOMPARE(castModel.SyncImageFieldsFromImageHost({firstEleImgsIndex, secondEleImgsIndex, thirdEleImgsIndex}), 0);  // only 0/3 record need sync
    }

    {  // DumpRecordsIntoPsonFile ok
      castModel.sort(PERFORMER_DB_HEADER_KEY::Name, Qt::AscendingOrder);
      const QModelIndex firstEleImgsIndex = castModel.index(0, PERFORMER_DB_HEADER_KEY::Imgs);
      const QModelIndex secondEleImgsIndex = castModel.index(1, PERFORMER_DB_HEADER_KEY::Imgs);
      QVERIFY(!tDir.exists("SuperHero/Chris Evans/Chris Evans.pson"));
      QVERIFY(!tDir.exists("SuperHero/Chris Hemsworth/Chris Hemsworth.pson"));
      QCOMPARE(castModel.DumpRecordsIntoPsonFile({firstEleImgsIndex, secondEleImgsIndex}), 2);
      QVERIFY(tDir.exists("SuperHero/Chris Evans/Chris Evans.pson"));
      QVERIFY(tDir.exists("SuperHero/Chris Hemsworth/Chris Hemsworth.pson"));
      QCOMPARE(castModel.DumpRecordsIntoPsonFile({firstEleImgsIndex, secondEleImgsIndex}), 0);  // unchange
    }

    {  // DeleteSelectionRange
      QCOMPARE(castModel.rowCount(), 5);
      const QItemSelection allSelection(castModel.index(0, PERFORMER_DB_HEADER_KEY::Name),  //
                                        castModel.index(4, PERFORMER_DB_HEADER_KEY::Name));
      QCOMPARE(castModel.DeleteSelectionRange(allSelection), 5);
      QCOMPARE(castModel.rowCount(), 0);
      QCOMPARE(castModel.DeleteSelectionRange(allSelection), 0);  // already delete
      QCOMPARE(castModel.rowCount(), 0);
      QCOMPARE(castDb.ReadFromImageHost(tDir.path()), 5);  // recover to 5 cast
      QVERIFY(castModel.select());                         // select first
      QCOMPARE(castModel.rowCount(), 5);
    }

    {  // RefreshVidsForRecords
      castModel.sort(PERFORMER_DB_HEADER_KEY::Name, Qt::AscendingOrder);
      QModelIndexList indexesVids{GetIndexessAtOneRow(castModel, 0, 5, PERFORMER_DB_HEADER_KEY::Vids)};
      QCOMPARE(castModel.RefreshVidsForRecords(indexesVids, QSqlDatabase()), FD_ERROR_CODE::FD_DB_OPEN_FAILED);  // default constructor not update

      const QString movieDbName{tDir.itemPath("MovieCaseDbModelTest.db")};
      const QString movieConnName{"MovieCaseDbModelTestConn"};
      const QString movieTableName{DB_TABLE::MOVIES};
      FdBasedDb movieDb{movieDbName, movieConnName};
      QVERIFY(movieDb.IsValid());
      QVERIFY(movieDb.CreateTable(movieTableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
      QVERIFY(movieDb.IsTableExist(movieTableName));
      QVERIFY(movieDb.IsTableEmpty(movieTableName));

      QCOMPARE(castModel.RefreshVidsForRecords(indexesVids, movieDb.GetDb()), 0);  // no records not update

      {
        QSet<qint64> needInsertFds = {1001, 1002, 1003};  // 三个唯一的文件描述符
        QHash<qint64, QString> newFd2Pth = {
            // 3 records, 2 of Chris Evans and 1 of Chris Hemsworth
            {1001, "videos/superhero/Chris Evans 1.mp4"},
            {1002, "videos/superhero/Chris Evans Captain America.mp4"},
            {1003, "videos/superhero/Chris Hemsworth.mp4"},
        };
        // 调用Insert函数插入记录
        int insertCnt = 0;
        FD_ERROR_CODE errorCode = movieDb.Insert(movieTableName, needInsertFds, newFd2Pth, insertCnt);
        QVERIFY(errorCode == FD_OK);
        QCOMPARE(insertCnt, 3);
      }
      QCOMPARE(movieDb.CountRow(movieTableName, ""), 3);

      QCOMPARE(castModel.RefreshVidsForRecords(indexesVids, movieDb.GetDb()), 2);  // affected rows count = 2. and sorted
      QCOMPARE(castModel.RefreshVidsForRecords(indexesVids, movieDb.GetDb()), 0);  // unchange at all skip

      const QModelIndex firstEleVidsIndex = castModel.index(0, PERFORMER_DB_HEADER_KEY::Vids);
      const QModelIndex secondEleVidsIndex = castModel.index(1, PERFORMER_DB_HEADER_KEY::Vids);
      const QModelIndex thirdEleVidsIndex = castModel.index(2, PERFORMER_DB_HEADER_KEY::Vids);
      QCOMPARE(castModel.data(firstEleVidsIndex, Qt::DisplayRole).toString(),
               "videos/superhero/Chris Evans 1.mp4\n"
               "videos/superhero/Chris Evans Captain America.mp4");
      QCOMPARE(castModel.data(secondEleVidsIndex, Qt::DisplayRole).toString(), "videos/superhero/Chris Hemsworth.mp4");
      QCOMPARE(castModel.data(thirdEleVidsIndex, Qt::DisplayRole).toString(), "");

      // ReadFromImageHost will not override vids field
      QCOMPARE(castDb.ReadFromImageHost(tDir.path()), 5);
      QCOMPARE(castModel.data(firstEleVidsIndex, Qt::DisplayRole).toString(),
               "videos/superhero/Chris Evans 1.mp4\n"
               "videos/superhero/Chris Evans Captain America.mp4");
      QCOMPARE(castModel.data(secondEleVidsIndex, Qt::DisplayRole).toString(), "videos/superhero/Chris Hemsworth.mp4");
      QCOMPARE(castModel.data(thirdEleVidsIndex, Qt::DisplayRole).toString(), "");
    }

    {  // MigrateCastsTo ok
      castModel.sort(PERFORMER_DB_HEADER_KEY::Name, Qt::AscendingOrder);
      const QModelIndex firstOriIndex = castModel.index(0, PERFORMER_DB_HEADER_KEY::Ori);
      const QModelIndex secondOriIndex = castModel.index(1, PERFORMER_DB_HEADER_KEY::Ori);
      QCOMPARE(castModel.data(firstOriIndex, Qt::DisplayRole).toString(), "SuperHero");
      QCOMPARE(castModel.data(secondOriIndex, Qt::DisplayRole).toString(), "SuperHero");
      QCOMPARE(castModel.MigrateCastsTo({firstOriIndex, secondOriIndex}, "path/not/exist/path"), FD_CAST_NEW_ORI_PATH_INVALID);

      QVERIFY(tDir.mkpath("CaptainAmerica"));  // new ori
      QString absPath = tDir.itemPath("CaptainAmerica");
      QCOMPARE(castModel.MigrateCastsTo({firstOriIndex, secondOriIndex}, absPath), 2);  // two ori get updated

      QCOMPARE(castModel.data(firstOriIndex, Qt::DisplayRole).toString(), "CaptainAmerica");
      QCOMPARE(castModel.data(secondOriIndex, Qt::DisplayRole).toString(), "CaptainAmerica");

      QCOMPARE(castModel.MigrateCastsTo({firstOriIndex, secondOriIndex}, absPath), 0);  // already there
      QCOMPARE(castModel.MigrateCastsTo({firstOriIndex, secondOriIndex}, absPath), 0);  // already there
    }

    {  // after db closed
      QVERIFY(castModel.setData(castModel.index(0, PERFORMER_DB_HEADER_KEY::Name), "ChrisEvans.jpg", Qt::EditRole));
      QVERIFY(castModel.isDirty());
      castModel.database().close();
      QVERIFY(!CastDbModel::isDbValidAndOpened(castModel.database()));
      QVERIFY(!castModel.submitSaveAllChanges());
    }
  }
};

#include "CastDbModelTest.moc"
REGISTER_TEST(CastDbModelTest, false)
