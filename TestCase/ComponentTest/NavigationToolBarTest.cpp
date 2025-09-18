#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "IntoNewPathMocker.h"
#include "BeginToExposePrivateMember.h"
#include "NavigationToolBar.h"
#include "EndToExposePrivateMember.h"
#include "DevicesDrivesActions.h"


void triggerFirstLevelActions(QToolBar& toolbar) {
  QList<QAction*> actions = toolbar.actions();

}

class NavigationToolBarTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void uers_fixed_folder_actions_into_new_path_correct() {
    IntoNewPathParms actualParams;
    IntoNewPathMocker mocker{&actualParams};
    NavigationExToolBar::BindIntoNewPath(mocker);

    NavigationToolBar naviTooBar{"Navi toolbar"};
    QList<QAction*> plainActLsts = naviTooBar.actions();
    QVERIFY(!plainActLsts.isEmpty());
    QAction* pFirstAct = plainActLsts[0];

    QCOMPARE(pFirstAct, DevicesDrivesActions::Inst().DEVICES_AND_DRIVES);

    // Special Action: will not call Into New Path
    pFirstAct->setChecked(true);
    emit naviTooBar.actionTriggered(pFirstAct);
    QCOMPARE(actualParams.m_newPath, "");

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
      QVERIFY(!actualParams.m_newPath.isEmpty());
      QVERIFY(QFile::exists(actualParams.m_newPath));
    }
  }
};

#include "NavigationToolBarTest.moc"
REGISTER_TEST(NavigationToolBarTest, false)
