#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "ScenePageControl.h"
#include "EndToExposePrivateMember.h"

class ScenePageControlTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initialized_ok() {
    Configuration().setValue("SCENES_COUNT_EACH_PAGE", 999);
    ScenePageControl spc{"Page Navigation Control"};

    QVERIFY(spc._THE_FRONT_PAGE != nullptr);
    QVERIFY(spc._PREVIOUS_PAGE != nullptr);
    QVERIFY(spc._NEXT_PAGE != nullptr);
    QVERIFY(spc._THE_BACK_PAGE != nullptr);
    QVERIFY(spc.mPageDimensionLE != nullptr);
    QVERIFY(spc.mPageIndexInputLE != nullptr);
  }

  void navigation_front_back_ok() {
    ScenePageControl spc;
    spc.onPagesCountChanged(5); // 总页数=5
    spc.mPageIndexInputLE->setText("2"); // 当前页=2（第3页）

    // 跳转到首页
    spc.PageIndexIncDec(spc._THE_FRONT_PAGE);
    QCOMPARE(spc.mPageIndexInputLE->text(), "0");

    // 跳转到末页
    spc.PageIndexIncDec(spc._THE_BACK_PAGE);
    QCOMPARE(spc.mPageIndexInputLE->text(), "4");

    // 再次跳转到首页
    spc.PageIndexIncDec(spc._THE_FRONT_PAGE);
    QCOMPARE(spc.mPageIndexInputLE->text(), "0");

    // 测试零页情况
    spc.onPagesCountChanged(0);
    spc.PageIndexIncDec(spc._THE_FRONT_PAGE);
    QCOMPARE(spc.mPageIndexInputLE->text(), "0");
    spc.PageIndexIncDec(spc._THE_BACK_PAGE);
    QCOMPARE(spc.mPageIndexInputLE->text(), "0");
  }

  void navigation_previous_next_wrap_ok() {
    ScenePageControl spc;
    spc.onPagesCountChanged(5); // 总页数=5
    spc.mPageIndexInputLE->setText("2"); // 当前页=2（第3页）

    // 连续点击3次PREVIOUS
    spc.PageIndexIncDec(spc._PREVIOUS_PAGE); // 2 -> 1
    spc.PageIndexIncDec(spc._PREVIOUS_PAGE); // 1 -> 0
    spc.PageIndexIncDec(spc._PREVIOUS_PAGE); // 0 -> 4（卷绕到最后一页）

    QCOMPARE(spc.mPageIndexInputLE->text(), "4");

    // 重置到第2页
    spc.mPageIndexInputLE->setText("2");

    // 连续点击3次NEXT
    spc.PageIndexIncDec(spc._NEXT_PAGE); // 2 -> 3
    spc.PageIndexIncDec(spc._NEXT_PAGE); // 3 -> 4
    spc.PageIndexIncDec(spc._NEXT_PAGE); // 4 -> 0（卷绕到第一页）

    QCOMPARE(spc.mPageIndexInputLE->text(), "0");
  }

  void navigation_invalid_action() {
    ScenePageControl spc;
    spc.onPagesCountChanged(5);
    spc.mPageIndexInputLE->setText("2");

    // 创建无效动作
    QAction invalidAction;

    // 触发无效动作
    bool result = spc.PageIndexIncDec(&invalidAction);

    QVERIFY(!result);                              // 应返回false
    QCOMPARE(spc.mPageIndexInputLE->text(), "2");  // 页码不应改变
  }

  void currentPageIndexChanged_signal_ok() {
    ScenePageControl spc;
    QSignalSpy spy(&spc, &ScenePageControl::currentPageIndexChanged);

    spc.onPagesCountChanged(5);
    spc.mPageIndexInputLE->setText("2");

    // 触发下一页
    spc.PageIndexIncDec(spc._NEXT_PAGE);

    QCOMPARE(spy.count(), 1);                    // 信号应被触发一次
    QCOMPARE(spy.takeFirst().at(0).toInt(), 3);  // 信号参数应为3
  }

  void maxScenesCountPerPageChanged_signal_ok() {
    ScenePageControl spc;
    QSignalSpy spy(&spc, &ScenePageControl::maxScenesCountPerPageChanged);

    // 设置有效值
    spc.mPageDimensionLE->setText("50");
    spc.SetScenesCountPerPage();

    QCOMPARE(spy.count(), 1);                     // 信号应被触发一次
    QCOMPARE(spy.takeFirst().at(0).toInt(), 50);  // 信号参数应为50

    // 设置无效值
    spc.mPageDimensionLE->setText("invalid");
    bool result = spc.SetScenesCountPerPage();

    QVERIFY(!result);          // 应返回false
    QCOMPARE(spy.count(), 0);  // 不应触发信号
  }

  void page_index_input_validation() {
    ScenePageControl spc;
    spc.onPagesCountChanged(5);

    // 设置有效值
    spc.mPageIndexInputLE->setText("3");
    spc.SetPageIndex();

    // 设置无效值
    spc.mPageIndexInputLE->setText("invalid");
    spc.SetPageIndex();  // 不应触发信号

    // 设置超出范围的值
    spc.mPageIndexInputLE->setText("10");
    spc.SetPageIndex();  // 会触发信号，但应由接收方处理
  }
};

#include "ScenePageControlTest.moc"
REGISTER_TEST(ScenePageControlTest, false)
