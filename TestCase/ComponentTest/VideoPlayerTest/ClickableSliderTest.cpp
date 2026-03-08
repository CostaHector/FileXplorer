#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "BeginToExposePrivateMember.h"
#include "ClickableSlider.h"
#include "EndToExposePrivateMember.h"

class ClickableSliderTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void slider_move_ok() {
    int val{100};

    ClickableSlider volumeSlider{Qt::Orientation::Horizontal};
    volumeSlider.setRange(0, 100);
    volumeSlider.setValue(val);
    QVERIFY(!volumeSlider.isRegistered());
    // 禁止注册空指针
    QVERIFY(!volumeSlider.regMouseEventProcessor(nullptr));
    QVERIFY(!volumeSlider.isRegistered());
    // 注册成功
    QVERIFY(volumeSlider.regMouseEventProcessor([&val](int newPosition) { val = newPosition; }));
    QVERIFY(volumeSlider.isRegistered());
    // 禁止多次注册
    QVERIFY(!volumeSlider.regMouseEventProcessor([](int newPosition) {}));
    QVERIFY(volumeSlider.isRegistered());

    emit volumeSlider.sliderMoved(0);  // 并不会主动调用setValue()
    QCOMPARE(val, 0);
  }

  void no_register_also_setValue_ok() {
    ClickableSlider volumeSlider{Qt::Orientation::Horizontal};
    volumeSlider.setRange(0, 100);
    volumeSlider.setValue(100);

    // 不注册时, 点击也能够更新数值
    QSignalSpy sliderSpy{&volumeSlider, &ClickableSlider::valueChanged};
    QMouseEvent pressLeftCorner(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    volumeSlider.mousePressEvent(&pressLeftCorner);
    QVERIFY(pressLeftCorner.isAccepted());

    QCOMPARE(volumeSlider.value(), 0);
    QCOMPARE(sliderSpy.count(), 1);
    QCOMPARE(sliderSpy.takeLast(), (QList<QVariant>{0}));
  }

  void mouse_click_ok() {
    int volumeValueNeedSetToPlayer{100};

    ClickableSlider volumeSlider{Qt::Orientation::Horizontal};
    volumeSlider.setRange(0, 100);
    volumeSlider.setValue(volumeValueNeedSetToPlayer);

    volumeSlider.regMouseEventProcessor([&volumeValueNeedSetToPlayer](int newPosition) { volumeValueNeedSetToPlayer = newPosition; });

    QSignalSpy sliderSpy{&volumeSlider, &ClickableSlider::valueChanged};
    // 1. mouse click value 0 水平方向点击最左侧
    {
      QMouseEvent pressLeftCorner(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
      volumeSlider.mousePressEvent(&pressLeftCorner);
      QVERIFY(pressLeftCorner.isAccepted());

      QCOMPARE(volumeValueNeedSetToPlayer, 0);
      QCOMPARE(volumeSlider.value(), 0);
      QCOMPARE(sliderSpy.count(), 1);
      QCOMPARE(sliderSpy.takeLast(), (QList<QVariant>{0}));
    }

    // 2. setValue 50
    {
      volumeSlider.setValue(50);
      QCOMPARE(sliderSpy.count(), 1);
      QCOMPARE(sliderSpy.takeLast(), (QList<QVariant>{50}));
    }

    // 3. mouse click value 100 水平方向点击最右侧
    {
      QMouseEvent pressRightCorner(QEvent::MouseButtonPress, QPoint(volumeSlider.width(), 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
      volumeSlider.mousePressEvent(&pressRightCorner);
      QVERIFY(pressRightCorner.isAccepted());

      QCOMPARE(volumeValueNeedSetToPlayer, 100);
      QCOMPARE(volumeSlider.value(), 100);
      QCOMPARE(sliderSpy.count(), 1);
      QCOMPARE(sliderSpy.takeLast(), (QList<QVariant>{100}));
    }

    // 4. setValue 14
    {
      volumeSlider.setValue(14);
      QCOMPARE(sliderSpy.count(), 1);
      QCOMPARE(sliderSpy.takeLast(), (QList<QVariant>{14}));
    }

    // 5. mouse click value 100 垂直方向点击底部
    volumeSlider.setOrientation(Qt::Orientation::Vertical);
    {
      QMouseEvent pressBottomCorner(QEvent::MouseButtonPress, QPoint(0, volumeSlider.height()), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
      volumeSlider.mousePressEvent(&pressBottomCorner);
      QVERIFY(pressBottomCorner.isAccepted());

      QCOMPARE(volumeValueNeedSetToPlayer, 100);
      QCOMPARE(volumeSlider.value(), 100);
      QCOMPARE(sliderSpy.count(), 1);
      QCOMPARE(sliderSpy.takeLast(), (QList<QVariant>{100}));
    }

    // 6. 点击相同位置, 值不变, 不调用回调, 不emit valueChanged
    {
      QMouseEvent pressBottomCorner(QEvent::MouseButtonPress, QPoint(0, volumeSlider.height()), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
      volumeSlider.mousePressEvent(&pressBottomCorner);
      QVERIFY(pressBottomCorner.isAccepted());

      QCOMPARE(volumeValueNeedSetToPlayer, 100);
      QCOMPARE(volumeSlider.value(), 100);
      QCOMPARE(sliderSpy.count(), 0);
    }
  }
};

#include "ClickableSliderTest.moc"
REGISTER_TEST(ClickableSliderTest, false)
