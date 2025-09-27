#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "IntoNewPathMocker.h"
#include "BeginToExposePrivateMember.h"
#include "NavigationToolBar.h"
#include "EndToExposePrivateMember.h"
#include "DevicesDrivesActions.h"

class NavigationToolBarTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void user_path_exist() {         //
    NavigationToolBar naviToolbar; //
    const QList<QAction*> actions = naviToolbar.actions();
    QVERIFY(actions.size() > 0);
    QStringList pathsFromTooltip;
    for (auto* pAct : actions) {
      QVERIFY(pAct != nullptr);
      if (pAct->isSeparator()) { // seperator
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
    for (const QString& path: pathsFromTooltip) {
      if (path.startsWith(prepath) && path != prepath) {
        QVERIFY2(QFileInfo(prepath).isDir(), qPrintable(prepath));
      }
    }
  }

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
    QVERIFY(naviTooBar.mDevDriveTV != nullptr); // connect only but widget not create yet
    QVERIFY(naviTooBar.mDevDriveTV->widget() == nullptr);

    pFirstAct->setChecked(true);
    emit pFirstAct->toggled(true);
    QVERIFY(naviTooBar.mDevDriveTV->widget() != nullptr);

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
