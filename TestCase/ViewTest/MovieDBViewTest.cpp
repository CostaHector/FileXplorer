#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
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
#include "VideoDurationGetter.h"
#include "AutoRollbackRename.h"

#include "UserInteractiveMock.h"
#include "MountPathTableNameMapperMock.h"
#include "VideoDurationGetterMock.h"
#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class MovieDBViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  const QString dbName{tDir.itemPath("MovieViewTest.db")};
  const QString connName{"MovieViewTestConn"};
  const QString path1 = tDir.itemPath("path1");
  const QString path2 = tDir.itemPath("path2");
  bool path1ToTableName1Ok{false};
  bool path2ToTableName2Ok{false};
  const QString tableName1 = MountPathTableNameMapperMock::invokeToTableName(path1, &path1ToTableName1Ok);
  const QString tableName2 = MountPathTableNameMapperMock::invokeToTableName(path2, &path2ToTableName2Ok);
  const QList<FsNodeEntry> nodes{
      // 3 videos in path1, tableName1
      {"path1/Chris Evans.mp4", false, "Chris Evans"},
      {"path1/Chris Hemsworth.mp4", false, "Chris Hemsworth"},
      {"path1/Chris Pine.mp4", false, "Chris Pine"},
      // 2 videos in path2, tableName2
      {"path2/Michael Fassbender.mp4", false, "Michael Fassbender"},
      {"path2/Cristiano Ronaldo.jpg", false, "Cristiano Ronaldo"},
      {"path2/Morata.mp4", false, "Contents in Morata.mp4"},
  };
  const QHash<QString, int> vidspath2Duration{
      {tDir.itemPath("path1/Chris Evans.mp4"), 0},  // broken file
      {tDir.itemPath("path1/Chris Hemsworth.mp4"), 8000},    {tDir.itemPath("path1/Chris Pine.mp4"), 7000},
      {tDir.itemPath("path2/Michael Fassbender.mp4"), 6000}, {tDir.itemPath("path2/Morata.mp4"), 5000},
  };
 private slots:
  void initTestCase() {
    Configuration().clear();
    {
      QVERIFY(tDir.IsValid());
      QCOMPARE(tDir.createEntries(nodes), 6);
      VideoDurationGetterMock::PresetVidsDuration(vidspath2Duration);
    }

    {  // path -> table name -> path. 预期: 路径->表名->路径 可逆
      QVERIFY(path1ToTableName1Ok);
      QVERIFY(path2ToTableName2Ok);

      bool tableName1ToPath1Ok = false;
      bool tableName2ToPath2Ok = false;
      using namespace MountPathTableNameMapperMock;
      QCOMPARE(invokeToMountPath(tableName1, &tableName1ToPath1Ok), path1);
      QCOMPARE(invokeToMountPath(tableName2, &tableName2ToPath2Ok), path2);
      QVERIFY(tableName1ToPath1Ok);
      QVERIFY(tableName2ToPath2Ok);
    }
  }

  void cleanupTestCase() { Configuration().clear(); }

  void init() {
    GlobalMockObject::reset();
    using namespace MountPathTableNameMapper;
    using namespace MountPathTableNameMapperMock;
    MOCKER(toMountPath).stubs().will(invoke(invokeToMountPath));
    MOCKER(toTableName).stubs().will(invoke(invokeToTableName));
    MOCKER(QInputDialog::getItem).stubs().will(invoke(UserInteractiveMock::InputDialog::invoke_getItem));
    using namespace VideoDurationGetterMock;
    MOCKER(VideoDurationGetter::GetLengthQuickStatic).stubs().will(invoke(invokeGetLengthQuickStatic));
    MOCKER(VideoDurationGetter::GetLengthsQuickStatic).stubs().will(invoke(invokeGetLengthsQuickStatic));
  }

  void cleanup() {  //
    GlobalMockObject::verify();
    if (QFile::exists(dbName)) {
      QVERIFY(QFile::remove(dbName));
    }
  }

  void default_behavior_ok() {
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
  }

  void create_tableName1_ok() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    // 1 create a table
    // 1.1 user cancel
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(false, tableName1);
    QVERIFY(!movieView.onCreateATable());
    QVERIFY(!fdDb.IsTableExist(tableName1));

    // 1.2 table name empty
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "");
    QVERIFY(!movieView.onCreateATable());
    QVERIFY(!fdDb.IsTableExist(tableName1));

    // 1.3 create tableName1 succeed
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName1);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName1));
    QCOMPARE(dbToolBar.m_tablesCB->itemText(0), tableName1);

    // 1.4 table name occupied
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName1);
    QVERIFY(!movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName1));

    QCOMPARE(fdDb.CountRow(tableName1), 0);
    QCOMPARE(dbModel.tableName(), tableName1);
    QCOMPARE(dbModel.rowCount(), 0);
  }

  void insert_into_tableName1_ok() {
    MOCKER(QFileDialog::getExistingDirectory)
        .stubs()
        .will(returnValue(QString()))
        .then(returnValue(QString("/path/not/start/with/mountPathOfTableName1")))
        .then(returnValue(path1))
        .then(returnValue(path1));

    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    // 1. create tableName1 ok
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName1);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName1));
    QCOMPARE(dbToolBar.m_tablesCB->itemText(0), tableName1);

    // 1.4.1 empty path skip
    QVERIFY(!movieView.onInsertIntoTable());
    QCOMPARE(fdDb.CountRow(tableName1), 0);

    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question)
        .stubs()
        .will(returnValue(QMessageBox::StandardButton::No))
        .then(returnValue(QMessageBox::StandardButton::Yes));
    // 1.4.2 path user specified differ from mount path from table name, skip
    QVERIFY(!movieView.onInsertIntoTable());
    QCOMPARE(fdDb.CountRow(tableName1), 0);

    // 1.4.3 path start with mount path from table name, but user cancel, skip
    QVERIFY(!movieView.onInsertIntoTable());
    QCOMPARE(fdDb.CountRow(tableName1), 0);

    // 1.4.4 path start with mount path from table name, user accept, 3 insert ok
    QVERIFY(movieView.onInsertIntoTable());
    QCOMPARE(fdDb.CountRow(tableName1), 3);
    QCOMPARE(dbModel.rowCount(), 3);
    QCOMPARE(movieView.onCountRow(), 3);
  }

  void union_tableName1_and_tableName2_into_dstTable_ok() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    // 1. create tableName1 ok
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName1);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName1));
    QCOMPARE(dbToolBar.m_tablesCB->itemText(0), tableName1);

    // 2 union multi-table into `DB_TABLE::MOVIES` ok and insert items from specified path into table ok
    // 2.1 destination table `DB_TABLE::MOVIES` not exist
    QVERIFY(!fdDb.IsTableExist(DB_TABLE::MOVIES));
    QVERIFY(!movieView.onUnionTables());

    // 2.2 only 1 table(except `DB_TABLE::MOVIES`) no need union at all
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, DB_TABLE::MOVIES);
    QVERIFY(movieView.onCreateATable());
    QCOMPARE(dbToolBar.m_tablesCB->findText(DB_TABLE::MOVIES), dbToolBar.m_tablesCB->count() - 1);
    QVERIFY(fdDb.IsTableExist(DB_TABLE::MOVIES));
    QCOMPARE(fdDb.CountRow(DB_TABLE::MOVIES), 0);
    QCOMPARE(dbModel.tableName(), DB_TABLE::MOVIES);
    QCOMPARE(dbModel.rowCount(), 0);
    QVERIFY(!movieView.onUnionTables());

    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question)
        .stubs()
        .will(returnValue(QMessageBox::StandardButton::No))
        .then(returnValue(QMessageBox::StandardButton::Yes));
    // 2.3 user cancel, skip
    // once create table, m_tablesCB will insert it at the bottom, then update currentText to new table name
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName2);
    QVERIFY(movieView.onCreateATable());
    QCOMPARE(dbToolBar.m_tablesCB->findText(tableName2), dbToolBar.m_tablesCB->count() - 1);
    QCOMPARE(dbToolBar.m_tablesCB->currentText(), tableName2);
    QCOMPARE(fdDb.CountRow(tableName2), 0);
    QVERIFY(fdDb.IsTableExist(tableName2));
    QCOMPARE(dbModel.tableName(), tableName2);
    QVERIFY(!movieView.onUnionTables());  // No

    // 2.4 union 2 empty tables into DB_TABLE::MOVIES ok, still 0 rows
    QVERIFY(movieView.onUnionTables());  // Yes
    QCOMPARE(fdDb.CountRow(DB_TABLE::MOVIES), 0);

    MOCKER(QFileDialog::getExistingDirectory)
        .stubs()  //
        .will(returnValue(path1))
        .then(returnValue(path2));

    // 2.4.0 insert 3 videos into tableName1
    {
      QVERIFY(dbToolBar.m_tablesCB->findText(tableName1) != -1);
      dbToolBar.m_tablesCB->setCurrentText(tableName1);
      emit dbToolBar.movieTableChanged(tableName1);  // signal-slot setCurrentMovieTable should connected
      QCOMPARE(dbModel.tableName(), tableName1);
      QCOMPARE(dbModel.rowCount(), 0);
      QVERIFY(movieView.onInsertIntoTable());
      QCOMPARE(fdDb.CountRow(tableName1), 3);
      QCOMPARE(dbModel.rowCount(), 3);
      QCOMPARE(movieView.onCountRow(), 3);
    }
    // 2.4.1 union(3 tableName1, 0 tableName2) => 3 DB_TABLE::MOVIES
    QVERIFY(movieView.onUnionTables());
    QCOMPARE(fdDb.CountRow(DB_TABLE::MOVIES), 3 + 0);

    // 2.5.0 insert 2 videos into tableName2
    {
      QVERIFY(dbToolBar.m_tablesCB->findText(tableName2) != -1);
      dbToolBar.m_tablesCB->setCurrentText(tableName2);
      emit dbToolBar.movieTableChanged(tableName2);  // aka setCurrentMovieTable
      QCOMPARE(dbModel.tableName(), tableName2);
      QCOMPARE(dbModel.rowCount(), 0);
      QVERIFY(movieView.onInsertIntoTable());
      QCOMPARE(fdDb.CountRow(tableName2), 2);
      QCOMPARE(dbModel.rowCount(), 2);
      QCOMPARE(movieView.onCountRow(), 2);
    }
    // 2.5.1 union(3 tableName1, 2 tableName2) => 5 DB_TABLE::MOVIES
    QVERIFY(movieView.onUnionTables());
    QCOMPARE(fdDb.CountRow(DB_TABLE::MOVIES), 3 + 2);
  }

  void filter_where_ok() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    // 1. create tableName2 ok
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName2);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName2));
    QCOMPARE(dbModel.tableName(), tableName2);
    QCOMPARE(dbToolBar.m_tablesCB->itemText(0), tableName2);

    MOCKER(QFileDialog::getExistingDirectory).stubs().will(returnValue(path2));
    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question).stubs().will(returnValue(QMessageBox::StandardButton::Yes));
    QVERIFY(movieView.onInsertIntoTable());
    QCOMPARE(fdDb.CountRow(tableName2), 2);
    QCOMPARE(dbModel.rowCount(), 2);
    QCOMPARE(movieView.onCountRow(), 2);

    // SetFilterAndSelect(dbToolBar.GetCurrentWhereClause()) filter should works ok
    // "path2/Michael Fassbender.mp4"
    // "path2/Morata.mp4"
    using namespace MOVIE_TABLE;
    dbToolBar.m_whereCB->setCurrentText(QString(R"(`%1` like "Michael%")").arg(ENUM_2_STR(Name)));
    dbModel.SetFilterAndSelect(dbToolBar.GetCurrentWhereClause());
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Name).toString(), "Michael Fassbender.mp4");
    QCOMPARE(dbModel.rowCount(), 1);

    dbToolBar.m_whereCB->setCurrentText(QString(R"(`%1` like "%mp4")").arg(ENUM_2_STR(Name)));
    dbModel.SetFilterAndSelect(dbToolBar.GetCurrentWhereClause());
    QCOMPARE(dbModel.rowCount(), 2);

    dbToolBar.m_whereCB->setCurrentText(QString(R"(`%1` like "%all rows not contains me%")").arg(ENUM_2_STR(Name)));
    dbModel.SetFilterAndSelect(dbToolBar.GetCurrentWhereClause());
    QCOMPARE(dbModel.rowCount(), 0);

    dbToolBar.m_whereCB->setCurrentText(R"(`Name` LIKE "%")");  // all pass
    dbModel.SetFilterAndSelect(dbToolBar.GetCurrentWhereClause());
    QCOMPARE(dbModel.rowCount(), 2);
  }

  void sort_ok() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    // 1. create tableName2 ok
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName2);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName2));
    QCOMPARE(dbModel.tableName(), tableName2);

    MOCKER(QFileDialog::getExistingDirectory).stubs().will(returnValue(path2));
    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question).stubs().will(returnValue(QMessageBox::StandardButton::Yes));
    QVERIFY(movieView.onInsertIntoTable());
    QCOMPARE(fdDb.CountRow(tableName2), 2);
    QCOMPARE(dbModel.rowCount(), 2);

    // 表名无连字符时才可以调用sort实现排序 call sort only if no special char exist in table Name
    if (!tableName2.contains("-")) {
      dbModel.sort((int)MOVIE_TABLE::Name, Qt::SortOrder::DescendingOrder);
      LOG_D("Generated SQL: %s", qPrintable(dbModel.query().lastQuery()));
      // Generated SQL: SELECT "Fd", "PrePathLeft", "PrePathRight", "Name", "Size", "Duration", "Studio", "Cast", "Tags", "PathHash" FROM
      // "_tmp_FileXplorerTest-IThoEl_path2" WHERE `Name` LIKE "%" ORDER BY _tmp_FileXplorerTest-IThoEl_path2."Name" ASC
      QVERIFY2(dbModel.select(), qPrintable(dbModel.lastError().text()));
      // no such column: _tmp_FileXplorerTest Unable to execute statement
    } else {  // 否则使用QSqlQuery实现排序(通用)
      QSqlQuery sqlQry(QString("SELECT * FROM `%1` ORDER BY `%2` DESC").arg(tableName2).arg(ENUM_2_STR(Name)), dbModel.database());
      dbModel.setQuery(sqlQry);
      QVERIFY2(dbModel.lastError().type() == QSqlError::NoError, qPrintable(dbModel.lastError().text()));
    }
    QCOMPARE(dbModel.rowCount(), 2);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Name).toString(), "Morata.mp4");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Name).toString(), "Michael Fassbender.mp4");
  }

  void set_duration_by_video_ok() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName2);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName2));
    QCOMPARE(dbModel.tableName(), tableName2);

    MOCKER(QFileDialog::getExistingDirectory).stubs().will(returnValue(path2));
    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question).stubs().will(returnValue(QMessageBox::StandardButton::Yes));
    // 1. 预期插入记录时不会填写Duration字段
    QVERIFY(movieView.onInsertIntoTable());
    QCOMPARE(fdDb.CountRow(tableName2), 2);
    QCOMPARE(dbModel.rowCount(), 2);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Duration).toInt(), 0);
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Duration).toInt(), 0);
    // 2. 根据视频协议标准读取文件时长
    movieView.onSetDurationByVideo();

    // 3. 按时长升序排列 Morata.mp4(5000ms) < Michael Fassbender.mp4(6000ms)
    using namespace MOVIE_TABLE;
    QSqlQuery sqlQry(QString("SELECT * FROM `%1` ORDER BY `%2` ASC").arg(tableName2).arg(ENUM_2_STR(Duration)), dbModel.database());
    dbModel.setQuery(sqlQry);
    QVERIFY2(dbModel.lastError().type() == QSqlError::NoError, qPrintable(dbModel.lastError().text()));
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Name).toString(), "Morata.mp4");
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Duration).toInt(), 5000);
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Name).toString(), "Michael Fassbender.mp4");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Duration).toInt(), 6000);
  }

  void no_row_selected_action_will_ignored() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName2);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName2));
    QCOMPARE(dbModel.tableName(), tableName2);

    MOCKER(QFileDialog::getExistingDirectory).stubs().will(returnValue(path2));
    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question)  //
        .stubs()
        .will(returnValue(QMessageBox::StandardButton::Yes));
    QVERIFY(movieView.onInsertIntoTable());
    QCOMPARE(dbModel.rowCount(), 2);

    MOCKER(FdBasedDbModel::setDataStatic).expects(never());

    movieView.selectionModel()->clear();
    auto& inst = g_dbAct();
    emit inst.SET_STUDIO->triggered();
    emit inst.SET_CAST->triggered();
    emit inst.APPEND_CAST->triggered();
    emit inst.REMOVE_CAST->triggered();
    emit inst.SET_TAGS->triggered();
    emit inst.APPEND_TAGS->triggered();
    emit inst.REMOVE_TAGS->triggered();
  }

  void set_studio_ok() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName2);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName2));
    QCOMPARE(dbModel.tableName(), tableName2);

    MOCKER(QFileDialog::getExistingDirectory).stubs().will(returnValue(path2));
    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question)  //
        .stubs()
        .will(returnValue(QMessageBox::StandardButton::Yes));
    QVERIFY(movieView.onInsertIntoTable());
    QCOMPARE(dbModel.rowCount(), 2);

    // 1.2.0 set studio ok
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "");

    movieView.selectionModel()->clear();
    // 1.2.1 no selection: set studio return 0
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Marvel");
    QCOMPARE(movieView.onSetStudio(), 0);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "");

    movieView.selectAll();
    // 1.2.2 all 2 row selected, user cancel: set studio return 0
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(false, "Marvel");
    QCOMPARE(movieView.onSetStudio(), 0);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "");

    // 1.2.3 all 2 row selected, user accept: set studio return 2 row
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Marvel");
    QCOMPARE(movieView.onSetStudio(), 2);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "Marvel");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "Marvel");

    // 1.2.4 all 2 row selected, user accept: set studio to empty(be regard as clear) return 2 row
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "");
    QCOMPARE(movieView.onSetStudio(), 2);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "");
  }

  void set_cast_json_set_append_remove_ok() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName2);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName2));
    QCOMPARE(dbModel.tableName(), tableName2);

    MOCKER(QFileDialog::getExistingDirectory).stubs().will(returnValue(path2));
    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question)  //
        .stubs()
        .will(returnValue(QMessageBox::StandardButton::Yes));
    QVERIFY(movieView.onInsertIntoTable());
    QCOMPARE(dbModel.rowCount(), 2);

    // set cast or tags should modify table
    // CAST SET no selection
    movieView.selectionModel()->clear();
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 0);

    // CAST SET "Cast Morata" Cancel
    movieView.selectAll();
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(false, "Cast Morata");
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 0);

    // CAST SET "Cast Morata" Apply
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Cast Morata");
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 2);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");

    // CAST SET "" Apply, will ignored
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "");
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 0);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");

    // 2.4.1 user accept cast add empty "", ignored
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "");
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND), 0);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");

    // 2.4.2 user accept cast add non empty "Cast Morata 2", ok
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Cast Morata 2");
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND), 2);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata,Cast Morata 2");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata,Cast Morata 2");

    // 2.5 user accept cast remove empty "", ignored
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "");
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::REMOVE), 0);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata,Cast Morata 2");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata,Cast Morata 2");

    // 2.5 user accept cast remove non empty "Cast Morata"(full match), ok
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Cast Morata");
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::REMOVE), 2);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata 2");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata 2");
  }

  void set_cast_json_clear_ok() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName2);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName2));
    QCOMPARE(dbModel.tableName(), tableName2);

    MOCKER(QFileDialog::getExistingDirectory).stubs().will(returnValue(path2));
    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question)  //
        .stubs()
        .will(returnValue(QMessageBox::StandardButton::Yes))  // onInsertIntoTable
        .then(returnValue(QMessageBox::StandardButton::No))   // onSetCastOrTags, 2.6
        .then(returnValue(QMessageBox::StandardButton::Yes))  // onSetCastOrTags, 2.7
        .then(returnValue(QMessageBox::StandardButton::Yes))  // onSetCastOrTags, 2.9
        ;
    QVERIFY(movieView.onInsertIntoTable());
    QCOMPARE(dbModel.rowCount(), 2);

    movieView.selectAll();

    // 2.5 user apply set cast "Cast Morata 2"
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Cast Morata 2");
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET), 2);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata 2");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata 2");

    // 2.6 user cancel cast CLEAR: NO
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::CLEAR), 0);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata 2");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata 2");

    // 2.7 user accept cast CLEAR: YES
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::CLEAR), 2);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "");

    // 2.8 user accept tags set
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Football,Spanish");
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::SET), 2);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Tags).toString(), "Football,Spanish");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Tags).toString(), "Football,Spanish");

    // 2.9 user accept tags CLEAR: YES
    QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::CLEAR), 2);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Tags).toString(), "");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Tags).toString(), "");
  }

  void export_to_json_update_by_json_ok() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName2);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName2));
    QCOMPARE(dbModel.tableName(), tableName2);

    MOCKER(QFileDialog::getExistingDirectory).stubs().will(returnValue(path2));
    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question)  //
        .stubs()
        .will(returnValue(QMessageBox::StandardButton::Yes))  // onInsertIntoTable and onSetCastOrTags all return ok
        ;

    QVERIFY(movieView.onInsertIntoTable());
    QCOMPARE(dbModel.rowCount(), 2);

    movieView.selectAll();
    // 3.1 modifed studio/cast/tags field to both valid
    {
      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Real Madrid");
      QCOMPARE(movieView.onSetStudio(), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "Real Madrid");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "Real Madrid");
      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Cast Morata");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "Cast Morata");
      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "FootBall");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::APPEND), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Tags).toString(), "FootBall");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Tags).toString(), "FootBall");
    }

    {
      // 4.1 dirty, cannot export
      QVERIFY(dbModel.isDirty());
      QCOMPARE(movieView.onExportToJson(), -1);

      // 4.2 not dirty export ok
      // export to json, json file should be changed updated if (must field studio/cast/tags all valid will get updated)
      QVERIFY(movieView.onSubmit());
      QVERIFY(!dbModel.isDirty());
      movieView.selectionModel()->clear();
      QVERIFY(!tDir.exists("path2/Michael Fassbender.json"));
      QVERIFY(!tDir.exists("path2/Morata.json"));
      QCOMPARE(movieView.onExportToJson(), 2);
      QVERIFY(tDir.exists("path2/Michael Fassbender.json"));
      QVERIFY(tDir.exists("path2/Morata.json"));
    }

    {
      // 5.1 update by json
      movieView.selectAll();
      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "X-Men");
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
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::CLEAR), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Cast).toString(), "");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Cast).toString(), "");
      QCOMPARE(movieView.onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::CLEAR), 2);
      QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Tags).toString(), "");
      QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Tags).toString(), "");

      // 5.2 table dirty cannot Update By Json, skip
      QVERIFY(dbModel.isDirty());
      QCOMPARE(movieView.onUpdateByJson(), -1);

      // 5.3 Update By Json, contents of studio/cast/tags in table should in consistence with json
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
  }

  void delete_from_table_ok() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question)
        .stubs()                                               //
        .will(returnValue(QMessageBox::StandardButton::Yes));  // onInsertIntoTable

    // create tableName2 ok
    {
      UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName2);
      QVERIFY(movieView.onCreateATable());
      QVERIFY(fdDb.IsTableExist(tableName2));
      QCOMPARE(dbModel.tableName(), tableName2);

      MOCKER(QFileDialog::getExistingDirectory).stubs().will(returnValue(path2));
      QVERIFY(movieView.onInsertIntoTable());
      QCOMPARE(dbModel.rowCount(), 2);
    }

    // onDeleteFromTable, table still exist. onCountRow should be correct
    const QString delete0RecordWhereClause = QString(R"(`%1` LIKE "%2")").arg(ENUM_2_STR(Name)).arg("%No records contains me%");
    const QString delete1RecordWhereClause = QString(R"(`%1` LIKE "%2")").arg(ENUM_2_STR(Name)).arg("Morata%");
    const QString deleteAllRecordWhereClause = "";
    // user cancel
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(false, delete1RecordWhereClause);
    QCOMPARE(movieView.onDeleteFromTable(), -1);
    QCOMPARE(dbModel.rowCount(), 2);

    // user accept
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, delete0RecordWhereClause);
    QCOMPARE(movieView.onDeleteFromTable(), 0);
    QCOMPARE(dbModel.rowCount(), 2);

    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, delete1RecordWhereClause);
    QCOMPARE(movieView.onDeleteFromTable(), 1);
    QCOMPARE(dbModel.rowCount(), 1);

    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, deleteAllRecordWhereClause);
    QCOMPARE(movieView.onDeleteFromTable(), 1);
    QCOMPARE(dbModel.rowCount(), 0);
  }

  void audit_a_table_ok() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question)
        .stubs()                                               //
        .will(returnValue(QMessageBox::StandardButton::Yes));  // onInsertIntoTable

    MOCKER(QFileDialog::getExistingDirectory)
        .stubs()                   //
        .will(returnValue(path2))  // 1.1
        .then(returnValue(path1))  // 2.1
        .then(returnValue(path2))  // 3.1
        .then(returnValue(path2))  // 4.1
        ;

    // create tableName2 ok
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName2);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName2));
    QCOMPARE(dbModel.tableName(), tableName2);

    // 1.1 读取path2路径下的文件到tableName2. use `path2`
    QVERIFY(movieView.onInsertIntoTable());
    QCOMPARE(dbModel.rowCount(), 2);

    // files not exist any more(if Fds not exist before, but now not exist) will be delete from this table,
    // files new added(if Fds not exist before) will be insert into this table
    // files renamed(if Fds unchange but name changed) will be updated into this table

    // recover tables
    QVERIFY(!dbModel.isDirty());

    // 2.1 表名和选中的路径不匹配, 不允许审计 `path1` does not match `tableName2` mount path, cannot audit
    QCOMPARE(dbModel.tableName(), tableName2);
    QVERIFY(!movieView.onAuditATable());

    // 后台改掉文件名称 Morata.mp4 -> Alvaro Morata.mp4
    QVERIFY(QDir(tDir).rename("path2/Morata.mp4", "path2/Alvaro Morata.mp4"));
    // 后台移除文件 Michael Fassbender.mp4
    QVERIFY(QFile::remove(tDir.itemPath("path2/Michael Fassbender.mp4")));
    // 后台增加文件 Kaka.mp4
    tDir.touch("path2/Kaka.mp4", "Contents in Kaka.mp4");

    // 3.1 更新1, 删除1, 增加1
    QVERIFY(movieView.onAuditATable());                        // path2 match tableName2
    QCOMPARE(dbModel.rowCount(), 2);                           // now it been recovered
    QSet<QString> expectNamesInDatabase{"Alvaro Morata.mp4", "Kaka.mp4"};  // no `Morata.mp4` anymore, no `Michael Fassbender.mp4` anymore
    QSet<QString> actualNamesInDatabase;
    QVERIFY(fdDb.QueryPK(tableName2, ENUM_2_STR(Name), actualNamesInDatabase));
    QCOMPARE(actualNamesInDatabase, expectNamesInDatabase);

    // 4.1 dirty cannot audit
    movieView.selectAll();
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, "Spanish/Brazil");
    QCOMPARE(movieView.onSetStudio(), 2);
    QCOMPARE(dbModel.record(0).value(MOVIE_TABLE::Studio).toString(), "Spanish/Brazil");
    QCOMPARE(dbModel.record(1).value(MOVIE_TABLE::Studio).toString(), "Spanish/Brazil");
    QVERIFY(dbModel.isDirty());
    QVERIFY(!movieView.onAuditATable());
  }

  void drop_a_table_ok() {
    QVERIFY(!QFile::exists(dbName));
    QWidget parent;
    MovieDBSearchToolBar dbToolBar{"MovieViewSearchToolBarTest", &parent};
    FdBasedDb fdDb{dbName, connName};
    FdBasedDbModel dbModel{&parent, fdDb.GetDb()};
    MovieDBView movieView{&dbModel, &dbToolBar, fdDb, &parent};

    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::question).stubs().will(returnValue(QMessageBox::StandardButton::Yes));
    UserInteractiveMock::InputDialog::getItem_set() = std::pair<bool, QString>(true, tableName2);
    QVERIFY(movieView.onCreateATable());
    QVERIFY(fdDb.IsTableExist(tableName2));
    MOCKER(QFileDialog::getExistingDirectory).stubs().will(returnValue(path2));
    QVERIFY(movieView.onInsertIntoTable());  // use `path2`
    QCOMPARE(dbModel.rowCount(), 2);

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
// todo: testcase too large. need extract
// sudo fdisk -l
// find in Device colums
// mkdir -p /mnt/DISKS/n1p2
// sudo mount /dev/nvme0n1p4 /mnt/DISKS/n1p2
// mkdir -p /mnt/DISKS/DD2_4T
// sudo mount /dev/nvme0n1p8 /mnt/DISKS/DD2_4T
// C:/home/DISKS/DD2_4T
#include "MovieDBViewTest.moc"
REGISTER_TEST(MovieDBViewTest, false)
