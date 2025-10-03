#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "TorrentsManagerWidget.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"

#include "TorrDBAction.h"
#include "PublicVariable.h"

class TorrentsManagerWidgetTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
 private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());  //
    QList<FsNodeEntry> nodes{
        {"Chris Evans.torrent", false, "contenst1"},      //
        {"Chris Hemsworth.torrent", false, "contenst2"},  //
        {"Chris Pine.torrent", false, "contenst3"},       //
        {"Morata.txt", false, "contenst4"},               //
    };
    QCOMPARE(tDir.createEntries(nodes), 4);

    Configuration().clear();
  }

  void cleanupTestCase() {
    Configuration().clear();  //
    TorrentsManagerWidgetMock::MockTorrDatabase() = std::pair<QString, QString>("", "");
    TorrentsManagerWidgetMock::MockDeleteTorrWhereClause() = "";
    TorrentsManagerWidgetMock::MockQryBeforeDropTable() = false;
  }

  void init_ok_actionEvent_filter_event_ok() {
    auto& torrInst = g_torrActions();

    TorrentsManagerWidgetMock::MockTorrDatabase() = std::pair<QString, QString>(tDir.itemPath("TorrDatabaseTest.db"), "TorrDatabaseTestConnName");

    TorrentsManagerWidget torrWid;
    QVERIFY(torrWid.mDb.IsValid());
    QVERIFY(torrWid.m_searchLE != nullptr);
    QVERIFY(torrWid.m_searchToobar != nullptr);
    QVERIFY(torrWid.m_torrentsListView != nullptr);
    QVERIFY(torrWid.m_torrentsDBModel != nullptr);
    QCOMPARE(torrWid.m_searchLE->text(), "Name like \"%\"");

    torrWid.onInitDataBase();
    {
      QVERIFY(!torrWid.mDb.IsTableExist(DB_TABLE::TORRENTS));  // no tables cannot insert into
      Configuration().setValue(MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.name, tDir.path());
      QVERIFY(!torrWid.onInsertIntoTable());
    }

    {
      torrWid.onInitATable();
      QVERIFY(torrWid.mDb.IsTableExist(DB_TABLE::TORRENTS));  // tables now ok
      QCOMPARE(torrWid.mDb.CountRow(DB_TABLE::TORRENTS), 0);
    }

    {  // onInsertIntoTable ok
      // inexist path
      Configuration().setValue(MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.name, "path/to/inexist_directory");
      QVERIFY(!torrWid.onInsertIntoTable());

      // exist path
      Configuration().setValue(MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.name, tDir.path());
      emit torrInst.INSERT_INTO_TABLE->triggered();
      QCOMPARE(torrWid.mDb.CountRow(DB_TABLE::TORRENTS), 3);  // 3 torrents in total
      QCOMPARE(torrWid.m_torrentsDBModel->rowCount(), 3);

      // the same exist path again
      Configuration().setValue(MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.name, tDir.path());
      QVERIFY(torrWid.onInsertIntoTable());
      QCOMPARE(torrWid.mDb.CountRow(DB_TABLE::TORRENTS), 3);  // 3 torrents unchange
      QCOMPARE(torrWid.m_torrentsDBModel->rowCount(), 3);
      QVERIFY(!torrWid.m_torrentsDBModel->isDirty());

      QList<FsNodeEntry> nodes{
          {"Michael Fassbender.torrent", false, "contents in michael"},
      };
      QCOMPARE(tDir.createEntries(nodes), 1);
      QVERIFY(torrWid.onInsertIntoTable());
      QCOMPARE(torrWid.mDb.CountRow(DB_TABLE::TORRENTS), 3 + 1);  // 3+1 torrents unchange
      QCOMPARE(torrWid.m_torrentsDBModel->rowCount(), 3 + 1);
      QVERIFY(!torrWid.m_torrentsDBModel->isDirty());
    }

    {  // filter ok
      torrWid.m_searchLE->setText(R"(`Name` LIKE "%Chris%")");
      emit torrWid.m_searchLE->returnPressed();
      QCOMPARE(torrWid.m_torrentsDBModel->rowCount(), 3);  // 3 torrents name contains "Chris"

      torrWid.m_searchLE->setText("");
      emit torrWid.m_searchLE->returnPressed();
      QCOMPARE(torrWid.m_torrentsDBModel->rowCount(), 3 + 1); // no filter
    }

    {                                                               // delete row by where clause ok
      TorrentsManagerWidgetMock::MockDeleteTorrWhereClause() = "";  // 0 deleted
      QVERIFY(!torrWid.onDeleteFromTable());                        // empty where clause. nothing delete
      QCOMPARE(torrWid.m_torrentsDBModel->rowCount(), 3 + 1);

      TorrentsManagerWidgetMock::MockDeleteTorrWhereClause() = R"(`Name` like "Michael%")";  // 1 deleted
      QVERIFY(torrWid.onDeleteFromTable());
      QCOMPARE(torrWid.m_torrentsDBModel->rowCount(), 3);
    }

    { // constructor will setTable and select if table already exist
      QVERIFY(torrWid.mDb.IsTableExist(DB_TABLE::TORRENTS));
      QCOMPARE(torrWid.mDb.CountRow(DB_TABLE::TORRENTS), 3);
      TorrentsManagerWidgetMock::MockTorrDatabase() = std::pair<QString, QString>(tDir.itemPath("TorrDatabaseTest.db"), "TorrDatabaseTestConnNameAnother");
      TorrentsManagerWidget aNewTorrWid;
      QVERIFY(aNewTorrWid.mDb.IsValid());
      QCOMPARE(aNewTorrWid.m_torrentsDBModel->rowCount(), 3);
    }

    {
      // drop table ok
      TorrentsManagerWidgetMock::MockQryBeforeDropTable() = false;  // user cancel
      QVERIFY(!torrWid.onDropATable());
      QVERIFY(torrWid.mDb.IsTableExist(DB_TABLE::TORRENTS));  // tables will exist

      TorrentsManagerWidgetMock::MockQryBeforeDropTable() = true;  // user cancel
      QVERIFY(torrWid.onDropATable());
      QVERIFY(!torrWid.mDb.IsTableExist(DB_TABLE::TORRENTS));  // tables now not exist
    }

    QVERIFY(torrWid.onSubmit());

    torrWid.showEvent(nullptr);
    QShowEvent defaultShowEvent;
    torrWid.showEvent(&defaultShowEvent);
    torrWid.close();
  }
};

#include "TorrentsManagerWidgetTest.moc"
REGISTER_TEST(TorrentsManagerWidgetTest, false)
