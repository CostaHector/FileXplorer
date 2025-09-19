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

    searchProxyModel.startFilterWhenTextChanged("files", "HENRY");
    QCOMPARE(searchProxyModel.rowCount(), 2);

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
