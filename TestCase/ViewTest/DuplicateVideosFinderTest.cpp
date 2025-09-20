#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "DuplicateVideosFinder.h"
#include "EndToExposePrivateMember.h"
#include "DupVidsManager.h"
#include <QDir>

class DuplicateVideosFinderTest : public PlainTestSuite {
  Q_OBJECT
 public:
  const QString VID_DUR_GETTER_SAMPLE_PATH = TESTCASE_ROOT_PATH "/test/TestEnv_VideosDurationGetter";
  const QString TS_FILE_MERGER_SAMPLE_PATH = TESTCASE_ROOT_PATH "/test/TestEnv_TSFilesMerger";
  DuplicateVideosFinder dvf;
  const QString DUP_VID_DB{DupVidsManager::GetAiDupVidDbPath()};
  const QString DUP_VID_CONN = DupVidsManager::CONNECTION_NAME;
 private slots:
  void initTestCase() {
    // precondition: db should not exists
    QCOMPARE(DupVidsManager::DropDatabaseForTest(DUP_VID_DB, false), true);

    QVERIFY(QFileInfo{VID_DUR_GETTER_SAMPLE_PATH}.isDir());
    QVERIFY(QFileInfo{TS_FILE_MERGER_SAMPLE_PATH}.isDir());

    QVERIFY(dvf.m_aiTables != nullptr);
    dvf.m_aiTables->LoadAiMediaTableNames(); // update
    QVERIFY(dvf.m_aiTables->m_aiMediaTblModel != nullptr);
    QCOMPARE(dvf.m_aiTables->m_aiMediaTblModel->rowCount(), 0);
    QVERIFY(dvf.m_aiTables->onScanAPath(VID_DUR_GETTER_SAMPLE_PATH));
    QCOMPARE(dvf.m_aiTables->m_aiMediaTblModel->rowCount(), 1);
    QVERIFY(dvf.m_aiTables->onScanAPath(TS_FILE_MERGER_SAMPLE_PATH));
    QCOMPARE(dvf.m_aiTables->m_aiMediaTblModel->rowCount(), 2);
  }

  void cleanupTestCase() {
    DupVidsManager::DropDatabaseForTest(DUP_VID_DB, false);
  }
};

#include "DuplicateVideosFinderTest.moc"
REGISTER_TEST(DuplicateVideosFinderTest, false)
