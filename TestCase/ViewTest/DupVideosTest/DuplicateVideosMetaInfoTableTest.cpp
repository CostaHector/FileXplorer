#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "DuplicateVideosMetaInfoTable.h"
#include "EndToExposePrivateMember.h"
#include "DuplicateVideosFinderActions.h"
#include "VideoTestPrecoditionTools.h"
#include "PublicVariable.h"

class DuplicateVideosMetaInfoTableTest : public PlainTestSuite {
  Q_OBJECT
 public:
  VideoTestPrecoditionTools& tool{VideoTestPrecoditionTools::getInst()};
 private slots:
  void initTestCase() { QCOMPARE(DupVidsManager::DropDatabaseForTest(tool.DUP_VID_DB, false), true); }

  void cleanupTestCase() { DupVidsManager::DropDatabaseForTest(tool.DUP_VID_DB, false); }

  void dragMoveDropEvent_dropTable_ok() {
    // precondition
    DuplicateVideosMetaInfoTable dupTv;
    QVERIFY(dupTv.m_aiMediaTblModel != nullptr);
    dupTv.LoadAiMediaTableNames();
    QCOMPARE(dupTv.m_aiMediaTblModel->rowCount(), 0);

    const QPoint dragEnterPos{dupTv.geometry().center()};

    // without url event get rejected
    QMimeData emptyMimeData;
    emptyMimeData.setText("No urls");
    QDragEnterEvent ignoreDragEnter(dragEnterPos, Qt::DropAction::IgnoreAction, &emptyMimeData, Qt::LeftButton, Qt::NoModifier);
    QDragMoveEvent ignoreMoveEnter(dragEnterPos, Qt::DropAction::IgnoreAction, &emptyMimeData, Qt::LeftButton, Qt::KeyboardModifier::NoModifier);
    QDropEvent dropOnFileIgnoredEvent(dragEnterPos, Qt::DropAction::IgnoreAction, &emptyMimeData, Qt::LeftButton, Qt::KeyboardModifier::NoModifier);
    dupTv.dragEnterEvent(&ignoreDragEnter);
    QCOMPARE(ignoreDragEnter.isAccepted(), false);
    dupTv.dragMoveEvent(&ignoreMoveEnter);
    QCOMPARE(ignoreMoveEnter.isAccepted(), false);
    dupTv.dropEvent(&dropOnFileIgnoredEvent);
    QCOMPARE(dropOnFileIgnoredEvent.isAccepted(), false);
    QCOMPARE(dupTv.m_aiMediaTblModel->rowCount(), 0);

    // with url event get accept
    QMimeData urlsMimeData;
    urlsMimeData.setText("2 urls");
    QList<QUrl> urlsList{QUrl::fromLocalFile(tool.VID_DUR_GETTER_SAMPLE_PATH), QUrl::fromLocalFile(tool.TS_FILE_MERGER_SAMPLE_PATH)};
    urlsMimeData.setUrls(urlsList);
    QDragEnterEvent acceptDragEnter(dragEnterPos, Qt::DropAction::IgnoreAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier);
    QDragMoveEvent acceptMoveEnter(dragEnterPos, Qt::DropAction::IgnoreAction, &urlsMimeData, Qt::LeftButton, Qt::KeyboardModifier::NoModifier);
    QDropEvent dropOnFileAcceptEvent(dragEnterPos, Qt::DropAction::IgnoreAction, &urlsMimeData, Qt::LeftButton, Qt::KeyboardModifier::NoModifier);
    dupTv.dragEnterEvent(&acceptDragEnter);
    QCOMPARE(acceptDragEnter.isAccepted(), true);
    dupTv.dragMoveEvent(&acceptMoveEnter);
    QCOMPARE(acceptMoveEnter.isAccepted(), true);
    dupTv.dropEvent(&dropOnFileAcceptEvent);
    QCOMPARE(dropOnFileAcceptEvent.isAccepted(), true);
    QCOMPARE(dupTv.m_aiMediaTblModel->rowCount(), 2);
    const DupVidTableName2RecordCountList beforeData = dupTv.m_aiMediaTblModel->m_data;
    QCOMPARE(beforeData.size(), 2);

    // effect same with call onScanAPath twice
    auto& dupVidActsInst = g_dupVidFinderAg();

    dupTv.clearSelection();
    QCOMPARE(dupTv.selectionModel()->hasSelection(), false);
    emit dupVidActsInst.DROP_THESE_TABLES->triggered();     // nothing select, skip
    QCOMPARE(dupTv.m_aiMediaTblModel->m_data, beforeData);  // unchange

    dupTv.selectAll();
    QCOMPARE(dupTv.selectionModel()->hasSelection(), true);
    emit dupVidActsInst.DROP_THESE_TABLES->triggered();      // all select, nothing left
    QVERIFY(dupTv.m_aiMediaTblModel->m_data != beforeData);  // changed
    QVERIFY(dupTv.m_aiMediaTblModel->m_data.isEmpty());
  }

  // this test only prevent program crash down, the event processor relies on it's model
  void audit_forceReload_open_drop_ok() {
    auto& dupVidAgInst = g_dupVidFinderAg();
    // %1 videos in %2 tables
    static auto isTitleMessageInExpect = [](const QString& title, const int videosCount, const int tablesCount) -> bool {
      return title.contains(QString("%1 videos in %2 tables").arg(videosCount).arg(tablesCount), Qt::CaseInsensitive);  //
    };

    DuplicateVideosMetaInfoTable dupTv;
    QCOMPARE(dupTv.onScanAPath(__FILE__), false); // scan a file, skip
    QCOMPARE(dupTv.onScanAPath(tool.VID_DUR_GETTER_SAMPLE_PATH), true);
    QCOMPARE(dupTv.m_aiMediaTblModel->rowCount(), 1);

    // no selection below:
    QCOMPARE(dupTv.onAnalyzeTheseSelectedTables(), 0);
    QCOMPARE(dupTv.startAnalyzeNewTables({}), 0);  // no videos need analyse
    QCOMPARE(dupTv.onClearAnalyzeList(), 0);
    QCOMPARE(dupTv.onAuditSelectedTables(), true);
    QCOMPARE(dupTv.onForceReloadTables(), true);
    QCOMPARE(dupTv.onDropSelectedTables(), true);
    QCOMPARE(dupTv.onOpenTableAssociatedPath(QModelIndex{}), false);  // invalid index

    // has only 1 table, select all is also select one
    dupTv.selectRow(0);
    QDir dir(tool.VID_DUR_GETTER_SAMPLE_PATH, "", QDir::SortFlag::Name, QDir::Filter::Files);
    const QStringList vidNames = dir.entryList(TYPE_FILTER::AI_DUP_VIDEO_TYPE_SET);

    // 1. anaylze emit title changed signal
    QSignalSpy windowTitleChangedSpy(&dupTv, &DuplicateVideosMetaInfoTable::windowTitleChanged);
    QSignalSpy analzedTableFinishedSpy(&dupTv, &DuplicateVideosMetaInfoTable::analyzeTablesFinished);
    emit dupVidAgInst.ANALYSE_THESE_TABLES->triggered();  // trigger onAnalyzeTheseSelectedTables
    QCOMPARE(dupTv.mVideosListNeedAnalyse.size(), vidNames.size());
    QCOMPARE(analzedTableFinishedSpy.count(), 1);
    QCOMPARE(windowTitleChangedSpy.count(), 1);

    QList<QVariant> windowTitleChangedParam = windowTitleChangedSpy.back();
    QCOMPARE(windowTitleChangedParam.size(), 1);
    const QString titleInMessage = windowTitleChangedParam[0].toString();
    QVERIFY(isTitleMessageInExpect(titleInMessage, vidNames.size(), 1));

    //
    QModelIndex srcIndex = dupTv.m_aiMediaTblModel->index(0, 0);
    QModelIndex proIndex = dupTv.m_sortProxy->mapFromSource(srcIndex);
    QCOMPARE(dupTv.onAuditSelectedTables(), true);
    QCOMPARE(dupTv.onForceReloadTables(), true);
    QCOMPARE(dupTv.onOpenTableAssociatedPath(proIndex), true);
  }
};

#include "DuplicateVideosMetaInfoTableTest.moc"
REGISTER_TEST(DuplicateVideosMetaInfoTableTest, false)
