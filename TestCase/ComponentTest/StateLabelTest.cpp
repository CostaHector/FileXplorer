#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "StateLabel.h"
#include "EndToExposePrivateMember.h"

class StateLabelTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_state_transitions() {
    // 创建标签并验证初始状态
    StateLabel label("Test Label");
    QCOMPARE(label.state(), StateLabel::SAVED);
    QCOMPARE(label.text(), "Test Label");

    // 测试状态转换到未保存
    label.ToNotSaved();
    QCOMPARE(label.state(), StateLabel::NOT_SAVED);

    // 测试状态转换回已保存
    label.ToSaved();
    QCOMPARE(label.state(), StateLabel::SAVED);

    // 测试多次状态转换
    label.ToNotSaved();
    QCOMPARE(label.state(), StateLabel::NOT_SAVED);
    label.ToNotSaved(); // 已经是未保存状态，再次调用
    QCOMPARE(label.state(), StateLabel::NOT_SAVED); // 状态应保持不变

    label.ToSaved();
    QCOMPARE(label.state(), StateLabel::SAVED);
    label.ToSaved(); // 已经是已保存状态，再次调用
    QCOMPARE(label.state(), StateLabel::SAVED); // 状态应保持不变

    // will not crash down
    GetLabelStatusPixmap(StateLabel::SAVED);
    GetLabelStatusPixmap(StateLabel::NOT_SAVED);
    GetLabelStatusPixmap(StateLabel::BUTT);
  }
};

#include "StateLabelTest.moc"
REGISTER_TEST(StateLabelTest, false)
