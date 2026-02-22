#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "FileToolMock.h"
#include "TDir.h"

#include "BeginToExposePrivateMember.h"
#include "RightVideoDuplicatesDetails.h"
#include "EndToExposePrivateMember.h"
#include "FileTool.h"
#include "DuplicateVideosFinderActions.h"
#include <QDesktopServices>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class RightVideoDuplicatesDetailsTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir tDir;
  QDir mDir{tDir.path()};
private slots:
  void initTestCase() {
    GlobalMockObject::reset();
    MOCKER(QDesktopServices::openUrl).stubs().will(invoke(FileToolMock::invokeOpenUrl));

    QVERIFY(tDir.IsValid());
    const QList<FsNodeEntry> nodes{
        //
        {"video 1.mp4", false, "0123456789"},   //
        {"video 1 ai.mp4", false, "012345678"}, //
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());
  }
  void cleanupTestCase() { //
    GlobalMockObject::verify();
  }

  void default_construct_ok() {
    RightVideoDuplicatesDetails rvdd;
    QVERIFY(rvdd.m_detailsModel != nullptr);
    QVERIFY(rvdd.m_rightSortProxy != nullptr);
    QCOMPARE(rvdd.isSortingEnabled(), true);
    QCOMPARE(rvdd.selectionBehavior(), QAbstractItemView::SelectionBehavior::SelectRows);

    // should not crash down below
    QCOMPARE(rvdd.setSharedMember(nullptr, nullptr), false);                // pass nullptr
    rvdd.onLeftVideoGroupsTableSelectionChanged(INVALID_LEFT_SELECTED_ROW); // pass invalid row number

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
        {"video 1 ai.mp4", 10, 4000, 0, mDir.absoluteFilePath("video 1 ai.mp4"), "h1"},    //
        {"video 1.mp4", 10, 4000, 0, mDir.absoluteFilePath("video 1.mp4"), "h2"},          //
        {"inexist video 1.mp4", 10, 4000, 0, "inexist path already deleted before", "h3"}, // a remain
        },
    };
    GroupedDupVidList durGrp = szGrp; // for simplicity. two of them are equal
    GroupedDupVidListArr groupedVidsLists{szGrp, durGrp};

    DuplicateVideoDetectionCriteria::DVCriteriaE differBy = DuplicateVideoDetectionCriteria::DVCriteriaE::SIZE;

    RightVideoDuplicatesDetails rvdd;
    QCOMPARE(rvdd.setSharedMember(&groupedVidsLists, &differBy), true);

    QCOMPARE(rvdd.m_detailsModel->rowCount(), 0);
    QCOMPARE(rvdd.onLeftVideoGroupsTableSelectionChanged(0), 3 - 0);
    QCOMPARE(szGrp.size(), 1);    // groups count=1
    QCOMPARE(szGrp[0].size(), 3); // item(s) int group[0]=3
    QCOMPARE(rvdd.m_detailsModel->rowCount(), 3);

    QModelIndex sourceIndex0 = rvdd.m_detailsModel->index(0, 0);
    QModelIndex proxyIndex0 = rvdd.m_rightSortProxy->mapFromSource(sourceIndex0);

    QModelIndex fileNotExistSourceIndex2 = rvdd.m_detailsModel->index(2, 0);
    QModelIndex fileNotExistProxyIndex2 = rvdd.m_rightSortProxy->mapFromSource(fileNotExistSourceIndex2);

    {
      // 1.0 copy to clipbord ok
      // only contains [0-9a-zA-Z_ ] char
      MOCKER(FileTool::CopyTextToSystemClipboard) //
          .expects(exactly(1))                    //
          .with(eq(QString{"video 1 ai mp4"}))    //
          .will(returnValue(true))
          .id("index0_Copy");
      MOCKER(FileTool::CopyTextToSystemClipboard) //
          .expects(exactly(1))                    //
          .with(eq(QString{"inexist video 1 mp4"}))    //
          .after("index0_Copy")
          .will(returnValue(true));
      // "video 1 ai mp4"
      QCOMPARE(rvdd.on_effectiveNameCopiedForEverything(proxyIndex0), true);
      // "inexist video 1 mp4"
      QCOMPARE(rvdd.on_effectiveNameCopiedForEverything(fileNotExistProxyIndex2), true);
    }

    {
      // 2.0 double clicked ok
      QCOMPARE(rvdd.on_cellDoubleClicked(proxyIndex0), true);
      QCOMPARE(rvdd.on_cellDoubleClicked(fileNotExistProxyIndex2), false);
    }

    {
      // 3.0 recycle when nothing selected
      QVERIFY(!rvdd.selectionModel()->hasSelection());
      const QStringList beforeNames = tDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name);
      QCOMPARE(beforeNames, (QStringList{"video 1 ai.mp4", "video 1.mp4"})); // ascii(' ') less than ascii('.')
      emit g_dupVidFinderAg().RECYCLE_SELECTIONS->triggered();
      const QStringList nothingSelectedRecyleNames = tDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name);
      QCOMPARE(nothingSelectedRecyleNames, beforeNames);

      // 3.1 recycle when all rows selected
      rvdd.selectAll();
      emit g_dupVidFinderAg().RECYCLE_SELECTIONS->triggered();
      const QStringList afterSelectedAllRecyleNames = tDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name);
      QVERIFY(afterSelectedAllRecyleNames.isEmpty());
    }
  }
};

#include "RightVideoDuplicatesDetailsTest.moc"
REGISTER_TEST(RightVideoDuplicatesDetailsTest, false)
