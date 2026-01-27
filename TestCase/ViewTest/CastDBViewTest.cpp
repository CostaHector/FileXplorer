#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "FdBasedDb.h"
#include "CastDBView.h"
#include "EndToExposePrivateMember.h"
#include "PublicVariable.h"
#include "TableFields.h"
#include "TDir.h"

#include <QSignalSpy>
#include <QSqlRecord>
#include "CastPsonFileHelper.h"
#include "SqlRecordTestHelper.h"
#include "SqlTableTestPreconditionTool.h"

using namespace SqlTableTestPreconditionTool;
using namespace SqlRecordTestHelper;

Q_DECLARE_METATYPE(QSqlRecord)

class CastDBViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() { qRegisterMetaType<QSqlRecord>("QSqlRecord"); }

  void default_construct() {
    TDir tDir;
    QVERIFY(tDir.IsValid());
    Configuration().setValue(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, tDir.path());

    const QString dbName{tDir.itemPath("CaseViewTest.db")};
    const QString connName{"CaseViewTestConn"};
    const QString tableName{DB_TABLE::PERFORMERS};

    QWidget parent;

    CastBaseDb castDb{dbName, connName};
    QVERIFY(!castDb.IsTableExist(tableName));

    CastDbModel castModel{nullptr, castDb.GetDb()};
    CastDatabaseSearchToolBar searchTb{"searchTb", &parent};
    CastDBView castView(&castModel, &searchTb, castDb, &parent);
    {  // initial ok
      QVERIFY(castView._castDbSearchBar != nullptr);
      QVERIFY(castView._castModel != nullptr);
      QVERIFY(castView._castDb.IsValid());

      QVERIFY(!castView.currentIndex().isValid());  // nothing selected

      QCOMPARE(castView.onRefreshVidsFieldCore({}), 0);
      castView.EmitCurrentCastRecordChanged(QModelIndex{}, QModelIndex{});
      castView.RefreshCurrentRowHtmlContents();

      QCOMPARE(castModel.rowCount(), 0);
      QCOMPARE(castModel.columnCount(), 0);
    }

    {  //
      castView.onInitATable();
      QCOMPARE(castModel.rowCount(), 0);
      QVERIFY(castModel.columnCount() > 0);
      QVERIFY(!castModel.isDirty());
      QVERIFY(castView.onModelSubmitAll());
    }

    {
      CastDbViewMocker::MockMultiLineInput() = std::pair<bool, QString>{false, "Guardiola, Pep\nHuge Jackman, Wolverine"};
      QCOMPARE(castView.onAppendCasts(), 0);
      CastDbViewMocker::MockMultiLineInput() = std::pair<bool, QString>{true, "Guardiola, Pep\nHuge Jackman, Wolverine"};
      QCOMPARE(castView.onAppendCasts(), 2);
      QCOMPARE(castModel.rowCount(), 2);
      QVERIFY(!castModel.isDirty());
      QVERIFY(!castView.currentIndex().isValid());

      QModelIndexList indexesNames{GetIndexessAtOneRow(castModel, 0, 2, PERFORMER_DB_HEADER_KEY::Name)};
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(castModel, indexesNames,  //
                                                 QStringList{"Guardiola", "Huge Jackman"}));
      QModelIndexList indexesAkas{GetIndexessAtOneRow(castModel, 0, 2, PERFORMER_DB_HEADER_KEY::ALIAS)};
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(castModel, indexesAkas,  //
                                                 QStringList{"Pep", "Wolverine"}));

      QVERIFY(castModel.setData(castModel.index(0, PERFORMER_DB_HEADER_KEY::Rate), 99, Qt::ItemDataRole::EditRole));
      QCOMPARE(castModel.data(castModel.index(0, PERFORMER_DB_HEADER_KEY::Rate), Qt::ItemDataRole::DisplayRole).toInt(), 99);
      QVERIFY(castModel.isDirty());
      QVERIFY(castModel.submitSaveAllChanges());
      QCOMPARE(castView.onAppendCasts(), 2);  // will not check if changes
      // should not modify other fields like rate
      QCOMPARE(castModel.data(castModel.index(0, PERFORMER_DB_HEADER_KEY::Rate), Qt::ItemDataRole::DisplayRole).toInt(), 99);
    }

    {  // onDeleteRecords ok
      QCOMPARE(castModel.rowCount(), 2);
      castView.clearSelection();
      QVERIFY(!castView.selectionModel()->hasSelection());
      QCOMPARE(castView.onDeleteRecords(), 0);

      castView.selectAll();
      QVERIFY(castView.selectionModel()->hasSelection());
      CastDbViewMocker::MockDeleteRecord() = false;
      QCOMPARE(castView.onDeleteRecords(), 0);

      CastDbViewMocker::MockDeleteRecord() = true;
      QCOMPARE(castView.onDeleteRecords(), 2);
      QCOMPARE(castModel.rowCount(), 0);

      QCOMPARE(castView.onDeleteRecords(), 0);
    }

    {                                                   // onDropDeleteTable ok
      CastDbViewMocker::MockDropDeleteTable() = false;  // user cancel drop/delete
      QVERIFY(castView.onDropDeleteTable(DbManagerHelper::DropOrDeleteE::DELETE));
      QVERIFY(castView.onDropDeleteTable(DbManagerHelper::DropOrDeleteE::DROP));

      CastDbViewMocker::MockDropDeleteTable() = true;  // user accept drop/delete
      QVERIFY(castView.onDropDeleteTable(DbManagerHelper::DropOrDeleteE::DELETE));
      QVERIFY(castModel.columnCount() > 0);
      QVERIFY(castDb.IsTableExist(tableName));
      QVERIFY(castView.onDropDeleteTable(DbManagerHelper::DropOrDeleteE::DROP));
      QVERIFY(!castDb.IsTableExist(tableName));
    }

    {                                                                         // onLoadFromFileSystemStructure ok
      QVERIFY(!castModel.isDirty());                                          // not dirty
      QCOMPARE(castView.onLoadFromFileSystemStructure(), FD_PREPARE_FAILED);  // table was dropped. cannot insert into
      castView.onInitATable();                                                // select in table
      QCOMPARE(castModel.rowCount(), 0);
      QVERIFY(castModel.columnCount() > 0);

      QVERIFY(castDb.IsTableExist(tableName));

      // only .db exist; no file structure
      QSet<QString> beforeDirsSets = tDir.Snapshot(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);  // only .db exist
      QVERIFY(beforeDirsSets.isEmpty());
      QCOMPARE(castView.onLoadFromFileSystemStructure(), 0);

      // prepare folder precondition
      QVERIFY(CreateFileStructure(tDir));
      QCOMPARE(castView.onLoadFromFileSystemStructure(), 5);
      QCOMPARE(castModel.rowCount(), 5);
    }

    {  // onLoadFromPsonDirectory ok, override fields if conflict, contents update ok
      QVERIFY(!castModel.isDirty());

      CastDbViewMocker::MockLoadFromPsonDirectory() = false;
      QCOMPARE(castView.onLoadFromPsonDirectory(), 0);  // no pson at all
      CastDbViewMocker::MockLoadFromPsonDirectory() = true;
      QCOMPARE(castView.onLoadFromPsonDirectory(), 0);  // no pson at all

      QVERIFY(!tDir.exists("SuperHero/Chris Evans/Chris Evans.pson"));
      QVERIFY(!tDir.exists("SuperHero/Chris Hemsworth/Chris Hemsworth.pson"));
      QVERIFY(!tDir.exists("Action/James Caviezel/James Caviezel.pson"));
      QVERIFY(CreateFileStructurePsonFiles(tDir));
      QVERIFY(tDir.exists("SuperHero/Chris Evans/Chris Evans.pson"));
      QVERIFY(tDir.exists("SuperHero/Chris Hemsworth/Chris Hemsworth.pson"));
      QVERIFY(tDir.exists("Action/James Caviezel/James Caviezel.pson"));
      //
      using namespace CastPsonFileHelper;
      castModel.sort(PERFORMER_DB_HEADER_KEY::Name, Qt::AscendingOrder);

      CastDbViewMocker::MockLoadFromPsonDirectory() = false;                                             // user cancel
      QVERIFY(CheckRecordIfEqual(castModel.record(0), "Chris Evans", DEFAULT_RATE, "", "", "SuperHero",  //
                                 -1, -1, "", "", "Chris Evans 1.jpg\nChris Evans 2.jpg", "", true));
      QVERIFY(CheckRecordIfEqual(castModel.record(1), "Chris Hemsworth", DEFAULT_RATE, "", "", "SuperHero",  //
                                 -1, -1, "", "", "Chris Hemsworth.jpg", "", true));
      QCOMPARE(castView.onLoadFromPsonDirectory(), 0);
      QVERIFY(CheckRecordIfEqual(castModel.record(0), "Chris Evans", DEFAULT_RATE, "", "", "SuperHero",  //
                                 -1, -1, "", "", "Chris Evans 1.jpg\nChris Evans 2.jpg", "", true));
      QVERIFY(CheckRecordIfEqual(castModel.record(1), "Chris Hemsworth", DEFAULT_RATE, "", "", "SuperHero",  //
                                 -1, -1, "", "", "Chris Hemsworth.jpg", "", true));

      CastDbViewMocker::MockLoadFromPsonDirectory() = true;  // user accept
      QCOMPARE(castView.onLoadFromPsonDirectory(), 3);       // 3 pson find out and updated
      QCOMPARE(castModel.rowCount(), 5 + 1);                 // James Caviezel added

      castModel.sort(PERFORMER_DB_HEADER_KEY::Name, Qt::AscendingOrder);
      // Chris Evans(0), Chris Hemsworth(1), Chris Pine, Cristiano Ronaldo, James Caviezel(4), Michael Fassbender
      QVERIFY(CheckRecordIfEqual(castModel.record(0), "Chris Evans", 10, "Captain,Steve", "hero,movie star", "SuperHero",
                                 -1, -1, "", "Chris Evans in captain america.mp4", "Chris Evans portait.jpg",  //
                                 "Chris Evans, American movie star", true));                       //
      QVERIFY(CheckRecordIfEqual(castModel.record(1), "Chris Hemsworth", 9, "Thor", "hero,movie star", "SuperHero",
                                 -1, -1, "", "Chris Hemsworth in captain america.mp4", "Chris Hemsworth portait.jpg",               //
                                 "Chris Hemsworth, Australia movie star", true));                                       //
      QVERIFY(CheckRecordIfEqual(castModel.record(4), "James Caviezel", 9, "General Zod", "hero,movie star", "Action",  //
                                 -1, -1, "", "James Caviezel in Man of Steel - General Zod.mp4", "James Caviezel portait.jpg",      //
                                 "James Caviezel, American movie star", true));
    }

    {  // onSyncAllImgsFieldFromImageHost ok, current index should not loose
      QVERIFY(!castView.selectionModel()->hasSelection());
      QCOMPARE(castView.onSyncImgsFieldFromImageHost(), 0);

      QModelIndex firstInd = castModel.index(0, 0);
      castView.setCurrentIndex(firstInd);

      QSignalSpy currentRecordChangedByImgUpdSig(&castView, &CastDBView::currentRecordChanged);
      QCOMPARE(castView.onSyncAllImgsFieldFromImageHost(), 3);  // 3 records images get removed
      QCOMPARE(currentRecordChangedByImgUpdSig.count(), 1);
      QCOMPARE(castView.currentIndex(), firstInd);

      QVERIFY(CheckRecordIfEqual(castModel.record(0), "Chris Evans", 10, "Captain,Steve", "hero,movie star", "SuperHero",
                                 -1, -1, "", "Chris Evans in captain america.mp4", "Chris Evans 1.jpg\nChris Evans 2.jpg",  //
                                 "Chris Evans, American movie star", true));                                    //
      QVERIFY(CheckRecordIfEqual(castModel.record(1), "Chris Hemsworth", 9, "Thor", "hero,movie star", "SuperHero",
                                 -1, -1, "", "Chris Hemsworth in captain america.mp4", "Chris Hemsworth.jpg",                       //
                                 "Chris Hemsworth, Australia movie star", true));                                       //
      QVERIFY(CheckRecordIfEqual(castModel.record(4), "James Caviezel", 9, "General Zod", "hero,movie star", "Action",  //
                                 -1, -1, "", "James Caviezel in Man of Steel - General Zod.mp4", "",                                //
                                 "James Caviezel, American movie star", true));

      QVERIFY(!castModel.isDirty());
      castView.selectionModel()->clearSelection();
    }

    {  // onDumpAllIntoPsonFile ok
      QVERIFY(!castView.selectionModel()->hasSelection());
      QCOMPARE(castView.onDumpIntoPsonFile(), 0);
      castView.selectAll();
      tDir.checkFileContents("SuperHero/Chris Evans/Chris Evans.pson", {"Chris Evans portait.jpg"}, {"Chris Evans 1.jpg", "Chris Evans 2.jpg"});
      tDir.checkFileContents("SuperHero/Chris Hemsworth/Chris Hemsworth.pson", {"Chris Hemsworth portait.jpg"}, {"Chris Hemsworth.jpg"});
      tDir.checkFileContents("Action/James Caviezel/James Caviezel.pson", {"James Caviezel portait.jpg"});

      QCOMPARE(castView.onDumpIntoPsonFile(), 6);  // 3 pson files updated succeed, 3 pson write into succced
      QVERIFY(castView.selectionModel()->hasSelection());
      QCOMPARE(castView.onDumpIntoPsonFile(), 0);  // unchange

      tDir.checkFileContents("SuperHero/Chris Evans/Chris Evans.pson", {"Chris Evans 1.jpg", "Chris Evans 2.jpg"}, {"Chris Evans portait.jpg"});
      tDir.checkFileContents("SuperHero/Chris Hemsworth/Chris Hemsworth.pson", {"Chris Hemsworth.jpg"}, {"Chris Hemsworth portait.jpg"});
      tDir.checkFileContents("Action/James Caviezel/James Caviezel.pson", {}, {"James Caviezel portait.jpg"});

      castView.selectionModel()->clearSelection();
      QVERIFY(!castView.selectionModel()->hasSelection());
      QCOMPARE(castView.onDumpAllIntoPsonFile(), 0);  // 0 pson files updated. all unchange
    }

    {  // onRefreshVidsField ok
       // has no selection
      QVERIFY(!castView.selectionModel()->hasSelection());
      QCOMPARE(castView.onRefreshVidsField(), 0);

      castView.setQueryConfirmIfRowSelectedCountAbove(1);
      QCOMPARE(castView.QUERY_CONFIRM_IF_ROW_SELECTED_COUNT_ABOVE, 1);

      castView.selectAll();
      CastDbViewMocker::MockRefreshVidsField() = false;  // user cancel
      QCOMPARE(castView.onRefreshVidsField(), 0);

      const QString movieDbFileAbsPath = tDir.itemPath("MovieCaseDbModelTest.db");
      const QString movieDbConnName = "MovieCaseDbViewTestConn";
      CastDbViewMocker::MockRefreshVidsField() = true;
      CastDbViewMocker::MockMovieDbAbsFilePath2ConnName() = std::pair<QString, QString>(movieDbFileAbsPath, movieDbConnName);
      {
        const QString movieTableName{DB_TABLE::MOVIES};
        FdBasedDb movieDb{movieDbFileAbsPath, movieDbConnName};
        QVERIFY(movieDb.IsValid());
        QVERIFY(movieDb.CreateTable(movieTableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
        QVERIFY(movieDb.IsTableExist(movieTableName));
        {
          QSet<QByteArray> needInsertFds = {"1001", "1002"};
          // 2 videos of Chris Evans
          QHash<QByteArray, QString> newFd2Pth = {
              {"1001", "videos/superhero/Chris Evans Steve.mp4"},
              {"1002", "videos/superhero/Chris Evans Captain America.mp4"},
          };
          int insertCnt = 0;
          QVERIFY(movieDb.Insert(movieTableName, needInsertFds, newFd2Pth, insertCnt) == FD_OK);
          QCOMPARE(insertCnt, 2);
        }
      }

      QModelIndex firstInd = castModel.index(0, 0);
      castView.setCurrentIndex(firstInd);
      castView.selectAll();
      QSignalSpy currentRecordChangedByVidUpdSig(&castView, &CastDBView::currentRecordChanged);
      // only Chris Evans/Chris Hemsworth/James Caviezel get updated, other 3 does not contains any videos
      QCOMPARE(castView.onRefreshVidsField(), 3);
      QCOMPARE(currentRecordChangedByVidUpdSig.count(), 1);
      QCOMPARE(castView.currentIndex(), firstInd);

      castModel.sort(PERFORMER_DB_HEADER_KEY::Name, Qt::AscendingOrder);
      // Chris Evans(0), Chris Hemsworth(1), Chris Pine, Cristiano Ronaldo, James Caviezel(4), Michael Fassbender
      QVERIFY(CheckRecordIfEqual(castModel.record(0), "Chris Evans", 10, "Captain,Steve", "hero,movie star", "SuperHero",
                                 -1, -1, "", "videos/superhero/Chris Evans Captain America.mp4\nvideos/superhero/Chris Evans Steve.mp4",  //
                                 "Chris Evans 1.jpg\nChris Evans 2.jpg",                                                      //
                                 "Chris Evans, American movie star", true));                                                  //
      QVERIFY(CheckRecordIfEqual(castModel.record(1), "Chris Hemsworth", 9, "Thor", "hero,movie star", "SuperHero",           //
                                 -1, -1, "", "", "Chris Hemsworth.jpg",                                                                   //
                                 "Chris Hemsworth, Australia movie star", true));                                             //
      QVERIFY(CheckRecordIfEqual(castModel.record(4), "James Caviezel", 9, "General Zod", "hero,movie star", "Action",        //
                                 -1, -1, "", "", "",                                                                                      //
                                 "James Caviezel, American movie star", true));
      QCOMPARE(castView.onRefreshAllVidsField(), 0);  // already all updated
      // QCOMPARE(currentRecordChangedByVidUpdSig.count(), 1);   // signal should not emit so still 1
      QVERIFY(!castModel.isDirty());
    }

    {  // onMigrateCastTo ok

    }

    {  // currentRecordChanged ok
      QModelIndex firstInd = castModel.index(0, 0);
      castView.setCurrentIndex(firstInd);
      QSignalSpy signalEmitSpy(&castView, &CastDBView::currentRecordChanged);
      castView.RefreshCurrentRowHtmlContents();
      QCOMPARE(signalEmitSpy.count(), 1);
    }
  }
};

#include "CastDBViewTest.moc"
REGISTER_TEST(CastDBViewTest, false)
