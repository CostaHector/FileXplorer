#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ItemView.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "FileTool.h"
#include "FileToolMock.h"

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
  void initupTestcase() {
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

  void default_constructor() {
    ImgsModel mImgModel{"ImgsListView"};
    QCOMPARE(mImgModel.setDirPath(mTDir.itemPath("emptyImageFolder"), {}, false), 0);
    QVERIFY(mImgModel.index(0, 0).data(Qt::DisplayRole).isNull());

    QCOMPARE(mImgModel.setDirPath(mTDir.itemPath("3ImagesFolder"), {}, false), 3);
    ItemView mImgTv{"ItemView Test"};
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

    ItemView mImgTv{"ItemView Test"};
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
};

#include "ItemViewTest.moc"
REGISTER_TEST(ItemViewTest, false)
