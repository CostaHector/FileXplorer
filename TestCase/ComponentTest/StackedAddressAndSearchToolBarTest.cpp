#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "StackedAddressAndSearchToolBar.h"
#include "EndToExposePrivateMember.h"

class StackedAddressAndSearchToolBarTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_toolbar_management() {
    // 创建主工具栏
    StackedAddressAndSearchToolBar toolbar("Test Toolbar");

    // 验证初始状态
    QVERIFY(toolbar.m_stackedToolBar != nullptr);
    QCOMPARE(toolbar.m_stackedToolBar->count(), 0);
    QVERIFY(toolbar.m_name2StackIndex.isEmpty());

    // 创建几个子工具栏
    QToolBar listBar("List Bar");
    QToolBar tableBar("Table Bar");
    QToolBar treeBar("Tree Bar");

    // 添加工具栏并验证
    int listIndex = toolbar.AddToolBar(ViewTypeTool::ViewType::LIST, &listBar);
    QCOMPARE(listIndex, 0);
    QCOMPARE(toolbar.m_stackedToolBar->count(), 1);
    QCOMPARE(toolbar.m_name2StackIndex[ViewTypeTool::ViewType::LIST], 0);
    QCOMPARE(toolbar.m_stackedToolBar->widget(0), &listBar);

    int tableIndex = toolbar.AddToolBar(ViewTypeTool::ViewType::TABLE, &tableBar);
    QCOMPARE(tableIndex, 1);
    QCOMPARE(toolbar.m_stackedToolBar->count(), 2);
    QCOMPARE(toolbar.m_name2StackIndex[ViewTypeTool::ViewType::TABLE], 1);
    QCOMPARE(toolbar.m_stackedToolBar->widget(1), &tableBar);

    int treeIndex = toolbar.AddToolBar(ViewTypeTool::ViewType::TREE, &treeBar);
    QCOMPARE(treeIndex, 2);
    QCOMPARE(toolbar.m_stackedToolBar->count(), 3);
    QCOMPARE(toolbar.m_name2StackIndex[ViewTypeTool::ViewType::TREE], 2);
    QCOMPARE(toolbar.m_stackedToolBar->widget(2), &treeBar);

    // 验证映射关系
    QCOMPARE(toolbar.m_name2StackIndex.size(), 3);
    QCOMPARE(toolbar.m_name2StackIndex.value(ViewTypeTool::ViewType::LIST), 0);
    QCOMPARE(toolbar.m_name2StackIndex.value(ViewTypeTool::ViewType::TABLE), 1);
    QCOMPARE(toolbar.m_name2StackIndex.value(ViewTypeTool::ViewType::TREE), 2);
  }
};

#include "StackedAddressAndSearchToolBarTest.moc"
REGISTER_TEST(StackedAddressAndSearchToolBarTest, false)
