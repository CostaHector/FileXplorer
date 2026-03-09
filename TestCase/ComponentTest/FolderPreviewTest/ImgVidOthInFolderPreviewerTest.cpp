#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "ImgVidOthInFolderPreviewer.h"
#include "EndToExposePrivateMember.h"

class ImgVidOthInFolderPreviewerTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() { Configuration().clear(); }

  void cleanupTestCase() {  //
    Configuration().clear();
  }

  void init_state_ok() {
    ImgVidOthInFolderPreviewer ivoWid{"ImgVidOthInFolderPreviewer"};
    QCOMPARE(ivoWid.GetFullScreenableWidget(), ivoWid.mVidTv);
    QCOMPARE(ivoWid.needSaveStateWhenClose(), true);

    QCOMPARE(ivoWid._IMG_ACT->isChecked(), true);
    QCOMPARE(ivoWid._VID_ACT->isChecked(), true);
    QCOMPARE(ivoWid._OTH_ACT->isChecked(), true);
    QCOMPARE(ivoWid.indexOf(ivoWid.mImgTv), 0);
    QCOMPARE(ivoWid.indexOf(ivoWid.mVidTv), 1);
    QCOMPARE(ivoWid.indexOf(ivoWid.mOthTv), 2);
    QVERIFY(ivoWid.UpdateImgs("", {}));
    QVERIFY(ivoWid.UpdateVids("", {}));
    QVERIFY(ivoWid.UpdateOthers({}));

    // 进入全屏
    QVERIFY(ivoWid.mVidTv != nullptr);
    QVERIFY(ivoWid.mVidTv->mFullScreenCallback != nullptr);
    QVERIFY(ivoWid.mVidTv->mBasicVideoView != nullptr);
    emit ivoWid.mVidTv->mBasicVideoView->reqFullscreenModeChange(true);
    QCOMPARE(ivoWid.needSaveStateWhenClose(), false);
  }

  void all_action_unchecked() {
    Configuration().setValue(BrowserKey::FLOATING_IMAGE_VIEW_SHOW.name, false);
    Configuration().setValue(BrowserKey::FLOATING_VIDEO_VIEW_SHOW.name, false);
    Configuration().setValue(BrowserKey::FLOATING_OTHER_VIEW_SHOW.name, false);
    Configuration().setValue(BrowserKey::FLOATING_MEDIA_TYPE_SEQ.name, "210");
    ImgVidOthInFolderPreviewer ivoWid{"ImgVidOthInFolderPreviewer"};
    QCOMPARE(ivoWid._IMG_ACT->isChecked(), false);
    QCOMPARE(ivoWid._VID_ACT->isChecked(), false);
    QCOMPARE(ivoWid._OTH_ACT->isChecked(), false);
    QCOMPARE(ivoWid.indexOf(ivoWid.mImgTv), 2);
    QCOMPARE(ivoWid.indexOf(ivoWid.mVidTv), 1);
    QCOMPARE(ivoWid.indexOf(ivoWid.mOthTv), 0);

    QVERIFY(!ivoWid.UpdateImgs("", {}));
    QVERIFY(!ivoWid.UpdateVids("", {}));
    QVERIFY(!ivoWid.UpdateOthers({}));
  }
};

#include "ImgVidOthInFolderPreviewerTest.moc"
REGISTER_TEST(ImgVidOthInFolderPreviewerTest, false)
