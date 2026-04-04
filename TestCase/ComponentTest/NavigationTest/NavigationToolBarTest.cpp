#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "NavigationToolBar.h"
#include "EndToExposePrivateMember.h"

#include "MemoryKey.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

bool IntoNewPathMock(QString path, bool isNew) {
  return true;
}

class NavigationToolBarTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() {  //
    GlobalMockObject::reset();
  }

  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void D2Ev_D0Ev_ok() {
    Configuration().remove(CompoVisKey::EXPAND_NAVIGATION_SIDEBAR.name);
    QCOMPARE(Configuration().value(CompoVisKey::EXPAND_NAVIGATION_SIDEBAR.name, CompoVisKey::EXPAND_NAVIGATION_SIDEBAR.v).toBool(), false);
    {  // 普通析构函数
      NavigationToolBar naviTooBar{"NaviToolbarOnExpandSidebar"};
      QCOMPARE(naviTooBar.EXPAND_SIDEBAR->isChecked(), false);
      QCOMPARE(naviTooBar.toolButtonStyle(), Qt::ToolButtonStyle::ToolButtonIconOnly);
      QCOMPARE(naviTooBar.maximumWidth(), NavigationToolBar::MAXIMUM_WIDTH_WHEN_NOT_EXPAND);

      naviTooBar.EXPAND_SIDEBAR->toggle();

      QCOMPARE(naviTooBar.EXPAND_SIDEBAR->isChecked(), true);
      QCOMPARE(naviTooBar.toolButtonStyle(), Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
      QCOMPARE(naviTooBar.maximumWidth(), NavigationToolBar::MAXIMUM_WIDTH_WHEN_EXPAND);
    }
    QVERIFY(Configuration().contains(CompoVisKey::EXPAND_NAVIGATION_SIDEBAR.name));
    QVERIFY(Configuration().value(CompoVisKey::EXPAND_NAVIGATION_SIDEBAR.name).toBool());

    {  // 删除析构函数
      std::unique_ptr<NavigationToolBar> d0ev{new NavigationToolBar};
    }
  }

  void fixed_link_all_under_user_path_ok() {  //
    NavigationToolBar naviToolbar;            //
    const QList<QAction*> actions = naviToolbar.actions();
    QVERIFY(actions.size() > 0);
    QStringList pathsFromTooltip;
    for (auto* pAct : actions) {
      QVERIFY(pAct != nullptr);
      if (pAct->isSeparator()) {  // seperator
        continue;
      }
      QVariant pathVar = pAct->data();
      if (!pathVar.canConvert<QString>()) {
        continue;
      }
      QString path = pathVar.toString();
      pathsFromTooltip.push_back(path);
    }
    // 固定的链接都是SystemPath::HOME_PATH()路径下的
    const QString prepath = SystemPath::HOME_PATH();
    QCOMPARE(QFileInfo(prepath).isDir(), true);
    QVERIFY(pathsFromTooltip.size() > 0);
    for (const QString& path : pathsFromTooltip) {
      if (path.startsWith(prepath) && path != prepath) {
        QVERIFY2(QFileInfo(prepath).isDir(), qPrintable(prepath));
      }
    }
  }

  void mDevDriveTV_ok() {
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
  }

  void onPathActionTriggeredNavi_ok() {
    NavigationToolBar naviTooBar{"Navi toolbar"};
    QVERIFY(naviTooBar.m_pathActionGroups != nullptr);
    QList<QAction*> fixedPathActs = naviTooBar.m_pathActionGroups->actions();
    QVERIFY(fixedPathActs.size() > 0);
    QAction* frontAct = fixedPathActs.front();
    QVERIFY(frontAct != nullptr);
    QVERIFY(frontAct->text() != "");
    QVariant pathVar = frontAct->data();
    QVERIFY(pathVar.canConvert<QString>());
    QString path = pathVar.toString();

    MOCKER(::IntoNewPathMock)
        .expects(exactly(3))       //
        .with(eq(path), eq(true))  //
        .will(returnValue(true))   //
        .then(returnValue(true));

    // will not crash down
    QCOMPARE(naviTooBar.onPathActionTriggeredNavi(nullptr), false);

    QVERIFY(!naviTooBar.m_IntoNewPathNavi);
    emit naviTooBar.m_pathActionGroups->triggered(frontAct);
    QCOMPARE(naviTooBar.onPathActionTriggeredNavi(frontAct), false);
    QCOMPARE(naviTooBar.onAccessNewPathRequest(path, true), false);

    naviTooBar.BindIntoNewPathNavi(::IntoNewPathMock);
    QVERIFY(naviTooBar.m_IntoNewPathNavi);
    emit naviTooBar.m_pathActionGroups->triggered(frontAct);
    QCOMPARE(naviTooBar.onPathActionTriggeredNavi(frontAct), true);
    QCOMPARE(naviTooBar.onAccessNewPathRequest(path, true), true);
  }
};

#include "NavigationToolBarTest.moc"
REGISTER_TEST(NavigationToolBarTest, false)
