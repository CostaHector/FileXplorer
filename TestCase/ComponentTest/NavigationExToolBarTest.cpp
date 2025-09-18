#include <QtTest/QtTest>
#include <QTestEventList>
#include <QSignalSpy>

#include "PlainTestSuite.h"
#include "IntoNewPathMocker.h"
#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "NavigationExToolBar.h"
#include "EndToExposePrivateMember.h"
#include "DraggableToolButton.h"
#include "QtProcessGuard.h"

#include <QMenu>

class NavigationExToolBarTest : public PlainTestSuite {
  Q_OBJECT
 private slots:
  void initTestCase() { Configuration().clear(); }

  void cleanupTestCase() { Configuration().clear(); }

  void clickToolButtonDirectly_willEmit_ToolBarActionTriggered_ok() {
    QAction act{"Hello"};

    QToolBar tb;
    tb.addAction(&act);
    connect(&tb, &QToolBar::actionTriggered, this, [](const QAction* pAct) { LOG_D("actionTriggered: %s", qPrintable(pAct->text())); });
    tb.show();

    QToolButton* button1 = qobject_cast<QToolButton*>(tb.widgetForAction(&act));
    QVERIFY(button1 != nullptr);

    QSignalSpy spyToolBarActionTriggered(&tb, &QToolBar::actionTriggered);
    QTest::mouseClick(button1, Qt::LeftButton, Qt::KeyboardModifier::NoModifier, {}, 50);
    QTRY_COMPARE(spyToolBarActionTriggered.count(), 1);
  }

  /*
   * WARNING: Avoid simulating right-click events on QToolBar using mouseClick!
   *
   * FORBIDDEN:
   *   QTest::mouseClick(&tb, Qt::RightButton, ...);
   *
   * REASON:
   *   QToolBar has known limitations in processing simulated right-click events.
   *   Even with active windows and correct positioning, events may be intercepted
   *   by internal handling mechanisms.
   *
   * RECOMMENDED SOLUTION:
   *   Use QContextMenuEvent to directly trigger context menu requests:
   *     QContextMenuEvent event(QContextMenuEvent::Mouse, localPos, tb.mapToGlobal(localPos));
   *     QApplication::sendEvent(&tb, &event);
   *     Or tb.contextMenuEvent(&event);
   *
   * This approach bypasses unreliable mouse event simulation and directly triggers the toolbar's context menu handling logic.
   */

#ifdef __linux__
  void breakpoint_at_menu_popup_will_block_system() {
    // QtProcessGuard guard{10};
    class SomeToolBar : public QToolBar {
     public:
      SomeToolBar() {
        mMenu = new QMenu{"Menu", this};

        pAct = new QAction{"act1", this};
        mMenu->addSeparator();
        mMenu->addAction(pAct);
        mMenu->addSeparator();
      }
      QAction* mRightClickAtAction{nullptr};
      QAction* pAct{nullptr};
      QMenu* mMenu{nullptr};
    };

    SomeToolBar tb;
    QVERIFY(tb.mMenu != nullptr);
    // NEVER CALL menu.popup() when breakpoint enabled, it will block system.
    QPoint center = tb.geometry().center();
    tb.mMenu->popup(center);
  }
#endif

  void rightClickToolButtonDirectly_will_Menu_ok() {
    QToolBar tb;
    tb.setContextMenuPolicy(Qt::CustomContextMenu);
    int menuRequestTime{0};
    connect(&tb, &QToolBar::customContextMenuRequested, this, [&menuRequestTime]() -> void {
      ++menuRequestTime;
      // NEVER call menu->popup();
    });
    const QPoint tbCenterPnt = tb.geometry().center();
    QSignalSpy spyToolBarActionTriggered(&tb, &QToolBar::customContextMenuRequested);
    QContextMenuEvent event(QContextMenuEvent::Mouse, tbCenterPnt, tb.mapToGlobal(tbCenterPnt));
    QApplication::sendEvent(&tb, &event);
    QCOMPARE(menuRequestTime, 1);
    QCOMPARE(spyToolBarActionTriggered.count(), 1);
  }

  void menu_popup_ok() {
    // QtProcessGuard guard{10};
    Configuration().clear();

    // precondition: 2 items in configuraion
    QMap<QString, QString> folderName2AbsPath;
    QStringList folderNames{"1", "2"};
    QStringList absPaths{"home/1", "home/2"};
    QCOMPARE(folderNames.size(), absPaths.size());
    const int valuesCount = folderNames.size();
    Configuration().beginWriteArray(NavigationExToolBar::EXTRA_NAVI_DICT, valuesCount);
    for (int i = 0; i < valuesCount; ++i) {
      Configuration().setArrayIndex(i);
      Configuration().setValue(NavigationExToolBar::EXTRA_NAVI_DICT_KEY, folderNames[i]);
      Configuration().setValue(NavigationExToolBar::EXTRA_NAVI_DICT_VALUE, absPaths[i]);
      folderName2AbsPath[folderNames[i]] = absPaths[i];
    }
    Configuration().endArray();

    NavigationExToolBar naviExToolBar{"navi extra toolbar unpin this"};
    naviExToolBar.show();
    naviExToolBar.raise();
    naviExToolBar.activateWindow();
    QCOMPARE(QTest::qWaitForWindowActive(&naviExToolBar), true);

    QList<QAction*> actList1DirectFromTb = naviExToolBar.actions();
    QList<QAction*> actList1FromAG = naviExToolBar.mCollectPathAgs->actions();
    QCOMPARE(actList1DirectFromTb.size(), valuesCount);
    QCOMPARE(actList1FromAG.size(), valuesCount);
    QAction* pAct1 = actList1DirectFromTb[0];
    QVERIFY(pAct1 != nullptr);
    QAction* pAct2 = actList1DirectFromTb[1];
    QVERIFY(pAct2 != nullptr);

    // 0. no right click happend, cannot unpin
    QVERIFY(naviExToolBar.mRightClickAtAction == nullptr);
    QVERIFY(!naviExToolBar.UnpinThis());

    // 1. unpin 1st action, mRightClickAtAction set ok, unpin succeed
    DraggableToolButton* button1 = dynamic_cast<DraggableToolButton*>(naviExToolBar.widgetForAction(pAct1));
    QVERIFY(button1 != nullptr);
    QPoint btn1CntrPos = button1->geometry().center();
    QVERIFY(!btn1CntrPos.isNull());
    QContextMenuEvent rightClickPosAtAct1Event(QContextMenuEvent::Mouse, btn1CntrPos, naviExToolBar.mapToGlobal(btn1CntrPos));
    naviExToolBar.contextMenuEvent(&rightClickPosAtAct1Event);
    QCOMPARE(naviExToolBar.mRightClickAtAction, pAct1);

    QVERIFY(naviExToolBar.UnpinThis());
    QVERIFY(naviExToolBar.mRightClickAtAction == nullptr);
    QVERIFY(!naviExToolBar.UnpinThis());  // cannot unpin, mRightClickAtAction is nullptr

    // 2. unpin 2nd action, mRightClickAtAction set ok, unpin succeed
    DraggableToolButton* button2 = dynamic_cast<DraggableToolButton*>(naviExToolBar.widgetForAction(pAct2));
    QVERIFY(button2 != nullptr);
    QPoint btn2CntrPos = button2->geometry().center();
    QVERIFY(!btn2CntrPos.isNull());
    // QTest::qWait(1000); QCOMPARE(btn2CntrPos, btn1CntrPos); wait until remove works
    QContextMenuEvent rightClickPosAtAct2Event(QContextMenuEvent::Mouse, btn2CntrPos, naviExToolBar.mapToGlobal(btn2CntrPos));
    naviExToolBar.contextMenuEvent(&rightClickPosAtAct2Event);
    QCOMPARE(naviExToolBar.mRightClickAtAction, pAct2);
    QVERIFY(naviExToolBar.UnpinThis());
    QVERIFY(naviExToolBar.mRightClickAtAction == nullptr);
    QVERIFY(!naviExToolBar.UnpinThis());  // cannot unpin, mRightClickAtAction is nullptr
  }

  void dragEnter_drop_event_ok() {
    Configuration().clear();
    NavigationExToolBar naviExToolBar{"navi extra toolbar dragMove/dragEnter/drop"};
    QList<QAction*> actList1DirectFromTb = naviExToolBar.actions();
    QCOMPARE(actList1DirectFromTb.size(), 0);  // empty

    const QPoint pos{naviExToolBar.geometry().center()};
    QMimeData emptyUrlsMimeData;
    emptyUrlsMimeData.setText("no urls only text");
    QCOMPARE(emptyUrlsMimeData.hasUrls(), false);

    const QString folderPath{QFileInfo(__FILE__).absolutePath()};  // 2 urls only 1 is unique
    QList<QUrl> urls{QUrl::fromLocalFile(folderPath), QUrl::fromLocalFile(folderPath)};
    QMimeData urlsMimeData;
    urlsMimeData.setText("urls exist");
    urlsMimeData.setUrls(urls);
    QCOMPARE(urlsMimeData.hasUrls(), true);

    QDragEnterEvent dragEnterEventIgnored{pos, Qt::DropAction::IgnoreAction, &emptyUrlsMimeData, Qt::MouseButton::LeftButton,
                                          Qt::KeyboardModifier::NoModifier};
    QDragEnterEvent dragEnterEventAccept{pos, Qt::DropAction::IgnoreAction, &urlsMimeData, Qt::MouseButton::LeftButton,
                                         Qt::KeyboardModifier::NoModifier};

    naviExToolBar.dragEnterEvent(&dragEnterEventIgnored);  // here event may need process in parent reorder. so don't expect accept or not here

    naviExToolBar.dragEnterEvent(&dragEnterEventAccept);
    QCOMPARE(dragEnterEventIgnored.isAccepted(), true);

    QDropEvent dropEventIgnored{pos, Qt::DropAction::IgnoreAction, &emptyUrlsMimeData, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier};
    QDropEvent dropEventAccept{pos, Qt::DropAction::IgnoreAction, &urlsMimeData, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier};

    naviExToolBar.dropEvent(&dropEventIgnored);  // here event may need process in parent reorder. so don't expect accept or not here
    QList<QAction*> actList2DirectFromTb = naviExToolBar.actions();
    QCOMPARE(actList2DirectFromTb.size(), 0);  // empty

    // update action in toolbar and configs ok
    naviExToolBar.dropEvent(&dropEventAccept);
    QCOMPARE(dropEventAccept.isAccepted(), true);

    QList<QAction*> actList3DirectFromTb = naviExToolBar.actions();
    QCOMPARE(actList3DirectFromTb.size(), 1);  // 1 added ok, 1 duplicate

    int lnkCntInCfg = Configuration().beginReadArray(NavigationExToolBar::EXTRA_NAVI_DICT);
    Configuration().endArray();
    QCOMPARE(lnkCntInCfg, 1);  // 1 added ok, 1 duplicate
  }

  void unpin_ok() {
    // precondition: 3 items in configuraion
    QMap<QString, QString> folderName2AbsPath;
    QStringList folderNames{"1", "2", "3"};
    QStringList absPaths{"home/1", "home/2", "home/3"};
    QCOMPARE(folderNames.size(), absPaths.size());
    const int valuesCount = folderNames.size();
    Configuration().beginWriteArray(NavigationExToolBar::EXTRA_NAVI_DICT, valuesCount);
    for (int i = 0; i < valuesCount; ++i) {
      Configuration().setArrayIndex(i);
      Configuration().setValue(NavigationExToolBar::EXTRA_NAVI_DICT_KEY, folderNames[i]);
      Configuration().setValue(NavigationExToolBar::EXTRA_NAVI_DICT_VALUE, absPaths[i]);
      folderName2AbsPath[folderNames[i]] = absPaths[i];
    }
    Configuration().endArray();
    int lnkCnt1 = Configuration().beginReadArray(NavigationExToolBar::EXTRA_NAVI_DICT);
    Configuration().endArray();
    QCOMPARE(lnkCnt1, valuesCount);
    QCOMPARE(folderName2AbsPath.size(), valuesCount);

    // initial lnk count: 3
    NavigationExToolBar naviExToolBar{"navi extra toolbar"};
    QList<QAction*> actList1DirectFromTb = naviExToolBar.actions();
    QList<QAction*> actList1FromAG = naviExToolBar.mCollectPathAgs->actions();
    QCOMPARE(actList1DirectFromTb.size(), valuesCount);
    QCOMPARE(actList1FromAG.size(), valuesCount);

    // unpin all
    naviExToolBar.UnpinAll();
    QList<QAction*> actList2FromAG = naviExToolBar.mCollectPathAgs->actions();
    QCOMPARE(actList2FromAG.size(), valuesCount);                      // ag not cleared until parent destructed
    emit naviExToolBar.mCollectPathAgs->triggered(actList1FromAG[0]);  // emit not crack down
    QList<QAction*> actList2DirectFromTb = naviExToolBar.actions();
    QCOMPARE(actList2DirectFromTb.size(), 0);  // action directly from toolbar empty

    // configuration get updated
    int lnkCnt2 = Configuration().beginReadArray(NavigationExToolBar::EXTRA_NAVI_DICT);
    Configuration().endArray();
    QCOMPARE(lnkCnt2, 0);

    // AppendExtraActions only used in read setting, will not update configuration
    naviExToolBar.AppendExtraActions(folderName2AbsPath);
    QList<QAction*> actList3DirectFromTb = naviExToolBar.actions();
    QCOMPARE(actList3DirectFromTb.size(), folderName2AbsPath.size());
    int lnkCnt3 = Configuration().beginReadArray(NavigationExToolBar::EXTRA_NAVI_DICT);
    Configuration().endArray();
    QCOMPARE(lnkCnt3, 0);
  }

  void action_triggered_call_into_new_path_ok() {  //
    NavigationExToolBar naviExToolBar{"navi extra toolbar accpet enter urls exists"};
    QCOMPARE(naviExToolBar.m_IntoNewPath, nullptr);
    naviExToolBar.AppendExtraActions({{"1", TESTCASE_ROOT_PATH}, {"2", __FILE__}});

    IntoNewPathParms actualParams;
    IntoNewPathMocker mocker{&actualParams};
    NavigationExToolBar::BindIntoNewPath(mocker);

    QVERIFY(naviExToolBar.mCollectPathAgs != nullptr);
    QList<QAction*> lsts = naviExToolBar.mCollectPathAgs->actions();
    QCOMPARE(lsts.size(), 2);
    naviExToolBar.onPathActionTriggered(lsts.front());
    QCOMPARE(actualParams, (IntoNewPathParms{TESTCASE_ROOT_PATH, true, true}));

    naviExToolBar.onPathActionTriggered(lsts.back());
    QCOMPARE(actualParams, (IntoNewPathParms{__FILE__, true, true}));
  }
};

#include "NavigationExToolBarTest.moc"
REGISTER_TEST(NavigationExToolBarTest, true)
