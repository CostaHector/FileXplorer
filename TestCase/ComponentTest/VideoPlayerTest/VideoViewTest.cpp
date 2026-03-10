#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "VideoView.h"
#include "EndToExposePrivateMember.h"

class VideoViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {  //
    // Configuration().clear();
  }

  void cleanupTestCase() {  //
    // Configuration().clear();
  }

  void basicMode_ok() {
    VideoView videoView{false, nullptr};
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

    videoView.StopPlay();
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

    videoView.StopPlay();
  }
};

#include "VideoViewTest.moc"
REGISTER_TEST(VideoViewTest, false)
