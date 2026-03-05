#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "LeftVideoGroupsTable.h"
#include "EndToExposePrivateMember.h"
#include "ModelTestHelper.h"

class LeftVideoGroupsTableTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void selection_toggle_non_and_valid_ok() {
    Configuration().setValue(MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.name, 2048);  // 2kB

    LeftVideoGroupsTable lvgt;
    QVERIFY(lvgt.m_leftGrpModel != nullptr);
    QVERIFY(lvgt.m_leftSortProxy != nullptr);
    QCOMPARE(lvgt.m_leftGrpModel->m_currentDiffer, DuplicateVideoDetectionCriteria::DEFAULT_VD_CRITERIA_E);
    DupVidMetaInfoList duplicaVideosListNeedAnalyze = {
        //
        {"v1.mp4", 2048, 999, 0, "/v1.mp4",  "hash1"},   //
        {"v2.mp4", 2048, 999, 0, "/v2.mp4",  "hash2"},   //
        {"v3.mp4", 2048, 999, 0, "/v3.mp4",  "hash3"},   //
        {"v4.mp4", 10240, 999, 0, "/v3.mp4", "hash3"},  //
        {"v5.mp4", 10240, 999, 0, "/v3.mp4", "hash3"},  //
    };
    QVERIFY(lvgt.m_leftGrpModel != nullptr);
    // [2048,2048,2048] round 2048=1, [10240,10240] round 2048=5
    QCOMPARE(lvgt.onDuplicateVideosListChanged(duplicaVideosListNeedAnalyze), 2);
    QCOMPARE(lvgt.m_leftGrpModel->rowCount(), 2);

    QSignalSpy spy(&lvgt, &LeftVideoGroupsTable::leftSelectionChanged);

    QItemSelectionModel* selectionModel = lvgt.selectionModel();
    QVERIFY(selectionModel != nullptr);

    QModelIndex sourceIndex0 = lvgt.m_leftGrpModel->index(0, 0);
    QModelIndex proxyIndex0 = lvgt.m_leftSortProxy->mapFromSource(sourceIndex0);
    selectionModel->setCurrentIndex(proxyIndex0, QItemSelectionModel::SelectionFlag::Select);
    selectionModel->select(proxyIndex0, QItemSelectionModel::Select);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments0 = spy.back();
    QCOMPARE(arguments0[0].toInt(), 0);  // row 0 selected

    QModelIndex sourceIndex1 = lvgt.m_leftGrpModel->index(1, 0);
    QModelIndex proxyIndex1 = lvgt.m_leftSortProxy->mapFromSource(sourceIndex1);
    selectionModel->setCurrentIndex(proxyIndex1, QItemSelectionModel::SelectionFlag::Select);
    selectionModel->select(proxyIndex1, QItemSelectionModel::Select);
    QCOMPARE(spy.count(), 2);
    QList<QVariant> arguments1 = spy.back();
    QCOMPARE(arguments1[0].toInt(), 1);  // row 1 selelcted

    selectionModel->clearSelection();
    QCOMPARE(spy.count(), 3);
    QList<QVariant> arguments2 = spy.back();
    QCOMPARE(arguments2[0].toInt(), INVALID_LEFT_SELECTED_ROW);  // non selected
  }

  void when_groupsCountChanged_emit_windowTitleChangedSignal_ok() {
    Configuration().setValue(MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.name, 2048);  // 2kB
    Configuration().setValue(MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.name, 2000);  // 2kB

    static auto isTitleMessageInExpect = [](const QString& title, const int batchCount, const QString& differBy) -> bool {
      return title.contains(QString("%1 batch(es)").arg(batchCount), Qt::CaseInsensitive)  //
             && title.contains(differBy, Qt::CaseInsensitive);                             //
    };

    LeftVideoGroupsTable lvgt;
    QVERIFY(lvgt.m_leftGrpModel != nullptr);
    QVERIFY(lvgt.m_leftSortProxy != nullptr);
    QCOMPARE(lvgt.m_leftGrpModel->m_currentDiffer, DuplicateVideoDetectionCriteria::DEFAULT_VD_CRITERIA_E);

    const QString defaultTitleMessage = lvgt.GetCurrentDupVideoGroupInfo();
    QVERIFY(isTitleMessageInExpect(defaultTitleMessage, 0, "SIZE"));

    DupVidMetaInfoList duplicaVideosListNeedAnalyze = {
        //
        {"v1.mp4", 2048, 10000, 0, "/v1.mp4", "hash1"},  //
        {"v2.mp4", 2048, 10000, 0, "/v2.mp4", "hash2"},  //
        {"v3.mp4", 2048, 2000, 0, "/v3.mp4",  "hash3"},   //
        {"v4.mp4", 10240, 2000, 0, "/v3.mp4", "hash3"},  //
        {"v5.mp4", 10240, 2000, 0, "/v3.mp4", "hash3"},  //
    };

    QSignalSpy winTitleSpy(&lvgt, &LeftVideoGroupsTable::windowTitleChanged);

    // 1. onDuplicateVideosListChanged changes windowTitle
    // [2048,2048,2048] round 2048=1, [10240,10240] round 2048=5
    QCOMPARE(lvgt.onDuplicateVideosListChanged(duplicaVideosListNeedAnalyze), 2);
    QCOMPARE(lvgt.m_leftGrpModel->rowCount(), 2);  // rowCount=2

    QCOMPARE(winTitleSpy.count(), 1);
    QList<QVariant> params1 = winTitleSpy.last();
    QCOMPARE(params1.size(), 1);
    const QString titleMessage1 = params1[0].toString();
    QVERIFY(isTitleMessageInExpect(titleMessage1, 2, "SIZE"));

    // 2. onDuplicateVideosListChanged changes windowTitle
    // [2048,2048,2048,10240,10240] round 20481 = 0
    lvgt.setDeviationSize(10240 * 2 + 1);
    QCOMPARE(lvgt.m_leftGrpModel->rowCount(), 1);  // rowCount=1
    QCOMPARE(winTitleSpy.count(), 2);
    QList<QVariant> params2 = winTitleSpy.last();
    QCOMPARE(params2.size(), 1);
    const QString titleMessage2 = params2[0].toString();
    QVERIFY(isTitleMessageInExpect(titleMessage2, 1, "SIZE"));

    // 3. setDifferType changes windowTitle
    lvgt.setDifferType(DuplicateVideoDetectionCriteria::DVCriteriaE::DURATION);
    // [2000,2000,2000] round 2000=1, [10000,10000] round 2000=5
    QCOMPARE(lvgt.m_leftGrpModel->rowCount(), 2);  // rowCount=2
    QCOMPARE(winTitleSpy.count(), 3);
    QList<QVariant> params3 = winTitleSpy.last();
    QCOMPARE(params3.size(), 1);
    const QString titleMessage3 = params3[0].toString();
    QVERIFY(isTitleMessageInExpect(titleMessage3, 2, "DURATION"));

    // 4. setDeviationDuration changes windowTitle
    // [2000,2000,2000,10000,10000] round 20001=0
    lvgt.setDeviationDuration(10000 * 2 + 1);
    QCOMPARE(lvgt.m_leftGrpModel->rowCount(), 1);  // rowCount=1
    QCOMPARE(winTitleSpy.count(), 4);
    QList<QVariant> params4 = winTitleSpy.last();
    QCOMPARE(params4.size(), 1);
    const QString titleMessage4 = params4[0].toString();
    QVERIFY(isTitleMessageInExpect(titleMessage4, 1, "DURATION"));
  }

  void sort_by_column_enabled_ok() {
    Configuration().setValue(MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.name, 2048);  // 2kB

    LeftVideoGroupsTable lvgt;
    QVERIFY(lvgt.m_leftGrpModel != nullptr);
    QVERIFY(lvgt.m_leftSortProxy != nullptr);
    QCOMPARE(lvgt.m_leftGrpModel->m_currentDiffer, DuplicateVideoDetectionCriteria::DEFAULT_VD_CRITERIA_E);
    DupVidMetaInfoList duplicaVideosListNeedAnalyze = {
        //
        {"v1.mp4", 2048, 999, 0, "/v1.mp4",  "hash1"},   //
        {"v2.mp4", 2048, 999, 0, "/v2.mp4",  "hash2"},   //
        {"v3.mp4", 2048, 999, 0, "/v3.mp4",  "hash3"},   //
        {"v4.mp4", 10240, 999, 0, "/v3.mp4", "hash3"},  //
        {"v5.mp4", 10240, 999, 0, "/v3.mp4", "hash3"},  //
    };
    QVERIFY(lvgt.m_leftGrpModel != nullptr);
    // [2048,2048,2048] round 2048=1, [10240,10240] round 2048=5
    QCOMPARE(lvgt.onDuplicateVideosListChanged(duplicaVideosListNeedAnalyze), 2);
    QCOMPARE(lvgt.m_leftGrpModel->rowCount(), 2);
    // 表格中的数据如下
    //        数量 文件大小
    // row 0: 3 2048
    // row 1: 2 10240

    QCOMPARE(lvgt.isSortingEnabled(), true);

    QHeaderView* header = lvgt.horizontalHeader();
    QVERIFY(header != nullptr);
    const QPoint column0HeaderPoint = HeaderRectHelper::GetHeaderRect(lvgt, 0, Qt::Orientation::Horizontal).center();
    const QPoint column1HeaderPoint = HeaderRectHelper::GetHeaderRect(lvgt, 1, Qt::Orientation::Horizontal).center();
    QCOMPARE(column0HeaderPoint.isNull(), false);
    QCOMPARE(column1HeaderPoint.isNull(), false);

    // 1. sort by first column: count
    lvgt.sortByColumn(0, Qt::AscendingOrder);
    QCOMPARE(lvgt.model()->index(0, 0).data().toInt(), 2);
    QCOMPARE(lvgt.model()->index(1, 0).data().toInt(), 3);
    lvgt.sortByColumn(0, Qt::DescendingOrder); // mouse click not work
    QTest::mouseClick(header, Qt::LeftButton, Qt::NoModifier, column0HeaderPoint, 20);
    QCOMPARE(lvgt.model()->index(0, 0).data().toInt(), 3);
    QCOMPARE(lvgt.model()->index(1, 0).data().toInt(), 2);

    // 2. sort by second column: size
    lvgt.sortByColumn(1, Qt::AscendingOrder);
    QCOMPARE(lvgt.model()->index(0, 1).data().toInt(), 2048);
    QCOMPARE(lvgt.model()->index(1, 1).data().toInt(), 10240);
    lvgt.sortByColumn(1, Qt::DescendingOrder); // mouse click not work
    QCOMPARE(lvgt.model()->index(0, 1).data().toInt(), 10240);
    QCOMPARE(lvgt.model()->index(1, 1).data().toInt(), 2048);
  }
};

#include "LeftVideoGroupsTableTest.moc"
REGISTER_TEST(LeftVideoGroupsTableTest, false)
