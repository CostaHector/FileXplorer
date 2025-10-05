#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "MovieDBView.h"
#include "EndToExposePrivateMember.h"
#include "PublicVariable.h"
#include "TDir.h"
#include "MountHelper.h"
#include "TableFields.h"
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlQuery>
#include "VideoTestPrecoditionTools.h"
#include "MovieDBActions.h"
#include "AutoRollbackRename.h"

class MovieDBViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  const QString dbName{tDir.itemPath("MovieViewTest.db")};
  const QString connName{"MovieViewTestConn"};
  QString tableName1, tableName2;
  const QString path1 = tDir.itemPath("path1");
  const QString path2 = tDir.itemPath("path2");
 private slots:
  void initTestCase() {
    Configuration().clear();
    QVERIFY(tDir.IsValid());

    const QList<FsNodeEntry> nodes{
        {"path1/Chris Evans.mp4", false, "Chris Evans"},  // 3 videos in path1, tableName1
        {"path1/Chris Hemsworth.mp4", false, "Chris Hemsworth"},
        {"path1/Chris Pine.mp4", false, "Chris Pine"},
        // 2 videos(-1 second Michael Fassbender.mp4 and 5 second Morata.mp4) in path2, tableName2
        {"path2/Michael Fassbender.mp4", false, "Michael Fassbender"},
        {"path2/Cristiano Ronaldo.jpg", false, "Cristiano Ronaldo"},
    };
    QCOMPARE(tDir.createEntries(nodes), 5);
    const QString morataVidAbsPath = tDir.itemPath("path2/Morata.mp4");  // 5second
    bool bVidGenOk = false;
    QByteArray morataVidBa = VideoTestPrecoditionTools::CreateVideoContentNormal(morataVidAbsPath, 5000, &bVidGenOk);
    QVERIFY(!morataVidBa.isEmpty());
    QVERIFY(bVidGenOk);
    QVERIFY(tDir.exists("path2/Morata.mp4"));

    bool bPath1ToTableName{false};
    tableName1 = MountPathTableNameMapper::toTableNameMock(path1, &bPath1ToTableName);
    QVERIFY(bPath1ToTableName);
    QCOMPARE(MountPathTableNameMapper::toMountPathMock(tableName1, &bPath1ToTableName), path1);
    QVERIFY(bPath1ToTableName);

    bool bPath2ToTableName{false};
    tableName2 = MountPathTableNameMapper::toTableNameMock(path2, &bPath2ToTableName);
    QVERIFY(bPath2ToTableName);
    QCOMPARE(MountPathTableNameMapper::toMountPathMock(tableName2, &bPath2ToTableName), path2);
    QVERIFY(bPath2ToTableName);

    MovieDBViewMock::clear();
  }

  void cleanupTestCase() {
    Configuration().clear();
    MovieDBViewMock::clear();
  }

  void movie_view_behavior_correct() {
    QWidget parent;

    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    QVERIFY(dbToolBar.m_tablesCB != nullptr);
    QCOMPARE(dbToolBar.m_tablesCB->count(), 0);
    QVERIFY(!dbToolBar.m_tablesCB->isEditable());  // tableNames can only insert via addItem
    QVERIFY(dbToolBar.m_whereCB != nullptr);
    QVERIFY(dbToolBar.m_whereCB->isEditable());  // where clause can be modified by user input

    QVERIFY(fdDb.IsValid());
    QVERIFY(!fdDb.IsTableExist(tableName1));
    QVERIFY(!fdDb.IsTableExist(tableName2));

    QCOMPARE(dbModel.rowCount(), 0);
    QCOMPARE(movieView.onExportToJson(), -1);  // no tables at all
    QCOMPARE(movieView.onUpdateByJson(), -1);  // no tables at all
    QVERIFY(!movieView.onAuditATable());       // no tables at all

    QVERIFY(movieView.onInitDataBase());  // usually we don't need call it

    {
      // 1 create a table
      // 1.1 user cancel
      MovieDBViewMock::InputATableNameMock() = std::pair<bool, QString>(false, tableName1);
      QVERIFY(!movieView.onCreateATable());
      QVERIFY(!fdDb.IsTableExist(tableName1));

      // 1.2 table name empty
      MovieDBViewMock::InputATableNameMock() = std::pair<bool, QString>(true, "");
      QVERIFY(!movieView.onCreateATable());
      QVERIFY(!fdDb.IsTableExist(tableName1));

      // 1.3 create tableName1 succeed
      MovieDBViewMock::InputATableNameMock() = std::pair<bool, QString>(true, tableName1);
      QVERIFY(movieView.onCreateATable());
      QVERIFY(fdDb.IsTableExist(tableName1));
      QCOMPARE(dbToolBar.m_tablesCB->itemText(0), tableName1);

      // 1.4 table name occupied
      MovieDBViewMock::InputATableNameMock() = std::pair<bool, QString>(true, tableName1);
      QVERIFY(!movieView.onCreateATable());
      QVERIFY(fdDb.IsTableExist(tableName1));

      QCOMPARE(fdDb.CountRow(tableName1), 0);
      QCOMPARE(dbModel.tableName(), tableName1);
      QCOMPARE(dbModel.rowCount(), 0);
    }

    {
      // 2 union multi-table into `DB_TABLE::MOVIES`, insert items from path into table ok
      MovieDBViewMock::ConfirmUnionIntoMock() = true;

      // 2.1 `DB_TABLE::MOVIES` not exist
      QVERIFY(!fdDb.IsTableExist(DB_TABLE::MOVIES));
      QVERIFY(!movieView.onUnionTables());

      // 2.2 only 1 table except `DB_TABLE::MOVIES` no need union at all
      MovieDBViewMock::InputATableNameMock() = std::pair<bool, QString>(true, DB_TABLE::MOVIES);
      QVERIFY(movieView.onCreateATable());
      QCOMPARE(dbToolBar.m_tablesCB->findText(DB_TABLE::MOVIES), dbToolBar.m_tablesCB->count() - 1);
      QVERIFY(fdDb.IsTableExist(DB_TABLE::MOVIES));
      QCOMPARE(fdDb.CountRow(DB_TABLE::MOVIES), 0);
      QCOMPARE(dbModel.tableName(), DB_TABLE::MOVIES);
      QCOMPARE(dbModel.rowCount(), 0);
      QVERIFY(!movieView.onUnionTables());

      // 2.3 user cancel, skip
      MovieDBViewMock::ConfirmUnionIntoMock() = false;
      MovieDBViewMock::InputATableNameMock() = std::pair<bool, QString>(true, tableName2);
      QVERIFY(movieView.onCreateATable());  // once create table, m_tablesCB will insert it at the bottom, then update currentText to new table name
      QCOMPARE(dbToolBar.m_tablesCB->findText(tableName2), dbToolBar.m_tablesCB->count() - 1);
      QCOMPARE(dbToolBar.m_tablesCB->currentText(), tableName2);
      QCOMPARE(fdDb.CountRow(tableName2), 0);
      QVERIFY(fdDb.IsTableExist(tableName2));
      QCOMPARE(dbModel.tableName(), tableName2);
      QVERIFY(!movieView.onUnionTables());

      // 2.4 union 2 empty tables into DB_TABLE::MOVIES ok, still 0 rows
      MovieDBViewMock::ConfirmUnionIntoMock() = true;
      QVERIFY(movieView.onUnionTables());
      QCOMPARE(fdDb.CountRow(DB_TABLE::MOVIES), 0);

      // 2.4 union tableName1 with 3 rows and tableName2 with 0 row into DB_TABLE::MOVIES
      {
        QVERIFY(dbToolBar.m_tablesCB->findText(tableName1) != -1);
        dbToolBar.m_tablesCB->setCurrentText(tableName1);
        emit dbToolBar.movieTableChanged(tableName1);  // signal-slot setCurrentMovieTable should connected
        QCOMPARE(dbModel.tableName(), tableName1);

        // 2.4.1 empty path skip
        MovieDBViewMock::GetAPathFromFileDialogMock() = "";
        QVERIFY(!movieView.onInsertIntoTable());

        // 2.4.2 path user specified differ from mount path from table name, skip
        MovieDBViewMock::GetAPathFromFileDialogMock() = "/path/not/start/with/mountPathOfTableName1";
        QVERIFY(!movieView.onInsertIntoTable());

        // 2.4.3 path start with mount path from table name, but user cancel, skip
        MovieDBViewMock::GetAPathFromFileDialogMock() = path1;
        MovieDBViewMock::ConfirmInsertIntoMock() = false;
        QVERIFY(!movieView.onInsertIntoTable());

        // 2.4.4 path start with mount path from table name, user accept, 3 insert ok
        MovieDBViewMock::ConfirmInsertIntoMock() = true;
        QVERIFY(movieView.onInsertIntoTable());

        QCOMPARE(fdDb.CountRow(tableName1), 3);
        QCOMPARE(dbModel.rowCount(), 3);
        QCOMPARE(movieView.onCountRow(), 3);
      }
      QVERIFY(movieView.onUnionTables());
      QCOMPARE(fdDb.CountRow(DB_TABLE::MOVIES), 3 + 0);

      // 2.5 union tableName1 with 3 rows and tableName2 with 2 rows into DB_TABLE::MOVIES
      {
        QVERIFY(dbToolBar.m_tablesCB->findText(tableName2) != -1);
        dbToolBar.m_tablesCB->setCurrentText(tableName2);
        emit dbToolBar.movieTableChanged(tableName2);  // aka setCurrentMovieTable
        QCOMPARE(dbModel.tableName(), tableName2);
        MovieDBViewMock::GetAPathFromFileDialogMock() = path2;
        MovieDBViewMock::ConfirmInsertIntoMock() = true;
        QVERIFY(movieView.onInsertIntoTable());
        QCOMPARE(fdDb.CountRow(tableName2), 2);
        QCOMPARE(dbModel.rowCount(), 2);
        QCOMPARE(movieView.onCountRow(), 2);
      }
      QVERIFY(movieView.onUnionTables());
      QCOMPARE(fdDb.CountRow(DB_TABLE::MOVIES), 3 + 2);
    }

    using namespace MOVIE_TABLE;
    {
      // onSearchDataBase filter should works ok
      // "path2/Michael Fassbender.mp4"
      // "path2/Morata.mp4"
      QCOMPARE(dbModel.tableName(), tableName2);
      QCOMPARE(dbModel.rowCount(), 2);
      dbToolBar.m_whereCB->setCurrentText(QString(R"(`%1` like "Michael%")").arg(ENUM_2_STR(Name)));
      movieView.onSearchDataBase();
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Name).toString(), "Michael Fassbender.mp4");

      QCOMPARE(dbModel.rowCount(), 1);
      dbToolBar.m_whereCB->setCurrentText(QString(R"(`%1` like "%mp4")").arg(ENUM_2_STR(Name)));
      movieView.onSearchDataBase();
      QCOMPARE(dbModel.rowCount(), 2);
      dbToolBar.m_whereCB->setCurrentText(QString(R"(`%1` like "%all rows not contains me%")").arg(ENUM_2_STR(Name)));
      movieView.onSearchDataBase();
      QCOMPARE(dbModel.rowCount(), 0);
      dbToolBar.m_whereCB->setCurrentText(R"(`Name` LIKE "%")");  // all
      movieView.onSearchDataBase();
      QCOMPARE(dbModel.rowCount(), 2);
    }

    {
      // sort ok
      QCOMPARE(dbModel.tableName(), tableName2);
      QCOMPARE(dbModel.rowCount(), 2);
      // call sort only if no special char exist in table Name
      if (!tableName2.contains("-")) {  // desc
        dbModel.sort((int)MOVIE_TABLE::Name, Qt::SortOrder::DescendingOrder);
        LOG_D("Generated SQL: %s", qPrintable(dbModel.query().lastQuery()));
        // Generated SQL: SELECT "Fd", "PrePathLeft", "PrePathRight", "Name", "Size", "Duration", "Studio", "Cast", "Tags", "PathHash" FROM
        // "_tmp_FileXplorerTest-IThoEl_path2" WHERE `Name` LIKE "%" ORDER BY _tmp_FileXplorerTest-IThoEl_path2."Name" ASC
        QVERIFY2(dbModel.select(), qPrintable(dbModel.lastError().text()));
        // no such column: _tmp_FileXplorerTest Unable to execute statement
      } else {  // desc
        QSqlQuery sqlQry(QString("SELECT * FROM `%1` ORDER BY `%2` DESC").arg(tableName2).arg(ENUM_2_STR(Name)), dbModel.database());
        dbModel.setQuery(sqlQry);
        QVERIFY2(dbModel.lastError().type() == QSqlError::NoError, qPrintable(dbModel.lastError().text()));
      }
      QCOMPARE(dbModel.rowCount(), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Name).toString(), "Morata.mp4");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Name).toString(), "Michael Fassbender.mp4");
    }

    {
      // onSetDurationByVideo ok
      QCOMPARE(dbModel.rowCount(), 2);
      QCOMPARE(dbModel.tableName(), tableName2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Duration).toInt(), 0);
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Duration).toInt(), 0);
      movieView.onSetDurationByVideo();
      QSqlQuery sqlQry(QString("SELECT * FROM `%1` ORDER BY `%2` DESC").arg(tableName2).arg(ENUM_2_STR(Name)), dbModel.database());
      dbModel.setQuery(sqlQry);
      QVERIFY2(dbModel.lastError().type() == QSqlError::NoError, qPrintable(dbModel.lastError().text()));
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Name).toString(), "Morata.mp4");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Name).toString(), "Michael Fassbender.mp4");
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Duration).toInt(), 5000);
      QVERIFY(dbModel.record(1).value(MOVIE_TABLE::Duration).toInt() <= 0);  // invalid video file linux return -1, windows return 0
    }

    auto& inst = g_dbAct();
    emit inst.SET_STUDIO->triggered();
    emit inst.SET_CAST->triggered();
    emit inst.APPEND_CAST->triggered();
    emit inst.REMOVE_CAST->triggered();
    emit inst.SET_TAGS->triggered();
    emit inst.APPEND_TAGS->triggered();
    emit inst.REMOVE_TAGS->triggered();
    {
      // 1.2 set studio ok
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "");
      // 1.2.1 set studio no selection
      MovieDBViewMock::InputStudioNameMock() = std::pair<bool, QString>(true, "Marvel");
      movieView.selectionModel()->clear();
      QCOMPARE(movieView.onSetStudio(), 0);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "");

      // 1.2.2 set studio user cancel
      movieView.selectAll();
      MovieDBViewMock::InputStudioNameMock() = std::pair<bool, QString>(false, "Marvel");
      QCOMPARE(movieView.onSetStudio(), 0);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "");

      // 1.2.3 set studio user accept, ok
      MovieDBViewMock::InputStudioNameMock() = std::pair<bool, QString>(true, "Marvel");
      QCOMPARE(movieView.onSetStudio(), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "Marvel");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "Marvel");

      // 1.2.4 set studio to empty(be regard as clear). user accept, ok
      MovieDBViewMock::InputStudioNameMock() = std::pair<bool, QString>(true, "");
      QCOMPARE(movieView.onSetStudio(), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "");
    }

    {
      // set cast or tags should modify table
      // 1.1 no selection
      movieView.selectionModel()->clear();
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 0);

      // 1.2 user cancel cast set
      movieView.selectAll();
      MovieDBViewMock::InputTagsOrCastsMock() = std::pair<bool, QString>(false, "Cast Morata");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 0);

      // 1.3.1 user accept cast set empty "", ignored
      MovieDBViewMock::InputTagsOrCastsMock() = std::pair<bool, QString>(true, "");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 0);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "");
      // 1.3.1 user accept cast set non empty "Cast Morata", ok
      MovieDBViewMock::InputTagsOrCastsMock() = std::pair<bool, QString>(true, "Cast Morata");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");

      // 1.4.1 user accept cast add empty "", ignored
      MovieDBViewMock::InputTagsOrCastsMock() = std::pair<bool, QString>(true, "");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND), 0);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");
      // 1.4.2 user accept cast add non empty "Cast Morata 2", ok
      MovieDBViewMock::InputTagsOrCastsMock() = std::pair<bool, QString>(true, "Cast Morata 2");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata,Cast Morata 2");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata,Cast Morata 2");

      // 1.5 user accept cast remove empty "", ignored
      MovieDBViewMock::InputTagsOrCastsMock() = std::pair<bool, QString>(true, "");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::REMOVE), 0);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata,Cast Morata 2");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata,Cast Morata 2");
      // 1.5 user accept cast remove non empty "Cast Morata"(full match), ok
      MovieDBViewMock::InputTagsOrCastsMock() = std::pair<bool, QString>(true, "Cast Morata");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::REMOVE), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata 2");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata 2");

      // 1.6 user cancel cast clear
      MovieDBViewMock::clearTagsOrCastsMock() = false;
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::CLEAR), 0);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata 2");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata 2");
      // 1.7 user accept cast clear
      MovieDBViewMock::clearTagsOrCastsMock() = true;
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::CLEAR), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "");

      // 1.8 user accept tags set
      MovieDBViewMock::InputTagsOrCastsMock() = std::pair<bool, QString>(true, "Football,Spanish");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::SET), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Tags).toString(), "Football,Spanish");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Tags).toString(), "Football,Spanish");
      // 1.9 user accept tags clear
      MovieDBViewMock::clearTagsOrCastsMock() = true;
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::CLEAR), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Tags).toString(), "");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Tags).toString(), "");
    }

    {
      // export to json, json file should be changed updated if (must field studio/cast/tags all valid will get updated)

      // 1.1 modifed studio/cast/tags field to both valid
      movieView.selectAll();
      MovieDBViewMock::InputStudioNameMock() = std::pair<bool, QString>(true, "Real Madrid");
      QCOMPARE(movieView.onSetStudio(), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "Real Madrid");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "Real Madrid");
      MovieDBViewMock::InputTagsOrCastsMock() = std::pair<bool, QString>(true, "Cast Morata");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");
      MovieDBViewMock::InputTagsOrCastsMock() = std::pair<bool, QString>(true, "FootBall");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::APPEND), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Tags).toString(), "FootBall");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Tags).toString(), "FootBall");

      // 1.2 dirty cannot export
      QVERIFY(dbModel.isDirty());
      QCOMPARE(movieView.onExportToJson(), -1);

      // 1.3 not dirty export ok
      QVERIFY(movieView.onSubmit());
      QVERIFY(!dbModel.isDirty());
      movieView.selectionModel()->clear();
      QVERIFY(!tDir.exists("path2/Michael Fassbender.json"));
      QVERIFY(!tDir.exists("path2/Morata.json"));
      QCOMPARE(movieView.onExportToJson(), 2);
      QVERIFY(tDir.exists("path2/Michael Fassbender.json"));
      QVERIFY(tDir.exists("path2/Morata.json"));

      // 1.4 modify them Studio mod then revert, cast/tags mod ok
      movieView.selectAll();
      MovieDBViewMock::InputStudioNameMock() = std::pair<bool, QString>(true, "X-Men");
      QCOMPARE(movieView.onSetStudio(), 2);  // it has not been write into db
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "X-Men");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "X-Men");
      QSet<QString> expectStudiosInDatabase{"Real Madrid"};
      QSet<QString> actualStudiosInDatabase;
      QVERIFY(fdDb.QueryPK(tableName2, ENUM_2_STR(Studio), actualStudiosInDatabase));
      QCOMPARE(actualStudiosInDatabase, expectStudiosInDatabase);
      QVERIFY(dbModel.isDirty());
      QVERIFY(movieView.onRevert());  // revert can also let it not dirty
      QVERIFY(!dbModel.isDirty());
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "Real Madrid");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "Real Madrid");
      MovieDBViewMock::clearTagsOrCastsMock() = true;
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::CLEAR), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::CLEAR), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Tags).toString(), "");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Tags).toString(), "");

      // 1.4 table dirty cannot Update By Json, skip
      QVERIFY(dbModel.isDirty());
      QCOMPARE(movieView.onUpdateByJson(), -1);
      //

      // Update By Json, contents of studio/cast/tags in table should in consistence with json
      QVERIFY(movieView.onSubmit());
      QVERIFY(!dbModel.isDirty());
      QCOMPARE(movieView.onUpdateByJson(), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "Real Madrid");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "Real Madrid");
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Tags).toString(), "FootBall");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Tags).toString(), "FootBall");
    }

    {
      // onDeleteFromTable, table still exist. onCountRow should be correct
      const QString delete0RecordWhereClause = QString(R"(`%1` LIKE "%2")").arg(ENUM_2_STR(Name)).arg("%No records contains me%");
      const QString delete1RecordWhereClause = QString(R"(`%1` LIKE "%2")").arg(ENUM_2_STR(Name)).arg("Morata%");
      const QString deleteAllRecordWhereClause = "";
      // user cancel
      MovieDBViewMock::InputADeleteWhereClauseMock() = std::pair<bool, QString>(false, delete1RecordWhereClause);
      QCOMPARE(movieView.onDeleteFromTable(), -1);
      QCOMPARE(dbModel.rowCount(), 2);

      // user accept
      MovieDBViewMock::InputADeleteWhereClauseMock() = std::pair<bool, QString>(true, delete0RecordWhereClause);
      QCOMPARE(movieView.onDeleteFromTable(), 0);
      QCOMPARE(dbModel.rowCount(), 2);

      MovieDBViewMock::InputADeleteWhereClauseMock() = std::pair<bool, QString>(true, delete1RecordWhereClause);
      QCOMPARE(movieView.onDeleteFromTable(), 1);
      QCOMPARE(dbModel.rowCount(), 1);

      MovieDBViewMock::InputADeleteWhereClauseMock() = std::pair<bool, QString>(true, deleteAllRecordWhereClause);
      QCOMPARE(movieView.onDeleteFromTable(), 1);
      QCOMPARE(dbModel.rowCount(), 0);
    }

    {
      // onAuditATable,
      // files not exist any more(if Fds not exist before, but now not exist) will be delete from this table,
      // files new added(if Fds not exist before) will be insert into this table
      // files renamed(if Fds unchange but name changed) will be updated into this table

      // recover tables
      MovieDBViewMock::GetAPathFromFileDialogMock() = path2;
      MovieDBViewMock::ConfirmInsertIntoMock() = true;
      QVERIFY(movieView.onInsertIntoTable());
      QCOMPARE(dbModel.rowCount(), 2);

      // 1.0 path not match table mount path, cannot audit
      QVERIFY(movieView.onRevert());
      QVERIFY(!dbModel.isDirty());
      MovieDBViewMock::GetAPathFromFileDialogMock() = path1;
      QVERIFY(!movieView.onAuditATable());

      AutoRollbackRename autoRollRenamer(tDir.path(), "path2/Morata.mp4", "path2/Alvaro Morata.mp4");
      QVERIFY(autoRollRenamer.Execute());
      QVERIFY(!tDir.exists("path2/Morata.mp4"));
      QVERIFY(tDir.exists("path2/Alvaro Morata.mp4"));
      QDir dir(tDir);
      QVERIFY(dir.remove("path2/Michael Fassbender.mp4"));
      QVERIFY(!tDir.exists("path2/Michael Fassbender.mp4"));

      MovieDBViewMock::GetAPathFromFileDialogMock() = path2;
      QVERIFY(movieView.onAuditATable());
      QCOMPARE(dbModel.rowCount(), 1);                           // now it been recovered
      QSet<QString> expectNamesInDatabase{"Alvaro Morata.mp4"};  // no Morata.mp4 anymore, no Michael Fassbender.mp4 anymore
      QSet<QString> actualNamesInDatabase;
      QVERIFY(fdDb.QueryPK(tableName2, ENUM_2_STR(Name), actualNamesInDatabase));
      QCOMPARE(actualNamesInDatabase, expectNamesInDatabase);

      // 2.0 dirty cannot audit
      MovieDBViewMock::GetAPathFromFileDialogMock() = path2;
      movieView.selectAll();
      MovieDBViewMock::InputStudioNameMock() = std::pair<bool, QString>(true, "Spanish Real Madrid");
      QCOMPARE(movieView.onSetStudio(), 1);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "Spanish Real Madrid");
      QVERIFY(dbModel.isDirty());
      QVERIFY(!movieView.onAuditATable());
    }

    {
      // onDropATable, table should not exist anymore
      MovieDBSearchToolBarMock::QryDropWhichTableMock() = std::pair<bool, QString>(false, tableName2);
      QVERIFY(!movieView.onDropATable());

      MovieDBSearchToolBarMock::QryDropWhichTableMock() = std::pair<bool, QString>(true, "");  // empty tablename
      QVERIFY(!movieView.onDropATable());

      MovieDBSearchToolBarMock::QryDropWhichTableMock() = std::pair<bool, QString>(true, "inexist table");
      QVERIFY(!movieView.onDropATable());

      QVERIFY(fdDb.IsTableExist(tableName2));
      MovieDBSearchToolBarMock::QryDropWhichTableMock() = std::pair<bool, QString>(true, tableName2);
      QVERIFY(movieView.onDropATable());
      QVERIFY(!fdDb.IsTableExist(tableName2));

      QVERIFY(!movieView.onDropATable());  // table already dropped
    }

    {
      movieView.onSubmit();
      movieView.onRevert();
    }
    movieView.close();
  }
};

// sudo fdisk -l
// find in Device colums
// mkdir -p /mnt/DISKS/DD2_4T
// sudo mount /dev/nvme0n1p8 /mnt/DISKS/DD2_4T
// C:/home/DISKS/DD2_4T
#include "MovieDBViewTest.moc"
REGISTER_TEST(MovieDBViewTest, false)
