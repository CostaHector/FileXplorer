#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "IntoNewPathMocker.h"
#include "BeginToExposePrivateMember.h"
#include "NavigationToolBar.h"
#include "EndToExposePrivateMember.h"
#include "DevicesDrivesActions.h"
#include "DevicesDrivesTV.h"
#include "Logger.h"

class NavigationToolBarTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void uers_fixed_folder_actions_into_new_path_correct() {
    IntoNewPathParms::GetInst().clear();
    IntoNewPathMocker mocker;
    NavigationExToolBar::BindIntoNewPathNavi(mocker);
    NavigationToolBar naviTooBar{"Navi toolbar"};
    QList<QAction*> plainActLsts = naviTooBar.actions();
    QVERIFY(!plainActLsts.isEmpty());
    QAction* pFirstAct = plainActLsts[0];
    QCOMPARE(pFirstAct, DevicesDrivesActions::Inst().DEVICES_AND_DRIVES);

    // Special Action: will not call Into New Path
    QVERIFY(naviTooBar.mDevDriveTV == nullptr);
    pFirstAct->setChecked(true);
    emit pFirstAct->toggled(true);
    QVERIFY(naviTooBar.mDevDriveTV != nullptr);
    QCOMPARE(IntoNewPathParms::GetInst().m_newPath, "");
    QCOMPARE(naviTooBar.mDevDriveTV->isVisible(), true);
    pFirstAct->setChecked(false);
    emit pFirstAct->toggled(false);
    QVERIFY(naviTooBar.mDevDriveTV != nullptr);
    QCOMPARE(naviTooBar.mDevDriveTV->isVisible(), false);

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
      emit naviTooBar.actionTriggered(action);
      QVERIFY(!IntoNewPathParms::GetInst().m_newPath.isEmpty());
      QVERIFY(QFile::exists(IntoNewPathParms::GetInst().m_newPath));
    }
  }
};

#include "NavigationToolBarTest.moc"
REGISTER_TEST(NavigationToolBarTest, false)
