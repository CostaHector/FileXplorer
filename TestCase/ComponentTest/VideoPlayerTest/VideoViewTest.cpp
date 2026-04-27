#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "VideoView.h"
#include "EndToExposePrivateMember.h"

#include <QSignalSpy>

class VideoViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void registerFullScreenToggleCallback_ok() {
    VideoView videoView{false, nullptr};
    QVERIFY(videoView.GetBasicVideoView() != nullptr);
    QCOMPARE(videoView.GetBasicVideoView(), videoView.mBasicVideoView);

    BasicVideoView& basicViewView = *videoView.mBasicVideoView;
    QVERIFY(basicViewView.GetVideoWidget() != nullptr);
    QCOMPARE(basicViewView.GetVideoWidget(), basicViewView.mVideoWidget);
    InteractiveVideoWidget* videoWid = basicViewView.mVideoWidget;
    QVERIFY(videoWid != nullptr);

    QVERIFY(videoView.mFullScreenCallback == nullptr);

    QSignalSpy reqFullscreenModeChangeSpy(&basicViewView, &BasicVideoView::reqFullscreenModeChange);
    emit videoWid->fullScreenModeToggled(true);
    emit videoWid->fullScreenModeToggled(false);
    basicViewView.emitFullScreenModeReq(true);
    basicViewView.emitFullScreenModeReq(false);
    QCOMPARE(reqFullscreenModeChangeSpy.count(), 4);
    reqFullscreenModeChangeSpy.clear();

    int intoFullScreenCnt{0}, exitFullScreenCnt{0};
    const auto fullScreenCallback = [&intoFullScreenCnt, &exitFullScreenCnt](bool bFullScreen) {
      bFullScreen ? ++intoFullScreenCnt : ++exitFullScreenCnt;
      return true;
    };

    QCOMPARE(basicViewView.registerFullScreenToggleCallback(nullptr), false);  // not crash down
    QVERIFY(basicViewView.mFullScreenCallback == nullptr);
    QCOMPARE(basicViewView.registerFullScreenToggleCallback(fullScreenCallback), true);
    QVERIFY(basicViewView.mFullScreenCallback != nullptr);
    QCOMPARE(basicViewView.registerFullScreenToggleCallback(fullScreenCallback), false);
    QVERIFY(basicViewView.mFullScreenCallback != nullptr);

    emit videoWid->fullScreenModeToggled(true);
    QCOMPARE(intoFullScreenCnt, 1);
    QCOMPARE(exitFullScreenCnt, 0);
    emit videoWid->fullScreenModeToggled(false);
    QCOMPARE(intoFullScreenCnt, 1);
    QCOMPARE(exitFullScreenCnt, 1);
    basicViewView.emitFullScreenModeReq(true);
    QCOMPARE(intoFullScreenCnt, 2);
    QCOMPARE(exitFullScreenCnt, 1);
    basicViewView.emitFullScreenModeReq(false);
    QCOMPARE(intoFullScreenCnt, 2);
    QCOMPARE(exitFullScreenCnt, 2);
    QCOMPARE(reqFullscreenModeChangeSpy.count(), 4);
    reqFullscreenModeChangeSpy.clear();
  }

  void basicMode_ok() {
    VideoView videoView{false, nullptr};
    QCOMPARE(videoView.registerFullScreenToggleCallback(nullptr), false); // not crash down

    InteractiveVideoWidget* videoWidget = videoView.mBasicVideoView->mVideoWidget;
    QVERIFY(videoWidget != nullptr);

    QCOMPARE(videoWidget->mBasicModeAct->isChecked(), false);
    QCOMPARE(videoView.mBasicVideoView->mFunctionCtrlBar->isHidden(), false);  // 不受影响
    QCOMPARE(videoView.mExtendedFunctionCtrlBar->isHidden(), false);
    QCOMPARE(videoView.mVideoList->isHidden(), false);

    videoWidget->mBasicModeAct->toggle();

    QCOMPARE(videoWidget->mBasicModeAct->isChecked(), true);                   // 基础模式
    QCOMPARE(videoView.mBasicVideoView->mFunctionCtrlBar->isHidden(), false);  // 不受影响
    QCOMPARE(videoView.mExtendedFunctionCtrlBar->isHidden(), true);
    QCOMPARE(videoView.mVideoList->isHidden(), true);

    const QSize szhint = videoView.sizeHint();
    QCOMPARE(szhint.isValid(), true);

    QVERIFY(videoView.GetBasicVideoView() != nullptr);
  }

  void toolBarVisibility_ok() {
    VideoView videoView{false, nullptr};
    InteractiveVideoWidget* videoWidget = videoView.mBasicVideoView->mVideoWidget;
    QVERIFY(videoWidget != nullptr);

    QCOMPARE(videoWidget->mHideToolBarAct->isChecked(), false);
    QCOMPARE(videoView.mBasicVideoView->mFunctionCtrlBar->isHidden(), false);
    QCOMPARE(videoView.mExtendedFunctionCtrlBar->isHidden(), false);

    videoWidget->mHideToolBarAct->toggle();

    QCOMPARE(videoWidget->mHideToolBarAct->isChecked(), true);
    QCOMPARE(videoView.mBasicVideoView->mFunctionCtrlBar->isHidden(), true);
    QCOMPARE(videoView.mExtendedFunctionCtrlBar->isHidden(), true);
  }

  void listVisibility_ok() {
    VideoView videoView{false, nullptr};
    InteractiveVideoWidget* videoWidget = videoView.mBasicVideoView->mVideoWidget;
    QVERIFY(videoWidget != nullptr);

    QCOMPARE(videoWidget->mShowVideoList->isChecked(), true);
    QCOMPARE(videoView.mVideoList->isHidden(), false);

    videoWidget->mShowVideoList->toggle();

    QCOMPARE(videoWidget->mShowVideoList->isChecked(), false);
    QCOMPARE(videoView.mVideoList->isHidden(), true);
  }

  void playAPath_ok() {
    VideoView videoView{false, nullptr};
    QCOMPARE(videoView.GetCurrentPlayingMediaPath(), "");
    QSignalSpy reqPlayMediaSpy{videoView.mVideoList, &VideoTableView::reqPlayMedia};

    QCOMPARE(videoView.PlayAPath("", true), 0);
    QCOMPARE(reqPlayMediaSpy.count(), 0);
    QCOMPARE(videoView.GetCurrentPlayingMediaPath(), "");

    QCOMPARE(videoView.PlayAPath("Path/to/InexistFolder", true), 0);
    QCOMPARE(reqPlayMediaSpy.count(), 0);
    QCOMPARE(videoView.GetCurrentPlayingMediaPath(), "");

    videoView.onStopPlaying();
  }

  void playVideos_ok() {
    VideoView videoView{false, nullptr};
    QCOMPARE(videoView.GetCurrentPlayingMediaPath(), "");
    QVERIFY(videoView.mVideoList != nullptr);
    QVERIFY(videoView.mVideoList->mProxyModel != nullptr);
    videoView.mVideoList->mProxyModel->sort(0, Qt::AscendingOrder);
    QSignalSpy reqPlayMediaSpy{videoView.mVideoList, &VideoTableView::reqPlayMedia};

    // empty path, 0 file
    QCOMPARE(videoView.PlayVideos("", {}, true), 0);
    QCOMPARE(reqPlayMediaSpy.count(), 0);
    QCOMPARE(videoView.GetCurrentPlayingMediaPath(), "");

    // 1 file exist
    QCOMPARE(videoView.PlayVideos("", {__FILE__}, true), 1);
    QCOMPARE(reqPlayMediaSpy.count(), 1);
    QCOMPARE(reqPlayMediaSpy.takeLast(), (QVariantList{__FILE__, true}));
    QCOMPARE(videoView.GetCurrentPlayingMediaPath(), __FILE__);

    // 2 file but not exist
    const int fileFindCnt2 = videoView.PlayVideos("Path/to/InexistFolder",  //
                                                  {"Path/to/InexistFolder/file1.mp4", "Path/to/InexistFolder/file2.mp4"}, true);

    QCOMPARE(fileFindCnt2, 2);
    QCOMPARE(reqPlayMediaSpy.count(), 1);
    QCOMPARE(reqPlayMediaSpy.takeLast(), (QVariantList{"Path/to/InexistFolder/file1.mp4", true}));
    QCOMPARE(videoView.GetCurrentPlayingMediaPath(), "");

    videoView.onStopPlaying();
  }
};

#include "VideoViewTest.moc"
REGISTER_TEST(VideoViewTest, false)
