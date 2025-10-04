#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ItemView.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"

class ItemViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:

  void default_constructor() {
    ImgsModel mImgModel;
    mImgModel.setDirPath("workPath", {"relative1.jpg", "relative2.jpg"}, true);
    QVERIFY(mImgModel.index(0, 0).data(Qt::DisplayRole).isNull());

    ItemView mImgTv{"ItemView Test"};
    mImgTv.setViewMode(QListView::ViewMode::IconMode);
    mImgTv.SetCurrentModel(&mImgModel);

    mImgTv.setCurrentIndex(QModelIndex());
    QVERIFY(mImgTv._PLAY_ITEM != nullptr);
    emit mImgTv._PLAY_ITEM->triggered();

    mImgTv.setCurrentIndex(mImgModel.index(0, 0));
    QVERIFY(!mImgTv.onCellDoubleClicked(mImgModel.index(0, 0)));
  }
};

#include "ItemViewTest.moc"
REGISTER_TEST(ItemViewTest, false)
