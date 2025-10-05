#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "DraggableToolButton.h"
#include "EndToExposePrivateMember.h"

class DraggableToolButtonTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_mousePressEvent() {
    DraggableToolButton button;

    // 左键按下应设置起始位置
    QMouseEvent leftPress(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button.mousePressEvent(&leftPress);
    QCOMPARE(button.mDragStartPosition, QPoint(10, 10));

    // 右键按下不应设置起始位置
    QMouseEvent rightPress(QEvent::MouseButtonPress, QPoint(20, 20), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    button.mousePressEvent(&rightPress);
    QCOMPARE(button.mDragStartPosition, QPoint(10, 10));  // 保持原值
  }

  void test_mouseMoveEvent_noDrag() {
    DraggableToolButton button;
    button.setEnabled(true);

    // 模拟按下事件
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button.mousePressEvent(&pressEvent);

    // 情况1: 非左键移动
    QMouseEvent nonLeftMove(QEvent::MouseMove, QPoint(20, 20), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    button.mouseMoveEvent(&nonLeftMove);
    QVERIFY(button.isEnabled());

    // 情况2: 移动距离不足
    QMouseEvent smallMove(QEvent::MouseMove, QPoint(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button.mouseMoveEvent(&smallMove);
    QVERIFY(button.isEnabled());
  }

  void test_mouseMoveEvent_startDrag() {
    DraggableToolButton button;
    button.setText("Test Button");
    button.setEnabled(true);

    // 模拟按下事件
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button.mousePressEvent(&pressEvent);

    // 创建足够距离的移动事件
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    button.mouseMoveEvent(&moveEvent);

    // 验证按钮状态变化
    QVERIFY(button.isEnabled());
  }
};

#include "DraggableToolButtonTest.moc"
REGISTER_TEST(DraggableToolButtonTest, false)
