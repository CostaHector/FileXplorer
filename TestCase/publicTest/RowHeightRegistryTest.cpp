#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RowHeightRegistry.h"
#include "EndToExposePrivateMember.h"

#include "CustomTableView.h"
#include "CustomTreeView.h"
template struct RowHeightRegistry<CustomTableView>;
template struct RowHeightRegistry<CustomTreeView>;

class RowHeightRegistryTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() {
    RowHeightRegistry<CustomTableView>::GetWidgetsSet()->clear();
    RowHeightRegistry<CustomTreeView>::GetWidgetsSet()->clear();
  }

  void always_alive() {
    CustomTableView* pTableView = nullptr;
    CustomTreeView* pTreeView = nullptr;
    {
      // 自动注册, 长期存活
      CustomTableView tableView{"tableViewRowHeightChangedOk"};
      CustomTreeView treeView{"treeViewRowHeightChangedOk"};
      pTableView = &tableView;
      pTreeView = &treeView;
      QCOMPARE(RowHeightRegistry<CustomTableView>::GetWidgetsSet()->size(), 1);
      QCOMPARE(RowHeightRegistry<CustomTreeView>::GetWidgetsSet()->size(), 1);

      // 不允许重复注册
      QCOMPARE(RowHeightRegistry<CustomTableView>::registerWidgetForAdjust(&tableView, false), false);
      QCOMPARE(RowHeightRegistry<CustomTreeView>::registerWidgetForAdjust(&treeView, false), false);

      const int beforeTableViewRowHeight = tableView.rowHeight();
      const int beforeTreeViewRowHeight = treeView.rowHeight();

      const int afterTableViewRowHeight = tableView.rowHeight() + 1;
      const int afterTreeViewRowHeight = treeView.rowHeight() + 2;

      // unchange
      QCOMPARE(RowHeightRegistry<CustomTableView>::updateRegisteredWidgetsForAdjust(beforeTableViewRowHeight), 0);
      QCOMPARE(RowHeightRegistry<CustomTreeView>::updateRegisteredWidgetsForAdjust(beforeTreeViewRowHeight), 0);
      QCOMPARE(tableView.rowHeight(), beforeTableViewRowHeight);
      QCOMPARE(treeView.rowHeight(), beforeTreeViewRowHeight);

      // change
      QCOMPARE(RowHeightRegistry<CustomTableView>::updateRegisteredWidgetsForAdjust(afterTableViewRowHeight), 1);
      QCOMPARE(RowHeightRegistry<CustomTreeView>::updateRegisteredWidgetsForAdjust(afterTreeViewRowHeight), 1);
      QCOMPARE(tableView.rowHeight(), afterTableViewRowHeight);
      QCOMPARE(treeView.rowHeight(), afterTreeViewRowHeight);
    }

    // will not removed after deleted
    QCOMPARE(RowHeightRegistry<CustomTableView>::GetWidgetsSet()->size(), 1);
    QCOMPARE(RowHeightRegistry<CustomTreeView>::GetWidgetsSet()->size(), 1);

    // unregister mannually not collapse
    QCOMPARE(RowHeightRegistry<CustomTableView>::unregisterWidgetForAdjust(pTableView), true);
    QCOMPARE(RowHeightRegistry<CustomTreeView>::unregisterWidgetForAdjust(pTreeView), true);

    QCOMPARE(RowHeightRegistry<CustomTableView>::GetWidgetsSet()->size(), 0);
    QCOMPARE(RowHeightRegistry<CustomTreeView>::GetWidgetsSet()->size(), 0);

    QCOMPARE(RowHeightRegistry<CustomTableView>::unregisterWidgetForAdjust(pTableView), false);
    QCOMPARE(RowHeightRegistry<CustomTreeView>::unregisterWidgetForAdjust(pTreeView), false);
  }
};

#include "RowHeightRegistryTest.moc"
REGISTER_TEST(RowHeightRegistryTest, false)
