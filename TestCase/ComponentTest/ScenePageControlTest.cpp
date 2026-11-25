#include <QtTest/QtTest>
#include "PlainTestSuite.h"


#include "BeginToExposePrivateMember.h"
#include "ScenePageControl.h"
#include "EndToExposePrivateMember.h"
#include "Configuration.h"

#include <QSignalSpy>
#include <QSpinBox>

class ScenePageControlTest : public PlainTestSuite {
  Q_OBJECT
private slots:

  void init() {
    Configuration().clear();
  }

  void initialized_ok() {
    ScenePageControl spc;

    // 重构后仅保留首页、末页Action
    QVERIFY(spc._THE_FRONT_PAGE != nullptr);
    QVERIFY(spc._THE_BACK_PAGE != nullptr);
    QVERIFY(spc.mScenesPerPageInputSB != nullptr);
    QVERIFY(spc.mPageIndexInputSB != nullptr);

    QCOMPARE(spc.mScenesPerPageInputSB->value(), 40); // initial=40 if no config

    QCOMPARE(spc.mPageIndexInputSB->prefix(), "0<=");
    QCOMPARE(spc.mPageIndexInputSB->suffix(), "<1");
    spc.onPagesCountChanged(99);
    QCOMPARE(spc.mPageIndexInputSB->prefix(), "0<=");
    QCOMPARE(spc.mPageIndexInputSB->suffix(), "<99");
  }

  void navigation_front_back_ok() {
    ScenePageControl spc;
    spc.onPagesCountChanged(5); // 总页数=5, [0, 5)
    // ->2
    spc.mPageIndexInputSB->setValue(2); // 当前页=2

    // ->0
    spc.onPageIndexFrontBackActTriggered(spc._THE_FRONT_PAGE);
    QCOMPARE(spc.mPageIndexInputSB->value(), 0);

    // ->4
    spc.onPageIndexFrontBackActTriggered(spc._THE_BACK_PAGE);
    QCOMPARE(spc.mPageIndexInputSB->value(), 4);

    // ->0
    spc.onPageIndexFrontBackActTriggered(spc._THE_FRONT_PAGE);
    QCOMPARE(spc.mPageIndexInputSB->value(), 0);

    {
      QAction invalidAction;
      const bool result1 = spc.onPageIndexFrontBackActTriggered(&invalidAction);
      QVERIFY(!result1);
      QCOMPARE(spc.mPageIndexInputSB->value(), 0);

      const bool result2 = spc.onPageIndexFrontBackActTriggered(nullptr);
      QVERIFY(!result2);
      QCOMPARE(spc.mPageIndexInputSB->value(), 0);
    }
  }

  void currentPageIndexChanged_signal_ok() {
    ScenePageControl spc;
    QSignalSpy spy(&spc, &ScenePageControl::currentPageIndexChanged);
    QCOMPARE(spc.mPageIndexInputSB->value(), 0);
    spc.onPagesCountChanged(5);

    spc.mPageIndexInputSB->setValue(2);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeLast(), QVariantList{QVariant{2}});
    spy.clear();

    // 直接操作spinbox赋值代替
    spc.mPageIndexInputSB->setValue(3);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeLast(), QVariantList{QVariant{3}});
    spy.clear();

    // SpinBox自动钳位，超出最大值直接clamp到最大值
    const int maxVal = spc.mPageIndexInputSB->maximum();
    spc.mPageIndexInputSB->setValue(maxVal + 1);
    QCOMPARE(spc.mPageIndexInputSB->value(), maxVal);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeLast(), QVariantList{QVariant{maxVal}});
    spy.clear();
  }

  void maxScenesCountPerPageChanged_signal_ok() {
    ScenePageControl spc;
    QSignalSpy spy(&spc, &ScenePageControl::maxScenesCountPerPageChanged);

    // 修改spinbox，触发valueChanged信号
    spc.mScenesPerPageInputSB->setValue(50);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeLast(), QVariantList{QVariant{50}});
    spy.clear();

    // SpinBox越界自动钳位, 也会不触发信号
    const int maxVal = spc.mScenesPerPageInputSB->maximum();
    spc.mScenesPerPageInputSB->setValue(maxVal + 1);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeLast(), QVariantList{QVariant{maxVal}});
    spy.clear();
  }
};

#include "ScenePageControlTest.moc"
REGISTER_TEST(ScenePageControlTest, false)
