#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "FullScreenableSplitter.h"
#include "EndToExposePrivateMember.h"
#include <QPushButton>

class FullScreenableSplitterDerived : public FullScreenableSplitter {
 public:
  explicit FullScreenableSplitterDerived(const QString& memoryKeyName)               //
      : FullScreenableSplitter{memoryKeyName, Qt::Orientation::Vertical, nullptr} {  //
    wid0 = new QWidget{this};
    btn1 = new QPushButton{"btn need fullscreen", this};
    btn1->setCheckable(true);
    btn1->setChecked(false);
    wid2 = new QWidget{this};
    addWidget(wid0);
    addWidget(btn1);
    addWidget(wid2);
    wid0->setFixedSize(20, 10);
    btn1->setFixedSize(20, 10);
    wid2->setFixedSize(20, 10);
    move(0, 0);
    setFixedSize(20, 30);
    setContentsMargins(0, 0, 0, 0);

    restoreState(Configuration().value(GetMemoryName() + "_STATE").toByteArray());

    connect(btn1, &QPushButton::toggled, this, &FullScreenableSplitterDerived::onReqFullscreenModeChange);
  }
  ~FullScreenableSplitterDerived() {  //
    saveStateInDerivedDestructor();
  }

  QWidget* GetFullScreenableWidget() const override { return btn1; }
  QWidget *wid0{nullptr}, *wid2{nullptr};
  QPushButton* btn1{nullptr};
};

class FullScreenableSplitterTest : public PlainTestSuite {
  Q_OBJECT
 public:
  const QString mMemoryName{"FullScreenableSplitterDerived"};
 private slots:
  void fullScreen_not_save_state_ok() {
    Configuration().setValue(mMemoryName + "_STATE", QByteArray{});
    QByteArray beforeState;
    {
      // 非全屏, 可以保存状态->保存状态, 全屏, 不可以保存状态->非全屏, 恢复状态, 可以保存状态
      FullScreenableSplitterDerived wid{mMemoryName};
      QCOMPARE(wid.GetFullScreenableWidget(), wid.btn1);
      QVERIFY(wid.mFullScreenWindow == nullptr);
      QCOMPARE(wid.btn1->isChecked(), false);
      QVERIFY(wid.btn1->parent() == &wid);
      QRect geometryNotFullScreen = wid.btn1->geometry();
      QCOMPARE(wid.indexOf(wid.wid0), 0);
      QCOMPARE(wid.indexOf(wid.btn1), 1);
      QCOMPARE(wid.indexOf(wid.wid2), 2);
      QCOMPARE(wid.mBeforeFullScreenState.isEmpty(), true);
      QCOMPARE(wid.needSaveStateWhenClose(), true);

      wid.btn1->toggle();
      QVERIFY(wid.mFullScreenWindow != nullptr);
      QCOMPARE(wid.mFullScreenWindow->isFullScreen(), true);
      const Qt::WindowFlags fullScreenflags{wid.mFullScreenWindow->windowFlags()};
      QVERIFY(fullScreenflags.testFlag(Qt::WindowType::Window));
      QCOMPARE(wid.btn1->isChecked(), true);
      QVERIFY(wid.btn1->parent() == wid.mFullScreenWindow);
      QRect geometryFullScreen = wid.btn1->geometry();
      QCOMPARE(wid.indexOf(wid.wid0), 0);
      QCOMPARE(wid.indexOf(wid.wid2), 1);
      QCOMPARE(wid.indexOf(wid.btn1), -1);
      QCOMPARE(wid.mBeforeFullScreenState.isEmpty(), false);
      QCOMPARE(wid.mBeforeFullScreenState,  //
               Configuration().value(mMemoryName + "_STATE").toByteArray());
      QCOMPARE(wid.needSaveStateWhenClose(), false);
      beforeState = wid.mBeforeFullScreenState;

      // 手动刷新splitter的state状态
      wid.wid0->setFixedSize(20, 10);
      wid.btn1->setFixedSize(20, 20);
      wid.wid2->setFixedSize(20, 10);
      wid.setFixedSize(20, 40);
      wid.move(0, 0);
    }
    // 析构中无需保存
    QCOMPARE(Configuration().value(mMemoryName + "_STATE").toByteArray(), beforeState);
  }

  void fullScreen_save_state_ok() {
    Configuration().setValue(mMemoryName + "_STATE", QByteArray{});
    QByteArray beforeState;
    {
      // 非全屏, 可以保存状态->保存状态, 全屏, 不可以保存状态->非全屏, 恢复状态, 可以保存状态
      FullScreenableSplitterDerived wid{mMemoryName};
      QVERIFY(wid.mFullScreenWindow == nullptr);
      QCOMPARE(wid.btn1->isChecked(), false);
      QVERIFY(wid.btn1->parent() == &wid);
      QRect geometryNotFullScreen = wid.btn1->geometry();
      QCOMPARE(wid.indexOf(wid.wid0), 0);
      QCOMPARE(wid.indexOf(wid.btn1), 1);
      QCOMPARE(wid.indexOf(wid.wid2), 2);
      QCOMPARE(wid.mBeforeFullScreenState.isEmpty(), true);
      QCOMPARE(wid.needSaveStateWhenClose(), true);

      wid.btn1->toggle();
      QVERIFY(wid.mFullScreenWindow != nullptr);
      QCOMPARE(wid.mFullScreenWindow->isFullScreen(), true);
      const Qt::WindowFlags fullScreenflags{wid.mFullScreenWindow->windowFlags()};
      QVERIFY(fullScreenflags.testFlag(Qt::WindowType::Window));
      QCOMPARE(wid.btn1->isChecked(), true);
      QVERIFY(wid.btn1->parent() == wid.mFullScreenWindow);
      QRect geometryFullScreen = wid.btn1->geometry();
      QCOMPARE(wid.indexOf(wid.wid0), 0);
      QCOMPARE(wid.indexOf(wid.wid2), 1);
      QCOMPARE(wid.indexOf(wid.btn1), -1);
      QCOMPARE(wid.mBeforeFullScreenState.isEmpty(), false);
      QCOMPARE(wid.mBeforeFullScreenState,  //
               Configuration().value(mMemoryName + "_STATE").toByteArray());
      QCOMPARE(wid.needSaveStateWhenClose(), false);

      wid.btn1->toggle();
      QVERIFY(wid.mFullScreenWindow == nullptr);
      QCOMPARE(wid.btn1->isChecked(), false);
      QVERIFY(wid.btn1->parent() == &wid);
      QCOMPARE(wid.indexOf(wid.wid0), 0);
      QCOMPARE(wid.indexOf(wid.btn1), 1);
      QCOMPARE(wid.indexOf(wid.wid2), 2);

      QTimer::singleShot(0, &wid, [&wid, geometryNotFullScreen, &beforeState]() {
        QRect geometryNotFullScreenRecover = wid.btn1->geometry();
        beforeState = wid.saveState();
        QCOMPARE(geometryNotFullScreenRecover, geometryNotFullScreen);
      });
      QCOMPARE(wid.needSaveStateWhenClose(), true);

      // 手动刷新splitter的state状态
      wid.wid0->setFixedSize(20, 10);
      wid.btn1->setFixedSize(20, 20);
      wid.wid2->setFixedSize(20, 10);
      wid.setFixedSize(20, 40);
      wid.move(0, 0);
    }
    // 析构中应当保存
    QVERIFY(Configuration().value(mMemoryName + "_STATE").toByteArray() != beforeState);
  }
};

#include "FullScreenableSplitterTest.moc"
REGISTER_TEST(FullScreenableSplitterTest, false)
