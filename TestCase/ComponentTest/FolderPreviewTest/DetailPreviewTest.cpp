#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "DetailPreview.h"
#include "EndToExposePrivateMember.h"

class DetailPreviewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void UpdateWhenSelectAFile_ok() {  //
    DetailPreview dp;
    QCOMPARE(dp.GetFullScreenableWidget(), dp.mBasicVideoView);
    QCOMPARE(dp.needSaveStateWhenClose(), true);

    dp.UpdateWhenSelectAFile("path/to/not an video.txt");
    QCOMPARE(dp.mBasicVideoView->isHidden(), true);

    dp.UpdateWhenSelectAFile("path/to/an video.mp4");
    QCOMPARE(dp.mBasicVideoView->isHidden(), false);

    // 进入全屏
    QVERIFY(dp.mBasicVideoView != nullptr);
    QVERIFY(dp.mBasicVideoView->mFullScreenCallback != nullptr);
    emit dp.mBasicVideoView->reqFullscreenModeChange(true);
    QCOMPARE(dp.needSaveStateWhenClose(), false);
  }
};

#include "DetailPreviewTest.moc"
REGISTER_TEST(DetailPreviewTest, false)
