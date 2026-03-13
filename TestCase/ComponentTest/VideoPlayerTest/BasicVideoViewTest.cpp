#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "BasicVideoView.h"
#include "EndToExposePrivateMember.h"

#include "FileTool.h"
#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class BasicVideoViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

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
    basicVideoView.mVideoWidget->mOpenInSystemApplication->trigger(); // time: 1

    // 禁用触发: 强制播放(非暂停状态) setMedia:1 play:1
    QVERIFY(basicVideoView.PlayAVideo(existVideoPath, true));
    QCOMPARE(basicVideoView.mVideoWidget->mPauseAct->isChecked(), false);
    QCOMPARE(basicVideoView.GetCurrentPlayingMediaPath(), existVideoPath);
    basicVideoView.reqPlayInSystemApplication(); // time: 2

    // 停止播放 mStopAct -> onStopPlaying -> StopPlay
    emit basicVideoView.mVideoWidget->mStopAct->triggered();
    QCOMPARE(basicVideoView.mVideoWidget->mPauseAct->isChecked(), true);
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

    QCOMPARE(BasicVideoView::SetMediaCore(nullptr, QUrl::fromLocalFile(__FILE__)), false);
    QCOMPARE(BasicVideoView::SetMediaCore(basicVideoView.mPlayer, QUrl::fromLocalFile(__FILE__)), true);

    QCOMPARE(BasicVideoView::PlayCore(nullptr), false);
    QCOMPARE(BasicVideoView::PlayCore(basicVideoView.mPlayer), true);

    QCOMPARE(basicVideoView.onError(QMediaPlayer::Error::NoError), QMediaPlayer::Error::NoError);
    QCOMPARE(basicVideoView.onAudioAvailableChanged(true), 98);  // floor(99*99/100)=floor(9801)=98

    basicVideoView.onMediaPlayStateChanged(QMediaPlayer::State::PausedState);
    QCOMPARE(basicVideoView.mPauseShieldButton->isHidden(), false);

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

  void onMediaPlayStateChanged_ok() {}

  void resizeEvent_ok() {
    BasicVideoView basicVideoView{false, nullptr};
    basicVideoView.resizeEvent(nullptr);

    QResizeEvent resizeEvent{QSize{480, 360}, QSize{360, 270}};
    basicVideoView.resizeEvent(&resizeEvent);
  }
};

#include "BasicVideoViewTest.moc"
REGISTER_TEST(BasicVideoViewTest, false)
