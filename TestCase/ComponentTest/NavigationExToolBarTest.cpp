#include <QtTest/QtTest>
#include <QTestEventList>
#include <QSignalSpy>

#include "PlainTestSuite.h"
#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "NavigationExToolBar.h"
#include "EndToExposePrivateMember.h"

#include <QDir>
#include <QDirIterator>

class NavigationExToolBarTest : public PlainTestSuite {
  Q_OBJECT
private slots:
  void initTestCase() {
    Configuration().clear();
  }

  void cleanupTestCase() {
    Configuration().clear();
  }

  void dragEnter_drop_event_ok() {
    Configuration().clear();
    NavigationExToolBar naviExToolBar{"navi extra toolbar dragMove/dragEnter/drop"};
    QList<QAction*> actList1DirectFromTb = naviExToolBar.actions();
    QCOMPARE(actList1DirectFromTb.size(), 0); // empty

    const QPoint pos{naviExToolBar.geometry().center()};
    QMimeData emptyUrlsMimeData;
    emptyUrlsMimeData.setText("no urls only text");
    QCOMPARE(emptyUrlsMimeData.hasUrls(), false);

    const QString folderPath{QFileInfo(__FILE__).absolutePath()}; // 2 urls only 1 is unique
    QList<QUrl> urls{QUrl::fromLocalFile(folderPath), QUrl::fromLocalFile(folderPath)};
    QMimeData urlsMimeData;
    urlsMimeData.setText("urls exist");
    urlsMimeData.setUrls(urls);
    QCOMPARE(urlsMimeData.hasUrls(), true);

    QDragEnterEvent dragEnterEventIgnored {pos, Qt::DropAction::IgnoreAction, &emptyUrlsMimeData, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier};
    QDragEnterEvent dragEnterEventAccept {pos, Qt::DropAction::IgnoreAction, &urlsMimeData, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier};

    naviExToolBar.dragEnterEvent(&dragEnterEventIgnored); // here event may need process in parent reorder. so don't expect accept or not here

    naviExToolBar.dragEnterEvent(&dragEnterEventAccept);
    QCOMPARE(dragEnterEventIgnored.isAccepted(), true);

    QDropEvent dropEventIgnored{pos, Qt::DropAction::IgnoreAction, &emptyUrlsMimeData, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier};
    QDropEvent dropEventAccept{pos, Qt::DropAction::IgnoreAction, &urlsMimeData, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier};

    naviExToolBar.dropEvent(&dropEventIgnored); // here event may need process in parent reorder. so don't expect accept or not here
    QList<QAction*> actList2DirectFromTb = naviExToolBar.actions();
    QCOMPARE(actList2DirectFromTb.size(), 0); // empty

    // update action in toolbar and configs ok
    naviExToolBar.dropEvent(&dropEventAccept);
    QCOMPARE(dropEventAccept.isAccepted(), true);

    QList<QAction*> actList3DirectFromTb = naviExToolBar.actions();
    QCOMPARE(actList3DirectFromTb.size(), 1); // 1 added ok, 1 duplicate

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
    QCOMPARE(actList2FromAG.size(), valuesCount); // ag not cleared until parent destructed
    emit naviExToolBar.mCollectPathAgs->triggered(actList1FromAG[0]); // emit not crack down
    QList<QAction*> actList2DirectFromTb = naviExToolBar.actions();
    QCOMPARE(actList2DirectFromTb.size(), 0); // action directly from toolbar empty

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
};

#include "NavigationExToolBarTest.moc"
REGISTER_TEST(NavigationExToolBarTest, false)
