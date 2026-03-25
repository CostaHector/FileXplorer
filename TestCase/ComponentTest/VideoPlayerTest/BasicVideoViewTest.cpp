#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "BasicVideoView.h"
#include "EndToExposePrivateMember.h"
#include "RateHelper.h"
#include "FileTool.h"
#include "VideoTestPrecoditionTools.h"

#include <QInputDialog>
#include <QSplitter>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class BasicVideoViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
  const QString mExistVidPath{VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH + QString("/Big Buck Bunny SampleVideo_360x240_1mb 13s.mp4")};
 private slots:
  void initTestCase() {  //
    QVERIFY(QFile::exists(mExistVidPath));
  }

  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void registerFullScreenToggleCallback_ok() {
    BasicVideoView basicViewView{false, nullptr};
    QVERIFY(basicViewView.GetVideoWidget() != nullptr);
    QCOMPARE(basicViewView.GetVideoWidget(), basicViewView.mVideoWidget);
    InteractiveVideoWidget* videoWid = basicViewView.mVideoWidget;
    QVERIFY(videoWid != nullptr);

    QVERIFY(basicViewView.mFullScreenCallback == nullptr);

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

  void playAVideo_trigger_disabled_ok() {
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_PLAYBACK_TRIGGER_MODE.name, (int)VideoPlayTool::PlaybackTriggerMode::DISABLED);
    const QString existVideoPath{__FILE__};
    MOCKER(BasicVideoView::SetMediaCore).expects(exactly(1)).will(returnValue(true));
    MOCKER(BasicVideoView::PlayCore).expects(exactly(1)).will(returnValue(true));
    MOCKER(FileTool::OpenLocalFileUsingDesktopService).expects(exactly(2)).with(eq(existVideoPath)).will(returnValue(true));

    BasicVideoView basicVideoView{true, nullptr};
    QVERIFY(!basicVideoView.bPauseButtonCenterInit);
    // 文件不存在
    QVERIFY(!basicVideoView.PlayAVideo("path/to/InexistsMediaFile.mp4", true));
    QVERIFY(basicVideoView.bPauseButtonCenterInit);
    QCOMPARE(basicVideoView.GetCurrentPlayingMediaPath(), "");

    // 禁用触发: 非强制不可触发(暂停状态) setMedia:0 play:0
    QVERIFY(basicVideoView.PlayAVideo(existVideoPath, false));
    QCOMPARE(basicVideoView.mVideoWidget->mPauseAct->isChecked(), true);
    QCOMPARE(basicVideoView.GetCurrentPlayingMediaPath(), existVideoPath);
    basicVideoView.mVideoWidget->mOpenInSystemApplication->trigger();  // time: 1

    // 禁用触发: 强制播放(非暂停状态) setMedia:1 play:1
    QVERIFY(basicVideoView.PlayAVideo(existVideoPath, true));
    QCOMPARE(basicVideoView.mVideoWidget->mPauseAct->isChecked(), false);
    QCOMPARE(basicVideoView.GetCurrentPlayingMediaPath(), existVideoPath);
    basicVideoView.reqPlayInSystemApplication();  // time: 2

    // 停止播放 mStopAct -> onStopPlaying 清理设置的media, 解除占用, 同时设置暂停按钮选中
    emit basicVideoView.mVideoWidget->mStopAct->triggered();
    QCOMPARE(basicVideoView.mVideoWidget->mPauseAct->isChecked(), true);
  }

  void onStopPlaying_release_file_ok() {
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_PLAYBACK_TRIGGER_MODE.name, (int)VideoPlayTool::PlaybackTriggerMode::DISABLED);

    BasicVideoView basicVideoView{true, nullptr};
    QCOMPARE(basicVideoView.mIsMediaCleared, false);
    basicVideoView.PlayAVideo(mExistVidPath, true);
    QCOMPARE(basicVideoView.GetCurrentPlayingMediaPath(), mExistVidPath);

    QCOMPARE(basicVideoView.mVideoWidget->mPauseAct->isChecked(), false);
    QCOMPARE(basicVideoView.mPlayer->media().isNull(), false);

    emit basicVideoView.mVideoWidget->mStopAct->triggered();
    QCOMPARE(basicVideoView.mVideoWidget->mPauseAct->isChecked(), true);
    QCOMPARE(basicVideoView.mPlayer->mediaStatus(), QMediaPlayer::MediaStatus::NoMedia);
    QCOMPARE(basicVideoView.mIsMediaCleared, true);

    basicVideoView.mVideoWidget->mPauseAct->toggle();
    QCOMPARE(basicVideoView.mVideoWidget->mPauseAct->isChecked(), false);
    QVERIFY(basicVideoView.mPlayer->mediaStatus() != QMediaPlayer::MediaStatus::NoMedia);
    QCOMPARE(basicVideoView.mIsMediaCleared, false);
  }

  void playAVideo_trigger_manual_ok() {
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_PLAYBACK_TRIGGER_MODE.name, (int)VideoPlayTool::PlaybackTriggerMode::MANUAL);

    MOCKER(BasicVideoView::SetMediaCore).expects(exactly(2)).will(returnValue(true));
    MOCKER(BasicVideoView::PlayCore).expects(exactly(2)).will(returnValue(true));

    BasicVideoView basicVideoView{false, nullptr};
    QVERIFY(!basicVideoView.bPauseButtonCenterInit);
    // 文件不存在
    QVERIFY(!basicVideoView.PlayAVideo("path/to/InexistsMediaFile.mp4", true));
    QVERIFY(basicVideoView.bPauseButtonCenterInit);
    QCOMPARE(basicVideoView.GetCurrentPlayingMediaPath(), "");

    // 手动触发: 非强制, setMedia:1, PlayCore:0
    QVERIFY(basicVideoView.PlayAVideo(__FILE__, false));
    QCOMPARE(basicVideoView.mVideoWidget->mPauseAct->isChecked(), true);
    QCOMPARE(basicVideoView.GetCurrentPlayingMediaPath(), __FILE__);
    // 手动触发 mPauseAct -> onPauseActionToggled -> PlayCore:1
    basicVideoView.mVideoWidget->mPauseAct->toggle();
    QCOMPARE(basicVideoView.mVideoWidget->mPauseAct->isChecked(), false);

    // 禁用触发: 强制可播放 setMedia:1, PlayCore:1
    QVERIFY(basicVideoView.PlayAVideo(__FILE__, true));
    QCOMPARE(basicVideoView.mVideoWidget->mPauseAct->isChecked(), false);
  }

  void emitFullScreenModeReq_ok() {
    BasicVideoView basicVideoView{true, nullptr};
    QCOMPARE(basicVideoView.isVideoFullScreen(), false);

    QSignalSpy layoutVisibilityChangedSpy{basicVideoView.mVideoWidget, &InteractiveVideoWidget::layoutVisibilityChanged};

    // 全屏
    QSignalSpy reqFullscreenModeChangeSpy{&basicVideoView, &BasicVideoView::reqFullscreenModeChange};
    basicVideoView.mVideoWidget->mFullScreenAct->toggle();
    QCOMPARE(reqFullscreenModeChangeSpy.count(), 1);
    QCOMPARE(reqFullscreenModeChangeSpy.takeLast(), (QVariantList{true}));
    QCOMPARE(layoutVisibilityChangedSpy.count(), 0);  // 超时才会发送

    // 超时后隐藏
    basicVideoView.mVideoWidget->onLongTimeNoEventHappen();
    QCOMPARE(layoutVisibilityChangedSpy.count(), 1);
    layoutVisibilityChangedSpy.takeLast();

    // 退出全屏
    basicVideoView.mVideoWidget->mFullScreenAct->toggle();
    QCOMPARE(reqFullscreenModeChangeSpy.count(), 1);
    QCOMPARE(reqFullscreenModeChangeSpy.takeLast(), (QVariantList{false}));
    QCOMPARE(layoutVisibilityChangedSpy.count(), 1);  // 立刻发送
    layoutVisibilityChangedSpy.takeLast();
  }

  void not_crash_ok() {
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_VOLUME.name, 99);
    BasicVideoView basicVideoView{false, nullptr};
    QCOMPARE(BasicVideoView::SetPositionCore(nullptr, 77), false);
    QCOMPARE(BasicVideoView::SetPositionCore(basicVideoView.mPlayer, 77), true);

    QCOMPARE(BasicVideoView::GetPositionCore(nullptr), 0);
    QCOMPARE(BasicVideoView::GetPositionCore(basicVideoView.mPlayer), 77);

    QCOMPARE(BasicVideoView::SetMediaCore(nullptr, __FILE__), false);
    QCOMPARE(BasicVideoView::SetMediaCore(&basicVideoView, __FILE__), true);

    QCOMPARE(BasicVideoView::PlayCore(nullptr), false);
    QCOMPARE(BasicVideoView::PlayCore(basicVideoView.mPlayer), true);

    QCOMPARE(basicVideoView.onError(QMediaPlayer::Error::NoError), QMediaPlayer::Error::NoError);
    QCOMPARE(basicVideoView.onAudioAvailableChanged(true), 98);  // floor(99*99/100)=floor(9801)=98

    basicVideoView.onStateChanged(QMediaPlayer::State::PausedState);
    QCOMPARE(basicVideoView.mPauseShieldButton->isHidden(), false);

    QCOMPARE(basicVideoView.GetCurrentPlayingMediaPath(), "");
    QCOMPARE(basicVideoView.adjustRateCurrentVideo(5), false);                 // curMedia empty
    QCOMPARE(basicVideoView.adjustRateCurrentVideo(0), true);                  // delta = 0
    QCOMPARE(basicVideoView.adjustRateAllVideoSameLevelAsCurrentVideo(0), 0);  // delta = 0

    // PauseBtn在VideoWidget上层
    const QObjectList& children = basicVideoView.children();
    const int levelOfVideoWidget = children.indexOf(basicVideoView.mVideoWidget);
    const int levelOfPauseBtnWidget = children.indexOf(basicVideoView.mPauseShieldButton);
    QVERIFY(levelOfVideoWidget != -1);
    QVERIFY(levelOfPauseBtnWidget != -1);
    QVERIFY(levelOfPauseBtnWidget > levelOfVideoWidget);
  }

  void mProgressSliderUpdateTimer_ok() {
    BasicVideoView basicVideoView{false, nullptr};
    QCOMPARE(basicVideoView.mProgressSliderUpdateTimer.isActive(), false);
    emit basicVideoView.mPlayer->durationChanged(100 * 1000);
    QCOMPARE(BasicVideoView::SetPositionCore(basicVideoView.mPlayer, 77 * 1000), true);
    QCOMPARE(basicVideoView.onUpdateProgressSliderPosition(), true);
    QCOMPARE(basicVideoView.mProgressSlider->value(), 77 * 1000);
    QCOMPARE(basicVideoView.mCurrentTimeLabel->text(), "00:01:17");
    QCOMPARE(basicVideoView.onUpdateProgressSliderPosition(), false);  // unchange
  }

  void position_changed_previous_10s_ok() {
    MOCKER(BasicVideoView::GetPositionCore).stubs().will(returnValue((qint64)100 * 1000));
    MOCKER(BasicVideoView::SetPositionCore).expects(exactly(1)).with(any(), (100 - 10) * 1000).will(returnValue(true));

    BasicVideoView basicVideoView{false, nullptr};
    InteractiveVideoWidget* interWid{basicVideoView.mVideoWidget};
    emit interWid->mSeekBackwardAct->triggered();
  }

  void position_changed_next_10s_ok() {
    MOCKER(BasicVideoView::GetPositionCore).stubs().will(returnValue((qint64)100 * 1000));
    MOCKER(BasicVideoView::SetPositionCore).expects(exactly(1)).with(any(), (100 + 10) * 1000).will(returnValue(true));

    BasicVideoView basicVideoView{false, nullptr};
    InteractiveVideoWidget* interWid{basicVideoView.mVideoWidget};
    emit interWid->mSeekForwardAct->triggered();
  }

  void durationChanged_ok() {
    BasicVideoView basicVideoView{false, nullptr};
    emit basicVideoView.mPlayer->durationChanged(100 * 1000);
    QCOMPARE(basicVideoView.mDurationLabel->text(), "00:01:40");
    QCOMPARE(basicVideoView.mProgressSlider->minimum(), 0);
    QCOMPARE(basicVideoView.mProgressSlider->maximum(), 100 * 1000);
  }

  void rateCurrentVideo_ok() {
    BasicVideoView basicVideoView{false, nullptr};
    QCOMPARE(basicVideoView.GetCurrentPlayingMediaPath(), "");
    QCOMPARE(basicVideoView.rateCurrentVideo(10), false);  // no media path

    basicVideoView.PlayAVideo(__FILE__, false);
    QCOMPARE(basicVideoView.GetCurrentPlayingMediaPath(), __FILE__);

    // rateCurrentVideo: assume the 1st time rate failed, second time succeed.
    MOCKER(RateHelper::RateMovie).expects(exactly(3)).will(returnValue(false)).then(returnValue(true));

    QCOMPARE(basicVideoView.rateCurrentVideo(10), false);  // 1st
    QCOMPARE(basicVideoView.rateCurrentVideo(10), true);   // 2nd

    RateActions* rateActions = basicVideoView.mVideoWidget->GetRateActions();
    QVERIFY(rateActions != nullptr);
    emit rateActions->RateMovieReq(9);  // 3rd

    // adjustRateCurrentVideo assume the 1st time rate failed, second time succeed.
    MOCKER(RateHelper::AdjustRateMovie).expects(exactly(3)).will(returnValue(false)).then(returnValue(true));
    QCOMPARE(basicVideoView.adjustRateCurrentVideo(0), true);   // delta=0, skipped
    QCOMPARE(basicVideoView.adjustRateCurrentVideo(3), false);  // 1st
    QCOMPARE(basicVideoView.adjustRateCurrentVideo(4), true);   // 2nd
    emit rateActions->AdjustRateMovieReq(5);                    // 3rd
  }

  void rateAllVideoSameLevelAsCurrentVideo_ok() {
    BasicVideoView basicVideoView{false, nullptr};
    QCOMPARE(basicVideoView.GetCurrentPlayingMediaPath(), "");

    QCOMPARE(basicVideoView.rateAllVideoSameLevelAsCurrentVideo(false), 0);    // no path specified
    QCOMPARE(basicVideoView.adjustRateAllVideoSameLevelAsCurrentVideo(5), 0);  // no path specified

    const bool bForceRecusive{false};
    basicVideoView.PlayAVideo(__FILE__, false);
    QCOMPARE(basicVideoView.GetCurrentPlayingMediaPath(), __FILE__);

    const QString expectRatePathRecursive{QFileInfo{__FILE__}.absolutePath()};
    const int rateSucceedFilesCnt{3};
    const int expectScore{9};
    bool expectOk{true};
    MOCKER(QInputDialog::getInt)
        .expects(exactly(2))                                                                                   //
        .with(any(), any(), any(), any(), any(), any(), any(), outBoundP(&expectOk, sizeof(expectOk)), any())  //
        .will(returnValue(expectScore));                                                                       //
    MOCKER(RateHelper::RateMovieRecursively)
        .expects(exactly(2))                                                     //
        .with(eq(expectRatePathRecursive), eq(expectScore), eq(bForceRecusive))  //
        .will(returnValue(rateSucceedFilesCnt));                                 // 3 file

    QCOMPARE(basicVideoView.rateAllVideoSameLevelAsCurrentVideo(bForceRecusive), rateSucceedFilesCnt);  // 1st
    RateActions* rateActions = basicVideoView.mVideoWidget->GetRateActions();
    QVERIFY(rateActions != nullptr);
    emit rateActions->RateMovieRecursivelyReq(false);  // 2nd

    const int expectDelta{9};
    MOCKER(RateHelper::AdjustRateMovieRecursively)
        .expects(exactly(2))                                                                               //
        .with(eq(expectRatePathRecursive), eq(expectDelta))                                                //
        .will(returnValue(rateSucceedFilesCnt));                                                           // 3 file
    QCOMPARE(basicVideoView.adjustRateAllVideoSameLevelAsCurrentVideo(expectDelta), rateSucceedFilesCnt);  // 1st
    emit rateActions->AdjustRateMovieRecursivelyReq(expectDelta);                                          // 2nd
  }

  void onMediaStatusChanged_ok() {
    // EndOfMedia -> play Next
    BasicVideoView basicVideoView{false, nullptr};
    basicVideoView.mVideoWidget->mPlaybackMode_CurrentItemOnce->setChecked(true);

    QSignalSpy reqPlayNextOneMediaSpy{&basicVideoView, &BasicVideoView::reqPlayNextOneMedia};

    // CurrentItemOnce, 不发送
    QCOMPARE(basicVideoView.mVideoWidget->GetPlaybackMode(), QMediaPlaylist::PlaybackMode::CurrentItemOnce);
    basicVideoView.onMediaStatusChanged(QMediaPlayer::MediaStatus::EndOfMedia);
    QCOMPARE(reqPlayNextOneMediaSpy.count(), 0);

    // 发送
    basicVideoView.mVideoWidget->mPlaybackMode_Loop->setChecked(true);
    basicVideoView.onMediaStatusChanged(QMediaPlayer::MediaStatus::EndOfMedia);
    QCOMPARE(reqPlayNextOneMediaSpy.count(), 1);
    reqPlayNextOneMediaSpy.takeLast();

    basicVideoView.mVideoWidget->mPlaybackMode_Sequential->setChecked(true);
    basicVideoView.onMediaStatusChanged(QMediaPlayer::MediaStatus::EndOfMedia);
    QCOMPARE(reqPlayNextOneMediaSpy.count(), 1);
    reqPlayNextOneMediaSpy.takeLast();
    emit basicVideoView.mPlayer->mediaStatusChanged(QMediaPlayer::MediaStatus::EndOfMedia);
    QCOMPARE(reqPlayNextOneMediaSpy.count(), 1);
    reqPlayNextOneMediaSpy.takeLast();

    // 非结束状态, 不发送
    basicVideoView.onMediaStatusChanged(QMediaPlayer::MediaStatus::LoadedMedia);
    QCOMPARE(reqPlayNextOneMediaSpy.count(), 0);
    emit basicVideoView.mPlayer->mediaStatusChanged(QMediaPlayer::MediaStatus::LoadedMedia);
    QCOMPARE(reqPlayNextOneMediaSpy.count(), 0);
  }

  void resizeEvent_ok() {
    BasicVideoView basicVideoView{false, nullptr};
    basicVideoView.resizeEvent(nullptr);

    QResizeEvent resizeEvent{QSize{480, 360}, QSize{360, 270}};
    basicVideoView.resizeEvent(&resizeEvent);
  }

  void eventFilter_ok() {
    BasicVideoView basicVideoView{false, nullptr};

    QSignalSpy spy{&basicVideoView, &BasicVideoView::userMousePressOrKeyPressHappened};
    QEvent mouseBtnPressEvent{QEvent::Type::MouseButtonPress};
    basicVideoView.eventFilter(&basicVideoView, &mouseBtnPressEvent);
    QCOMPARE(spy.count(), 1);
    spy.takeLast();
    QCOMPARE(basicVideoView.mVideoWidget->isClickPressHappend(), true);

    QEvent keyPressEvent{QEvent::Type::KeyPress};
    basicVideoView.eventFilter(&basicVideoView, &keyPressEvent);
    QCOMPARE(spy.count(), 1);
    spy.takeLast();
    QCOMPARE(basicVideoView.mVideoWidget->isClickPressHappend(), true);

    QEvent keyReleaseEvent{QEvent::Type::KeyRelease};
    basicVideoView.eventFilter(&basicVideoView, &keyReleaseEvent);
    QCOMPARE(spy.count(), 0);
  }
};

#include "BasicVideoViewTest.moc"
REGISTER_TEST(BasicVideoViewTest, false)
