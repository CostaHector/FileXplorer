#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ItemView.h"
#include "EndToExposePrivateMember.h"

#include "Configuration.h"
#include "TDir.h"
#include "FileTool.h"
#include "FileToolMock.h"
#include "BatchRenameBy.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class ItemViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
 private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
    QList<FsNodeEntry> nodes{
        {"emptyImageFolder", true, ""},
        {"3ImagesFolder/CR7 0.jpg", false, ""},
        {"3ImagesFolder/CR7 1.png", false, ""},
        {"3ImagesFolder/Kaka 2.webp", false, ""},
    };
    QCOMPARE(mTDir.createEntries(nodes), 4);
  }

  void cleanupTestCase() {}

  void init() {  //
    GlobalMockObject::reset();
  }

  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void default_constructor() {
    Configuration().clear();

    ImgsModel mImgModel{"ImgsListView"};
    QCOMPARE(mImgModel.setDirPath(mTDir.itemPath("emptyImageFolder"), {}, false), 0);
    QVERIFY(mImgModel.index(0, 0).data(Qt::DisplayRole).isNull());

    QCOMPARE(mImgModel.setDirPath(mTDir.itemPath("3ImagesFolder"), {}, false), 3);
    ItemView mImgTv{"ItemViewImageTest"};
    mImgTv.InitListView();
    QCOMPARE(mImgTv.viewMode(), QListView::ViewMode::IconMode); // default ok
    QCOMPARE(mImgTv.flow(), QListView::Flow::LeftToRight);
    QCOMPARE(mImgTv.isWrapping(), true);
    mImgTv.setViewMode(QListView::ViewMode::IconMode);

    QCOMPARE(mImgTv.SetCurrentModel(nullptr), false);
    QCOMPARE(mImgTv.onCellDoubleClicked({}), false);  // will not crash
    QCOMPARE(mImgTv.onPlayCurrentIndex(), false);     // will not crash
    QCOMPARE(mImgTv.onRecycleSelections(), false);    // will not crash

    QCOMPARE(mImgTv.SetCurrentModel(&mImgModel), true);
    mImgTv.setCurrentIndex(QModelIndex());
    QVERIFY(mImgTv._PLAY_ITEM != nullptr);
    emit mImgTv._PLAY_ITEM->triggered();
    QVERIFY(!mImgTv.onPlayCurrentIndex());

    mImgTv.clearSelection();
    QVERIFY(mImgTv._RECYCLE_ITEM != nullptr);
    emit mImgTv._RECYCLE_ITEM->triggered();
    QVERIFY(!mImgTv.onRecycleSelections());

    mImgTv.setCurrentIndex(mImgModel.index(0, 0));
    QVERIFY(!mImgTv.onCellDoubleClicked(mImgModel.index(0, 0)));
  }

  void subscribe_ok() {
    MOCKER(FileTool::OpenLocalFile).stubs().will(invoke(FileToolMock::invokeOpenLocalFile));

    ImgsModel mImgModel{"ImgsListView"};
    QCOMPARE(mImgModel.setDirPath(mTDir.itemPath("3ImagesFolder"), {"*CR7*"}, true), 2);

    ItemView mImgTv{"ItemViewImageTest"};
    mImgTv.InitListView();
    mImgTv.setViewMode(QListView::ViewMode::IconMode);
    mImgTv.SetCurrentModel(&mImgModel);

    QCOMPARE(mImgModel.index(0, 0).data(Qt::ToolTipRole).toString(), mTDir.itemPath("3ImagesFolder/CR7 0.jpg"));
    QCOMPARE(mImgModel.index(1, 0).data(Qt::ToolTipRole).toString(), mTDir.itemPath("3ImagesFolder/CR7 1.png"));
    mImgTv.setCurrentIndex(mImgModel.index(1, 0));
    QCOMPARE(mImgTv.onCellDoubleClicked(mImgModel.index(1, 0)), true);

    // recycle folder, index valid but file not exists
    QCOMPARE(mImgTv.onRecycleSelections(), true);
    QCOMPARE(mTDir.exists("3ImagesFolder/CR7 1.png"), false);
    QCOMPARE(mImgModel.index(1, 0).isValid(), true);
    QCOMPARE(mImgTv.onCellDoubleClicked(mImgModel.index(1, 0)), false);
  }

  void onRenameSelectedItems_ok() {
    MOCKER(BatchRenameBy::NumerizerQueryAndConfirm).stubs().will(returnValue(BatchRenameBy::RnmResult::ALL_SUCCEED));

    ImgsModel mImgModel{"ImgsModel"};
    mImgModel.UpdateData({
        "selected/Chris.jpg",
        "selected/Chris 1.jpg",
        "selected/Chris 2.jpg",
        "selected/Chris Evans.jpg",
        "selected/Kaka.jpg",
    });
    QCOMPARE(mImgModel.rowCount(), 5);

    ItemView mImgTv{"ImagesItemView"};
    mImgTv.InitListView();
    mImgTv.setViewMode(QListView::ViewMode::IconMode);
    mImgTv.SetCurrentModel(&mImgModel);

    mImgTv.clearSelection();
    QCOMPARE(mImgTv.onRenameSelectedItems(), 0);

    mImgTv.selectionModel()->select(mImgModel.index(0), QItemSelectionModel::SelectionFlag::Select);
    mImgTv.selectionModel()->select(mImgModel.index(4), QItemSelectionModel::SelectionFlag::Select);

    QCOMPARE(mImgTv.onRenameSelectedItems(), 2);
    QCOMPARE(mImgModel.rowCount(), 5 - 2);
  }
};

#include "ItemViewTest.moc"
REGISTER_TEST(ItemViewTest, false)
