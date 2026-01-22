#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "DuplicateVideosFinder.h"
#include "EndToExposePrivateMember.h"
#include "DupVidsManager.h"
#include "VideoTestPrecoditionTools.h"
#include "DuplicateVideosFinderActions.h"
#include "ClipboardGuard.h"
#include "TDir.h"
#include <QDesktopServices>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

using namespace VideoTestPrecoditionTools;

class DuplicateVideosFinderTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tdir;
  QString aNewDupVidPrePath = tdir.path();
  SetDatabaseParmRetType dbNameSetResult = setDupVidDbAbsFilePath(aNewDupVidPrePath);
  SetDatabaseParmRetType connNameSetResult = setDupVidDbConnectionName("DUP_VID_FINDER_CONN_TEST", __LINE__);
  // must initialize databaseName and connectionName in front of instance DupVidsManager.
  // this testcase file share a same connection name
  DuplicateVideosFinder dvf;
 private slots:
  void initTestCase() {
    GlobalMockObject::reset();
    MOCKER(QDesktopServices::openUrl).stubs().will(returnValue(true));

    // precondition: drop
    QVERIFY(tdir.IsValid());
    QVERIFY2(dbNameSetResult.first, qPrintable(dbNameSetResult.second));
    QVERIFY2(connNameSetResult.first, qPrintable(connNameSetResult.second));

    QVERIFY(QFileInfo{VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH}.isDir());
    QVERIFY(QFileInfo{VideoTestPrecoditionTools::TS_FILE_MERGER_SAMPLE_PATH}.isDir());

    QVERIFY(dvf.m_aiTables != nullptr);
    dvf.m_aiTables->LoadAiMediaTableNames();  // update
    QVERIFY(dvf.m_aiTables->m_aiMediaTblModel != nullptr);
    QCOMPARE(dvf.m_aiTables->m_aiMediaTblModel->rowCount(), 0);
    QVERIFY(dvf.m_aiTables->onScanAPath(VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH));
    QCOMPARE(dvf.m_aiTables->m_aiMediaTblModel->rowCount(), 1);
    QVERIFY(dvf.m_aiTables->onScanAPath(VideoTestPrecoditionTools::TS_FILE_MERGER_SAMPLE_PATH));
    QCOMPARE(dvf.m_aiTables->m_aiMediaTblModel->rowCount(), 2);
  }

  void cleanupTestCase() {  //
    GlobalMockObject::verify();
  }

  void dup_vid_tables_drop_available() {
    // precondition:
    QVERIFY(dvf.m_aiTables->m_aiMediaTblModel->rowCount() >= 1);

    // cancel all selection to test initial state
    dvf.m_rightDetailsTbl->clearSelection();
    QCOMPARE(dvf.m_rightDetailsTbl->selectionModel()->hasSelection(), false);

    auto& inst = g_dupVidFinderAg();

    dvf.onDifferTypeChanged(inst.DIFFER_BY_SIZE);
    dvf.onDifferTypeChanged(inst.DIFFER_BY_DURATION);

    QVERIFY(dvf.durationDevLE != nullptr);
    dvf.durationDevLE->setText("-999");  // Negative number
    dvf.onChangeDurationDeviation();
    dvf.durationDevLE->setText("1000000");  // 1000s a large enough value => all videos will be grouped into one
    emit dvf.durationDevLE->returnPressed();

    QVERIFY(dvf.sizeDevLE != nullptr);
    dvf.sizeDevLE->setText("-999");  // Negative number
    dvf.onChangeSizeDeviation();
    dvf.sizeDevLE->setText("10000000");  // 10*1024*1024=10MB a large enough value =>  all videos will be grouped into one
    emit dvf.sizeDevLE->returnPressed();

    QVERIFY(dvf.tableNameFilterLE != nullptr);
    dvf.tableNameFilterLE->setText("[a-z0-9_].*");
    emit dvf.tableNameFilterLE->returnPressed();
    const QRegularExpression acutalReg = dvf.m_aiTables->m_sortProxy->filterRegularExpression();
    QCOMPARE(acutalReg.pattern(), "[a-z0-9_].*");

    // clear all analyse
    dvf.onAnalyzeDuplicatesInVideosList({});
    QCOMPARE(dvf.m_leftGrpsTbl->selectionModel()->hasSelection(), false);

    {  // in m_rightDetailsTbl, copy filename for everything tool ok, open file
      // 0. ignored, m_rightDetailsTbl has no records at all
      QKeyEvent copyEvent{QEvent::KeyPress, Qt::Key::Key_Insert, Qt::KeyboardModifier::ControlModifier, QString(), false, 1};
      dvf.keyPressEvent(&copyEvent);
      QCOMPARE(copyEvent.isAccepted(), false);

      QKeyEvent doubleClickedEvent{QEvent::KeyPress, Qt::Key::Key_Enter, Qt::KeyboardModifier::NoModifier, QString(), false, 1};
      dvf.keyPressEvent(&doubleClickedEvent);
      QCOMPARE(doubleClickedEvent.isAccepted(), false);

      // 1. let analyze some table
      dvf.m_aiTables->selectAll();
      QVERIFY(dvf.m_aiTables->onAnalyzeTheseSelectedTables() > 0);

      // 2. let m_leftGrpsTbl has 1 selection
      QVERIFY(dvf.m_leftGrpsTbl->m_leftGrpModel->rowCount() >= 1);
      dvf.m_leftGrpsTbl->selectRow(0);
      emit dvf.m_leftGrpsTbl->leftSelectionChanged(0);

      // 3. let m_rightDetailsTbl has 1 selection
      QVERIFY(dvf.m_rightDetailsTbl->m_detailsModel->rowCount() >= 1);
      dvf.m_rightDetailsTbl->selectRow(0);
      {
        ClipboardGuard clipboardGuard;
        const QString beforeText{clipboardGuard.getCurText()};
        dvf.keyPressEvent(&copyEvent);
        QCOMPARE(copyEvent.isAccepted(), true);
        const QString afterText{clipboardGuard.getCurText()};
        if (afterText != beforeText) {
          LOG_W("clipboard not steady[%s][%s]", qPrintable(afterText), qPrintable(beforeText));
        }
      }
      {
        dvf.show();
        QTRY_VERIFY(QTest::qWaitForWindowActive(&dvf));

        dvf.m_rightDetailsTbl->setFocus();
        dvf.keyPressEvent(&doubleClickedEvent);
        QCOMPARE(doubleClickedEvent.isAccepted(), true);
      }
    }
    dvf.hide();
    dvf.close();
  }
};

#include "DuplicateVideosFinderTest.moc"
REGISTER_TEST(DuplicateVideosFinderTest, false)
