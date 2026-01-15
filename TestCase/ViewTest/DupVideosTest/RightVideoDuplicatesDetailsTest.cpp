#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "RightVideoDuplicatesDetails.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "ClipboardGuard.h"
#include "DuplicateVideosFinderActions.h"

class RightVideoDuplicatesDetailsTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  QDir mDir{tDir.path()};
 private slots:
  void initTestCase() {
    const QList<FsNodeEntry> nodes{
        //
        {"video 1.mp4", false, "0123456789"},    //
        {"video 1 ai.mp4", false, "012345678"},  //
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());
  }

  void initalized_and_no_exception_ok() {
    RightVideoDuplicatesDetails rvdd;
    QVERIFY(rvdd.m_detailsModel != nullptr);
    QVERIFY(rvdd.m_rightSortProxy != nullptr);
    QCOMPARE(rvdd.isSortingEnabled(), true);
    QCOMPARE(rvdd.selectionBehavior(), QAbstractItemView::SelectionBehavior::SelectRows);

    // should not crash down below
    QCOMPARE(rvdd.setSharedMember(nullptr, nullptr), false);                 // pass nullptr
    rvdd.onLeftVideoGroupsTableSelectionChanged(INVALID_LEFT_SELECTED_ROW);  // pass invalid row number

    QModelIndex invalidIndex;
    QCOMPARE(rvdd.on_cellDoubleClicked(invalidIndex), false);
    QCOMPARE(rvdd.on_effectiveNameCopiedForEverything(invalidIndex), false);

    // nothing selected
    QCOMPARE(rvdd.onRecycleSelection(), false);
  }

  void copy_to_clipboard_and_recycle_selection_ok() {
    GroupedDupVidList szGrp{
        DupVidMetaInfoList{
            // QString name; qint64 sz; int dur; qint64 modifiedDate; QString abspath; QString hash;
            {"video 1 ai.mp4", 10, 4000, 0, mDir.absoluteFilePath("video 1 ai.mp4"),    "h1"},      //
            {"video 1.mp4", 10, 4000, 0, mDir.absoluteFilePath("video 1.mp4"),          "h2"},            //
            {"inexist video 1.mp4", 10, 4000, 0, "inexist path already deleted before", "h3"},  // a remain
        },
    };
    GroupedDupVidList durGrp = szGrp;  // for simplicity. two of them are equal
    GroupedDupVidListArr groupedVidsLists{szGrp, durGrp};

    DuplicateVideoDetectionCriteria::DVCriteriaE differBy = DuplicateVideoDetectionCriteria::DVCriteriaE::SIZE;

    RightVideoDuplicatesDetails rvdd;
    QCOMPARE(rvdd.setSharedMember(&groupedVidsLists, &differBy), true);

    QCOMPARE(rvdd.m_detailsModel->rowCount(), 0);
    QCOMPARE(rvdd.onLeftVideoGroupsTableSelectionChanged(0), 3 - 0);
    QCOMPARE(rvdd.m_detailsModel->rowCount(), 3);

    QModelIndex sourceIndex0 = rvdd.m_detailsModel->index(0, 0);
    QModelIndex proxyIndex0 = rvdd.m_rightSortProxy->mapFromSource(sourceIndex0);

    QModelIndex fileNotExistSourceIndex2 = rvdd.m_detailsModel->index(2, 0);
    QModelIndex fileNotExistProxyIndex2 = rvdd.m_rightSortProxy->mapFromSource(fileNotExistSourceIndex2);

    // 1.0 copy to clipbord ok
    {
      ClipboardGuard systemClipboardGuard;
      QCOMPARE(rvdd.on_effectiveNameCopiedForEverything(proxyIndex0), true);
      auto* cb = QApplication::clipboard();
      QVERIFY(cb != nullptr);
      QCOMPARE(cb->text(), "video 1 ai mp4");  // only contains [0-9a-zA-Z_ ] char

      QCOMPARE(rvdd.on_effectiveNameCopiedForEverything(fileNotExistProxyIndex2), true);
#ifndef _WIN32
      QCOMPARE(cb->text(), "inexist video 1 mp4");  // clipboard is extremely unreliable in windows. only contains [0-9a-zA-Z_ ] char. clipboard is unreliable
#endif
    }

    // 2.0 double clicked ok
    {
      QCOMPARE(rvdd.on_cellDoubleClicked(proxyIndex0), true);
      QCOMPARE(rvdd.on_cellDoubleClicked(fileNotExistProxyIndex2), false);
    }

    // 2.0 recycle ok;
    {
      const QStringList beforeNames = tDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name);
      QCOMPARE(beforeNames, (QStringList{"video 1 ai.mp4", "video 1.mp4"}));  // ascii(' ') less than ascii('.')
      emit g_dupVidFinderAg().RECYCLE_SELECTIONS->triggered();
      const QStringList nothingSelectedRecyleNames = tDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name);
      QCOMPARE(nothingSelectedRecyleNames, beforeNames);

      rvdd.selectAll();
      emit g_dupVidFinderAg().RECYCLE_SELECTIONS->triggered();
      const QStringList afterSelectedAllRecyleNames = tDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name);
      QVERIFY(afterSelectedAllRecyleNames.isEmpty());
    }
  }
};

#include "RightVideoDuplicatesDetailsTest.moc"
REGISTER_TEST(RightVideoDuplicatesDetailsTest, false)
