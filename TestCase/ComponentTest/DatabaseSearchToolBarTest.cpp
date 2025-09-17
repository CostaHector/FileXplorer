#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "DatabaseSearchToolBar.h"
#include "EndToExposePrivateMember.h"


class DatabaseSearchToolBarTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void initTestCase() {
    Configuration().clear();
  }

  void cleanupTestCase() { Configuration().clear(); }

  void MovieDBSearchToolBar_emitsignal_ok() {
    QWidget parent;
    MovieDBSearchToolBar mdbSearchBar{"MovieSearchToolBarTest", &parent};

    QVERIFY(mdbSearchBar.m_whereCB != nullptr);
    QVERIFY(mdbSearchBar.m_whereCB->count() >= 2); // at least 2 where clause

    auto* lineEdit = mdbSearchBar.m_whereCB->lineEdit();
    QVERIFY(lineEdit != nullptr);

    QSignalSpy whereClauseChangedSpy(&mdbSearchBar, &MovieDBSearchToolBar::whereClauseChanged);
    lineEdit->setText("New where Clause return pressed");
    emit lineEdit->returnPressed();
    QCOMPARE(whereClauseChangedSpy.count(), 1);
    QList<QVariant> whereClauseParams = whereClauseChangedSpy.back();
    QCOMPARE(whereClauseParams.size(), 1);
    QCOMPARE(whereClauseParams[0].toString(), "New where Clause return pressed");

    auto* tableCB = mdbSearchBar.m_tablesCB;
    QVERIFY(tableCB != nullptr);
    QCOMPARE(tableCB->isEditable(), false); // cannot edit by user directly
    QCOMPARE(tableCB->count(), 0); // no candidates. because InitTables not called until castView get Initialized
    QCOMPARE(mdbSearchBar.AskUserDropWhichTable(), ""); // no drop table

    QSignalSpy movieTableChangedSpy(&mdbSearchBar, &MovieDBSearchToolBar::movieTableChanged);
    tableCB->AddItem("Disk_GUID", "Directory Path");
    tableCB->setCurrentIndex(0);
    emit tableCB->currentTextChanged("Disk_GUID|Directory Path");
    QList<QVariant> movieTableParams = movieTableChangedSpy.back();
    QCOMPARE(movieTableParams.size(), 1);
    QCOMPARE(movieTableParams[0].toString(), "Disk_GUID|Directory Path");

    QCOMPARE(tableCB->currentText(), "Disk_GUID|Directory Path");
    QCOMPARE(mdbSearchBar.AskUserDropWhichTable(), "Disk_GUID"); // drop current text table by default
  }

  void CastDatabaseSearchToolBar_emitSignal_ok() {
    QWidget parent;
    CastDatabaseSearchToolBar mCastSearchBar{"MovieSearchToolBarTest", &parent};

    QVERIFY(mCastSearchBar.m_whereCB != nullptr);
    QVERIFY(mCastSearchBar.m_whereCB->count() >= 2); // at least 2 where clause

    auto* lineEdit = mCastSearchBar.m_whereCB->lineEdit();
    QVERIFY(lineEdit != nullptr);

    QSignalSpy whereClauseChangedSpy(&mCastSearchBar, &CastDatabaseSearchToolBar::whereClauseChanged);
    lineEdit->setText("New where Cast Clause return pressed");
    emit lineEdit->returnPressed();
    QCOMPARE(whereClauseChangedSpy.count(), 1);
    QList<QVariant> whereClauseParams = whereClauseChangedSpy.back();
    QCOMPARE(whereClauseParams.size(), 1);
    QCOMPARE(whereClauseParams[0].toString(), "New where Cast Clause return pressed");
  }
};

#include "DatabaseSearchToolBarTest.moc"
REGISTER_TEST(DatabaseSearchToolBarTest, false)
