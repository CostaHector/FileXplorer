#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "InteractiveVideoWidget.h"
#include "EndToExposePrivateMember.h"
#include "PublicVariable.h"
#include <QFileDialog>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class InteractiveVideoWidgetTest : public PlainTestSuite {
  Q_OBJECT
 public:
  void init() { GlobalMockObject::reset(); }

  void cleanup() { GlobalMockObject::verify(); }
 private slots:
  void initTestCase() {  //
    Configuration().clear();
    {
      InteractiveVideoWidget videoWid{true, nullptr};
      QCOMPARE(videoWid.mFullScreenAct->isChecked(), false);
      QCOMPARE(videoWid.mPauseAct->isChecked(), true);
      QCOMPARE(videoWid.mHideToolBarAct->isChecked(), false);
      QCOMPARE(videoWid.mShowVideoList->isChecked(), true);
      QCOMPARE(videoWid.mLongTimeNoClickTimer.isActive(), false);
      QCOMPARE(videoWid.isClickPressHappend(), false);

      QWidget* extToolBarHide = videoWid.GetExtendedFunctionCtrlBar(&videoWid);
      QVERIFY(extToolBarHide != nullptr);
      QCOMPARE(extToolBarHide->isHidden(), true);

      QCOMPARE(videoWid.GetPlaybackMode(), InteractiveVideoWidget::DEFAULT_PLAYBACK_MODE);
      QCOMPARE(videoWid.GetPlaybackTriggerMode(), VideoPlayTool::DEFAULT_PLAYBACK_TRIGGER_MODE);

      QToolButton* playbackBtn = videoWid.GetPlaybackModelMenuToolButton(&videoWid);
      QToolButton* playbackTriggerBtn = videoWid.GetPlaybackTriggerModelMenuToolButton(&videoWid);

      QSignalSpy spy1{&videoWid, &InteractiveVideoWidget::playbackModeChanged};
      QSignalSpy spy2{&videoWid, &InteractiveVideoWidget::playbackTriggerModeChanged};

      videoWid.mPlaybackMode_Loop->setChecked(true);
      videoWid.mPlaybackTrigger_AUTO->setChecked(true);

      emit playbackBtn->triggered(videoWid.mPlaybackMode_Loop);
      emit playbackTriggerBtn->triggered(videoWid.mPlaybackTrigger_AUTO);

      videoWid.onPlaybackModeTriggered(videoWid.mPlaybackTrigger_AUTO);
      videoWid.onPlaybackTriggerModeTriggered(videoWid.mPlaybackMode_Loop);
      QCOMPARE(spy1.count(), 1);
      QCOMPARE(spy2.count(), 1);
    }

    {
      QVERIFY(Configuration().contains(MemoryKey::VIDEO_PLAYER_PLAYBACK_MODE.name));
      QCOMPARE(Configuration().value(MemoryKey::VIDEO_PLAYER_PLAYBACK_MODE.name).toInt(), (int)QMediaPlaylist::PlaybackMode::Loop);

      QVERIFY(Configuration().contains(MemoryKey::VIDEO_PLAYER_PLAYBACK_TRIGGER_MODE.name));
      QCOMPARE(Configuration().value(MemoryKey::VIDEO_PLAYER_PLAYBACK_TRIGGER_MODE.name).toInt(), (int)VideoPlayTool::PlaybackTriggerMode::AUTO);
    }

    {
      InteractiveVideoWidget videoWid{false, nullptr};
      QCOMPARE(videoWid.GetPlaybackMode(), QMediaPlaylist::PlaybackMode::Loop);
      QCOMPARE(videoWid.GetPlaybackTriggerMode(), VideoPlayTool::PlaybackTriggerMode::AUTO);
      videoWid.contextMenuEvent(nullptr);  // no crash happened
      QVERIFY(videoWid.mContextMenu != nullptr);
      QCOMPARE(InteractiveVideoWidget::GetFocusCore(nullptr), false);
      QCOMPARE(InteractiveVideoWidget::GetFocusCore(&videoWid), true);

      QWidget* extToolBarShow = videoWid.GetExtendedFunctionCtrlBar(&videoWid);
      QVERIFY(extToolBarShow != nullptr);
      QCOMPARE(extToolBarShow->isHidden(), false);
    }
  }

  void keyboard_event_ok() {
    // 初始状态下无焦点, 全屏模式下获得焦点, 响应键盘案件
    MOCKER(InteractiveVideoWidget::GetFocusCore).expects(exactly(1)).will(returnValue(true));

    InteractiveVideoWidget videoWid;
    QCOMPARE(videoWid.mHideToolBarAct->isChecked(), false);
    QCOMPARE(videoWid.hasFocus(), false);
    QCOMPARE(videoWid.isVideoFullScreen(), false);
    QCOMPARE(videoWid.mFullScreenAct->isChecked(), false);
    videoWid.mFullScreenAct->toggle();
    QCOMPARE(videoWid.mFullScreenAct->isChecked(), true);
    QCOMPARE(videoWid.isVideoFullScreen(), true);
    // QCOMPARE(videoWid.hasFocus(), true); // 没有调用show, 此处不检查, 而是检查是否调用了GetFocusCore
    {
      QSignalSpy toggleToolBarSpy{videoWid.mHideToolBarAct, &QAction::toggled};
      QKeyEvent altReturnEvent1(QEvent::KeyPress, Qt::Key_Return, Qt::AltModifier);
      videoWid.keyPressEvent(&altReturnEvent1);
      QCOMPARE(altReturnEvent1.isAccepted(), true);
      QCOMPARE(videoWid.mHideToolBarAct->isChecked(), true);
      QCOMPARE(toggleToolBarSpy.count(), 1);
      QCOMPARE(toggleToolBarSpy.takeLast(), (QVariantList{true}));
      toggleToolBarSpy.clear();

      QKeyEvent altReturnEvent2(QEvent::KeyPress, Qt::Key_Return, Qt::AltModifier);
      videoWid.keyPressEvent(&altReturnEvent2);
      QCOMPARE(altReturnEvent2.isAccepted(), true);
      QCOMPARE(videoWid.mHideToolBarAct->isChecked(), false);
      QCOMPARE(toggleToolBarSpy.count(), 1);
      QCOMPARE(toggleToolBarSpy.takeLast(), (QVariantList{false}));
      toggleToolBarSpy.clear();
    }

    {
      QKeyEvent next10SecondsEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
      videoWid.keyPressEvent(&next10SecondsEvent);
      QCOMPARE(next10SecondsEvent.isAccepted(), true);
    }

    {
      QKeyEvent precious10SecondsEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
      videoWid.keyPressEvent(&precious10SecondsEvent);
      QCOMPARE(precious10SecondsEvent.isAccepted(), true);
    }

    {
      QKeyEvent volumeUpEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
      videoWid.keyPressEvent(&volumeUpEvent);
      QCOMPARE(volumeUpEvent.isAccepted(), true);
    }

    {
      QKeyEvent volumeDownEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
      videoWid.keyPressEvent(&volumeDownEvent);
      QCOMPARE(volumeDownEvent.isAccepted(), true);
    }

    {
      QKeyEvent previousMediaEvent(QEvent::KeyPress, Qt::Key_PageUp, Qt::NoModifier);
      videoWid.keyPressEvent(&previousMediaEvent);
      QCOMPARE(previousMediaEvent.isAccepted(), true);
    }

    {
      QKeyEvent nextMediaEvent(QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier);
      videoWid.keyPressEvent(&nextMediaEvent);
      QCOMPARE(nextMediaEvent.isAccepted(), true);
    }

    {
      QCOMPARE(videoWid.mPauseAct->isChecked(), true);
      QKeyEvent pauseToPlayToggleEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
      videoWid.keyPressEvent(&pauseToPlayToggleEvent);
      QCOMPARE(pauseToPlayToggleEvent.isAccepted(), true);
      QCOMPARE(videoWid.mPauseAct->isChecked(), false);
      QKeyEvent playToPauseToggleEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
      videoWid.keyPressEvent(&playToPauseToggleEvent);
      QCOMPARE(playToPauseToggleEvent.isAccepted(), true);
      QCOMPARE(videoWid.mPauseAct->isChecked(), true);
    }

    {
      QCOMPARE(videoWid.mFullScreenAct->isChecked(), true);
      QKeyEvent quitFullScreenEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
      videoWid.keyPressEvent(&quitFullScreenEvent);
      QCOMPARE(quitFullScreenEvent.isAccepted(), true);
      QCOMPARE(videoWid.mFullScreenAct->isChecked(), false);
    }

    // mFullScreenAct的快捷键触发最大化, 不走keyPressEvent流程
    {
      QCOMPARE(videoWid.mFullScreenAct->shortcut(), (QKeySequence{Qt::Key_F12}));
      QKeyEvent f12ShortcutEvent(QEvent::KeyPress, Qt::Key_F12, Qt::NoModifier);
      videoWid.keyPressEvent(&f12ShortcutEvent);
      QCOMPARE(f12ShortcutEvent.isAccepted(), false);
    }
  }

  void mousepress_event_ok() {
    // 鼠标左键点击暂停/播放 toggle
    InteractiveVideoWidget videoWid;
    QCOMPARE(videoWid.mPauseAct->isChecked(), true);

    QSignalSpy pauseToggledSpy{videoWid.mPauseAct, &QAction::toggled};

    QMouseEvent leftClickPause2PlayToggleEvent{QEvent::MouseButtonPress, videoWid.geometry().center(), Qt::LeftButton, Qt::LeftButton,
                                               Qt::NoModifier};
    videoWid.mousePressEvent(&leftClickPause2PlayToggleEvent);
    QCOMPARE(leftClickPause2PlayToggleEvent.isAccepted(), true);
    QCOMPARE(pauseToggledSpy.count(), 1);
    QCOMPARE(pauseToggledSpy.takeLast(), (QVariantList{false}));

    QMouseEvent leftClickPlay2PauseToggleEvent{QEvent::MouseButtonPress, videoWid.geometry().center(), Qt::LeftButton, Qt::LeftButton,
                                               Qt::NoModifier};
    videoWid.mousePressEvent(&leftClickPlay2PauseToggleEvent);
    QCOMPARE(leftClickPlay2PauseToggleEvent.isAccepted(), true);
    QCOMPARE(pauseToggledSpy.count(), 1);
    QCOMPARE(pauseToggledSpy.takeLast(), (QVariantList{true}));

    QCOMPARE(videoWid.updatePauseActionState(false), true);
    QCOMPARE(videoWid.updatePauseActionState(false), false);  // no need update, already unchecked
    QCOMPARE(videoWid.mPauseAct->isChecked(), false);

    // 鼠标中键不处理
    QMouseEvent middleButtonClickEvent{QEvent::MouseButtonPress, videoWid.geometry().center(), Qt::MiddleButton, Qt::MiddleButton, Qt::NoModifier};
    videoWid.mousePressEvent(&middleButtonClickEvent);
    QCOMPARE(middleButtonClickEvent.isAccepted(), false);

    // 鼠标右键 全屏模式下同时显示菜单和工具栏, 非全屏模式下只显示工具栏 见 auto_hide_toolbar_ok 用例
  }

  void auto_hide_toolbar_ok() {
    InteractiveVideoWidget videoWid;
    QVERIFY(!videoWid.mLongTimeNoClickTimer.isActive());  // 初始状态下不活跃
    QSignalSpy layoutVisibilityChangedSpy{&videoWid, &InteractiveVideoWidget::layoutVisibilityChanged};

    // 进入全屏模式, 启动定时器
    QCOMPARE(videoWid.mFullScreenAct->isChecked(), false);
    videoWid.mFullScreenAct->toggle();
    QCOMPARE(videoWid.mFullScreenAct->isChecked(), true);
    QVERIFY(videoWid.mLongTimeNoClickTimer.isActive());  // 活跃

    // 用户点击/按下键盘事件
    videoWid.onUserMouseClickOrKeyPressEvent();
    QCOMPARE(videoWid.isClickPressHappend(), true);

    // 模拟首次超时， 重启定时器， 下次再隐藏
    videoWid.mLongTimeNoClickTimer.stop();  //
    videoWid.onLongTimeNoEventHappen();
    QVERIFY(videoWid.mLongTimeNoClickTimer.isActive());
    QCOMPARE(videoWid.isClickPressHappend(), false);
    QCOMPARE(videoWid.mHideToolBarAct->isChecked(), false);
    QCOMPARE(videoWid.mShowVideoList->isChecked(), true);
    QCOMPARE(layoutVisibilityChangedSpy.count(), 0);

    // 第二次超时后, 将自动隐藏工具栏和视频文件列表
    videoWid.mLongTimeNoClickTimer.stop();  // 模拟超时停止
    videoWid.onLongTimeNoEventHappen();
    QVERIFY(!videoWid.mLongTimeNoClickTimer.isActive());  // 停用定时器
    QCOMPARE(videoWid.isClickPressHappend(), false);

    QCOMPARE(videoWid.mHideToolBarAct->isChecked(), true);
    QCOMPARE(videoWid.mShowVideoList->isChecked(), false);
    QCOMPARE(layoutVisibilityChangedSpy.count(), 1);
    layoutVisibilityChangedSpy.takeLast();

    // 右键点击事件, 显示工具栏和视频文件列表, a.全屏模式下还会重新启动定时器
    {
      QMouseEvent rightClickEvent{QEvent::MouseButtonPress, videoWid.geometry().center(), Qt::RightButton, Qt::RightButton, Qt::NoModifier};
      videoWid.mousePressEvent(&rightClickEvent);
      QCOMPARE(rightClickEvent.isAccepted(), true);

      QCOMPARE(videoWid.mHideToolBarAct->isChecked(), false);
      QCOMPARE(videoWid.mShowVideoList->isChecked(), true);
      QCOMPARE(layoutVisibilityChangedSpy.count(), 1);
      layoutVisibilityChangedSpy.takeLast();
      QVERIFY(videoWid.mLongTimeNoClickTimer.isActive());  // 重新启动定时器
    }

    // 退出全屏模式将显示工具栏和视频文件列表
    {
      QCOMPARE(videoWid.mFullScreenAct->isChecked(), true);
      videoWid.mFullScreenAct->toggle();
      QCOMPARE(videoWid.mFullScreenAct->isChecked(), false);

      QCOMPARE(videoWid.mHideToolBarAct->isChecked(), false);
      QCOMPARE(videoWid.mShowVideoList->isChecked(), true);
      QVERIFY(!videoWid.mLongTimeNoClickTimer.isActive());  // 停用定时器
    }

    // 预制: 隐藏工具栏和视频文件列表
    {
      videoWid.mHideToolBarAct->toggle();
      videoWid.mShowVideoList->toggle();
      QCOMPARE(videoWid.mHideToolBarAct->isChecked(), true);
      QCOMPARE(videoWid.mShowVideoList->isChecked(), false);
    }

    // 右键点击事件, 显示工具栏和视频文件列表, b.非全屏模式下不会重新启动定时器
    {
      QMouseEvent rightClickEvent{QEvent::MouseButtonPress, videoWid.geometry().center(), Qt::RightButton, Qt::RightButton, Qt::NoModifier};
      videoWid.mousePressEvent(&rightClickEvent);
      QCOMPARE(rightClickEvent.isAccepted(), true);

      QCOMPARE(videoWid.mHideToolBarAct->isChecked(), false);
      QCOMPARE(videoWid.mShowVideoList->isChecked(), true);
      QCOMPARE(layoutVisibilityChangedSpy.count(), 1);
      layoutVisibilityChangedSpy.takeLast();
      QVERIFY(!videoWid.mLongTimeNoClickTimer.isActive());  // 不会重新启动定时器
    }
  }

  void selectAFile_ok() {
    Configuration().setValue(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, "invalid path");
    const QString openPath{SystemPath::HOME_PATH()};

    InteractiveVideoWidget videoWid;
    QSignalSpy selectAFileSpy{&videoWid, &InteractiveVideoWidget::newFileSelectedByUser};

    MOCKER(QFileDialog::getOpenFileName)                    //
        .expects(exactly(3))                                //
        .with(any(), any(), openPath, any(), any(), any())  //
        .will(returnValue(QString{""}))                     // 用户取消选择
        .then(returnValue(QString{""}))                     // 用户取消选择
        .then(returnValue(QString{"path/to/MediaFile.mp4"}));

    emit videoWid.mSelectVideoFileAct->triggered(false);
    QCOMPARE(selectAFileSpy.count(), 0);

    QCOMPARE(videoWid.onSelectAFile(), false);
    QCOMPARE(selectAFileSpy.count(), 0);

    QCOMPARE(videoWid.onSelectAFile(), true);
    QCOMPARE(selectAFileSpy.count(), 1);
    QCOMPARE(selectAFileSpy.takeLast(), (QVariantList{"path/to/MediaFile.mp4", true}));

    Configuration().setValue(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, "path/to");
  }

  void selectAFolder_ok() {
    Configuration().setValue(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, "invalid path");
    const QString openPath{SystemPath::HOME_PATH()};

    InteractiveVideoWidget videoWid;
    QSignalSpy selectAFolderSpy{&videoWid, &InteractiveVideoWidget::newFolderSelectedChangedByUser};

    MOCKER(QFileDialog::getExistingDirectory)  //
        .expects(exactly(3))                   //
        .with(any(), any(), openPath, any())   //
        .will(returnValue(QString{""}))        // 用户取消选择
        .then(returnValue(QString{""}))        // 用户取消选择
        .then(returnValue(QString{"path/to/MediaFolder"}));

    emit videoWid.mSelectVideoFolder->triggered(false);
    QCOMPARE(selectAFolderSpy.count(), 0);

    QCOMPARE(videoWid.onSelectAFolder(), false);
    QCOMPARE(selectAFolderSpy.count(), 0);

    QCOMPARE(videoWid.onSelectAFolder(), true);
    QCOMPARE(selectAFolderSpy.count(), 1);
    QCOMPARE(selectAFolderSpy.takeLast(), (QVariantList{"path/to/MediaFolder", true}));

    Configuration().setValue(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, "path/to");
  }
};

#include "InteractiveVideoWidgetTest.moc"
REGISTER_TEST(InteractiveVideoWidgetTest, false)
