#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "TDir.h"
#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "AdvanceSearchTableView.h"
#include "EndToExposePrivateMember.h"
#include "FileBasicOperationsActions.h"
#include "MimeDataHelper.h"
#include <QDir>

class AdvanceSearchTableViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_1() {
    QList<FsNodeEntry> nodes{
        {"files 1.txt", false, "henry cavill superman"},
        {"files 2.txt", true, "Henry Cavill SUPERMAN"},
        {"folder", true, ""},
    };
    TDir tdir;
    QCOMPARE(tdir.createEntries(nodes), nodes.size());
    QString rootPath = tdir.path();
    QVERIFY(QFileInfo(rootPath).isDir());

    AdvanceSearchModel sourceModel;
    sourceModel.initFilter(QDir::Filter::Dirs | QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);
    sourceModel.initIteratorFlag(QDirIterator::IteratorFlag::Subdirectories);
    sourceModel.setRootPath(rootPath);

    SearchProxyModel searchProxyModel;
    searchProxyModel.initSearchMode(SearchTools::SearchModeE::NORMAL);
    searchProxyModel.initNameFilterDisables(false);  // bGrayorHide: hide
    searchProxyModel.initFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
    searchProxyModel.initFileContentsCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);

    AdvanceSearchTableView advSearch{&sourceModel, &searchProxyModel};
    sourceModel.setRootPath(rootPath);

    // 1. filter ok
    searchProxyModel.startFilterWhenTextChanged("files", "HENRY");
    QCOMPARE(searchProxyModel.rowCount(), 2);

    { // 2. copy selections ok(first half procedure)
      advSearch.selectAll();
      QModelIndexList rowsSelected2 = advSearch.selectionModel()->selectedRows(0); // the name row
      QCOMPARE(rowsSelected2.size(), 2);
      auto mimeDataMember = MimeDataHelper::GetMimeDataMemberFromSearchModel(sourceModel, searchProxyModel, rowsSelected2);
      QCOMPARE(mimeDataMember.texts.size(), 2);
      QCOMPARE(mimeDataMember.urls.size(), 2);
      QCOMPARE(mimeDataMember.srcIndexes.size(), 2);

      const QString expectPaths = mimeDataMember.texts.join('\n');
      QCOMPARE(expectPaths.count("files 1.txt"), 1);
      QCOMPARE(expectPaths.count("files 2.txt"), 1);
      QCOMPARE(expectPaths.count("\n"), 1);

      QVERIFY(MimeDataHelper::FillCutCopySomething<AdvanceSearchModel>(sourceModel, mimeDataMember.srcIndexes, Qt::DropAction::MoveAction));
      SelectionsRangeHelper::ROW_RANGES_LST rowRangeList = sourceModel.mCutIndexes.GetTopBottomRange();
      QCOMPARE(rowRangeList.isEmpty(), false);
    }

    // 3. remove selections ok
    advSearch.clearSelection();
    QSignalSpy removeSignalSpy(g_fileBasicOperationsActions().MOVE_TO_TRASHBIN,
                               &QAction::triggered);
    // nothing selected
    QKeyEvent deleteEvent{QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier, QString(), false, 1};
    advSearch.keyPressEvent(&deleteEvent);
    QCOMPARE(removeSignalSpy.count(), 0);

    // all 3 item(s) selected
    advSearch.selectAll();
    advSearch.keyPressEvent(&deleteEvent);
    QCOMPARE(removeSignalSpy.count(), 1);
  }
};

#include "AdvanceSearchTableViewTest.moc"
REGISTER_TEST(AdvanceSearchTableViewTest, false)
