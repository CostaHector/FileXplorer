#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "NavigationToolBar.h"
#include "EndToExposePrivateMember.h"

#include "MemoryKey.h"

class NavigationToolBarTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void user_path_exist() {          //
    NavigationToolBar naviToolbar;  //
    const QList<QAction*> actions = naviToolbar.actions();
    QVERIFY(actions.size() > 0);
    QStringList pathsFromTooltip;
    for (auto* pAct : actions) {
      QVERIFY(pAct != nullptr);
      if (pAct->isSeparator()) {  // seperator
        continue;
      }
      QString path = pAct->toolTip();
      if (path.isEmpty()) {
        continue;
      }
      pathsFromTooltip.push_back(path);
    }

    const QString prepath = SystemPath::HOME_PATH();
    QCOMPARE(QFileInfo(prepath).isDir(), true);
    QVERIFY(pathsFromTooltip.size() > 0);
    for (const QString& path : pathsFromTooltip) {
      if (path.startsWith(prepath) && path != prepath) {
        QVERIFY2(QFileInfo(prepath).isDir(), qPrintable(prepath));
      }
    }
  }

  void uers_fixed_folder_actions_into_new_path_correct() {
    NavigationToolBar naviTooBar{"Navi toolbar"};
    QList<QAction*> plainActLsts = naviTooBar.actions();
    QVERIFY(!plainActLsts.isEmpty());

    // Special Action: will not call Into New Path
    QVERIFY(naviTooBar.mDevDriveTV != nullptr);  // connect only but widget not create yet
    QVERIFY(naviTooBar.mDevDriveTV->widget() == nullptr);
    QCOMPARE(naviTooBar.DEVICES_AND_DRIVES->isChecked(), false);

    naviTooBar.DEVICES_AND_DRIVES->toggle();
    QCOMPARE(naviTooBar.DEVICES_AND_DRIVES->isChecked(), true);
    QVERIFY(naviTooBar.mDevDriveTV->widget() != nullptr);

    QCOMPARE(naviTooBar.mDevDriveTV->isVisible(), true);
    naviTooBar.DEVICES_AND_DRIVES->toggle();
    QVERIFY(naviTooBar.mDevDriveTV != nullptr);
    QCOMPARE(naviTooBar.mDevDriveTV->isVisible(), false);
    QCOMPARE(naviTooBar.DEVICES_AND_DRIVES->isChecked(), false);

    QList<QAction*> otherActLst = plainActLsts.mid(1);
    for (QAction* action : otherActLst) {
      if (action->isSeparator()) {
        continue;
      }

      // built-in: QWidgetAction
      if (QWidget* widget = naviTooBar.widgetForAction(action)) {
        if (qobject_cast<QToolBar*>(widget)) {
          // built-in toolbar skip
          continue;
        }
      }
      if (!action->isEnabled() || !action->isVisible()) {
        continue;
      }
    }
  }

  void onExpandSidebar_ok() {
    Configuration().remove(MemoryKey::EXPAND_QUICK_NAVIGATION_TOOL_BAR.name);
    QCOMPARE(Configuration().value(MemoryKey::EXPAND_QUICK_NAVIGATION_TOOL_BAR.name, MemoryKey::EXPAND_QUICK_NAVIGATION_TOOL_BAR.v).toBool(), false);
    {
      NavigationToolBar naviTooBar{"NaviToolbarOnExpandSidebar"};
      QCOMPARE(naviTooBar.EXPAND_SIDEBAR->isChecked(), false);
      QCOMPARE(naviTooBar.toolButtonStyle(), Qt::ToolButtonStyle::ToolButtonIconOnly);
      QCOMPARE(naviTooBar.maximumWidth(), NavigationToolBar::MAXIMUM_WIDTH_WHEN_NOT_EXPAND);

      naviTooBar.EXPAND_SIDEBAR->toggle();

      QCOMPARE(naviTooBar.EXPAND_SIDEBAR->isChecked(), true);
      QCOMPARE(naviTooBar.toolButtonStyle(), Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
      QCOMPARE(naviTooBar.maximumWidth(), NavigationToolBar::MAXIMUM_WIDTH_WHEN_EXPAND);
    }
    QVERIFY(Configuration().contains(MemoryKey::EXPAND_QUICK_NAVIGATION_TOOL_BAR.name));
    QVERIFY(Configuration().value(MemoryKey::EXPAND_QUICK_NAVIGATION_TOOL_BAR.name).toBool());
  }
};

#include "NavigationToolBarTest.moc"
REGISTER_TEST(NavigationToolBarTest, false)
