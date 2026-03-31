#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"

#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "DatabaseSearchToolBar.h"
#include "EndToExposePrivateMember.h"
#include "MountHelper.h"
#include "MountPathTableNameMapperMock.h"
#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class DatabaseSearchToolBarTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {
    GlobalMockObject::reset();
    using namespace MountPathTableNameMapper;
    using namespace MountPathTableNameMapperMock;
    MOCKER(toMountPath).stubs().will(invoke(invokeToMountPath));
    MOCKER(toTableName).stubs().will(invoke(invokeToTableName));

    Configuration().clear();
    MovieDBSearchToolBarMock::QryDropWhichTableMock() = std::pair<bool, QString>(false, "");
  }

  void cleanupTestCase() {
    GlobalMockObject::verify();

    Configuration().clear();
    MovieDBSearchToolBarMock::QryDropWhichTableMock() = std::pair<bool, QString>(false, "");
  }

  void MovieDBSearchToolBar_emitsignal_ok() {
    Configuration().setValue(MemoryKey::VIDS_LAST_TABLE_NAME.name, "InexistTableName");

    QWidget parent;
    MovieDBSearchToolBar mdbSearchBar{"MovieSearchToolBarTest", &parent};
    QVERIFY(mdbSearchBar.m_whereCB != nullptr);
    QVERIFY(mdbSearchBar.m_whereCB->count() >= 2);  // at least 2 where clause
    auto* lineEdit = mdbSearchBar.m_whereCB->lineEdit();
    QVERIFY(lineEdit != nullptr);
    mdbSearchBar.InitCurrentIndex();
    QVERIFY(mdbSearchBar.GetCurrentTableName() != "InexistTableName");

    QSignalSpy whereClauseChangedSpy(&mdbSearchBar, &MovieDBSearchToolBar::whereClauseChanged);
    lineEdit->setText("New where Clause return pressed");
    emit lineEdit->returnPressed();
    QCOMPARE(whereClauseChangedSpy.count(), 1);
    QCOMPARE(whereClauseChangedSpy.takeLast(), (QVariantList{"New where Clause return pressed"}));

    auto* tableCB = mdbSearchBar.m_tablesCB;
    QVERIFY(tableCB != nullptr);
    QCOMPARE(tableCB->isEditable(), false);              // cannot edit by user directly
    QCOMPARE(tableCB->count(), 0);                       // no candidates. because InitTables not called until castView get Initialized
    QCOMPARE(mdbSearchBar.AskUserDropWhichTable(), "");  // no drop table

    mdbSearchBar.AddATable("Disk_GUID");  // in linux rootpath=""
    tableCB->setCurrentIndex(0);
    QCOMPARE(mdbSearchBar.m_tablesCB->count(), 1);

    QSignalSpy movieTableChangedSpy(&mdbSearchBar, &MovieDBSearchToolBar::movieTableChanged);
    const QString fullPath = tableCB->currentText();  // guid|rootpath
    emit tableCB->currentTextChanged(fullPath);
    QCOMPARE(movieTableChangedSpy.count(), 1);
    QCOMPARE(movieTableChangedSpy.takeLast(), (QVariantList{fullPath}));

    QCOMPARE(tableCB->currentText(), fullPath);

    MovieDBSearchToolBarMock::QryDropWhichTableMock() = std::pair<bool, QString>(false, "Disk_GUID");
    QCOMPARE(mdbSearchBar.AskUserDropWhichTable(), "");  // drop current text table by default

    MovieDBSearchToolBarMock::QryDropWhichTableMock() = std::pair<bool, QString>(true, "Disk_GUID");
    QCOMPARE(mdbSearchBar.AskUserDropWhichTable(), "Disk_GUID");  // drop current text table by default

    QStringList validTableNames{"_mnt_DISKS_DD2", "C__home_DISKS_DD2"};
    mdbSearchBar.InitTables(validTableNames);
    QCOMPARE(mdbSearchBar.m_tablesCB->count(), validTableNames.size());  // the former 1 table name in combobox will be cleared
    mdbSearchBar.m_tablesCB->setCurrentText("_mnt_DISKS_DD2");
    QCOMPARE(mdbSearchBar.GetMovieTableMountPath(), "/mnt/DISKS/DD2");
    mdbSearchBar.m_tablesCB->setCurrentText("C__home_DISKS_DD2");
    QCOMPARE(mdbSearchBar.GetMovieTableMountPath(), "C:/home/DISKS/DD2");

    Configuration().setValue(MemoryKey::VIDS_LAST_TABLE_NAME.name, "_mnt_DISKS_DD2");
    mdbSearchBar.InitCurrentIndex();
    QCOMPARE(mdbSearchBar.GetCurrentTableName(), "_mnt_DISKS_DD2");
  }

  void CastDatabaseSearchToolBar_emitSignal_ok() {
    QWidget parent;
    CastDatabaseSearchToolBar mCastSearchBar{"MovieSearchToolBarTest", &parent};    
    QVERIFY(mCastSearchBar.m_whereCB != nullptr);
    QVERIFY(mCastSearchBar.m_whereCB->count() >= 2);  // at least 2 where clause
    mCastSearchBar.onGetFocus();

    QLineEdit* lineEdit = mCastSearchBar.m_whereCB->lineEdit();
    QVERIFY(lineEdit != nullptr);

    QSignalSpy whereClauseChangedSpy(&mCastSearchBar, &CastDatabaseSearchToolBar::whereClauseChanged);
    mCastSearchBar.SetWhereClause("New where Cast Clause return pressed");
    QCOMPARE(lineEdit->text(), "New where Cast Clause return pressed");
    emit lineEdit->returnPressed();
    QCOMPARE(whereClauseChangedSpy.count(), 1);
    QCOMPARE(whereClauseChangedSpy.takeLast(), (QVariantList{"New where Cast Clause return pressed"}));

    mCastSearchBar.onQuickWhereClause();
    QCOMPARE(whereClauseChangedSpy.count(), 1);
    whereClauseChangedSpy.takeLast();
    QVERIFY(mCastSearchBar.m_quickWhereClause != nullptr);

    mCastSearchBar.m_quickWhereClause->m_whereLineEdit->setText("Chris Evans");
    mCastSearchBar.onQuickWhereClause();
    QCOMPARE(whereClauseChangedSpy.count(), 1);
    QCOMPARE(lineEdit->text(), "Chris Evans");
    QCOMPARE(whereClauseChangedSpy.takeLast(), (QVariantList{"Chris Evans"}));
  }
};

#include "DatabaseSearchToolBarTest.moc"
REGISTER_TEST(DatabaseSearchToolBarTest, false)
