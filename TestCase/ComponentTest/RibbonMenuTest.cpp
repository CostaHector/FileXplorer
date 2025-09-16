#include <QtTest/QtTest>
#include <QTestEventList>

#include "PlainTestSuite.h"
#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "RibbonMenu.h"
#include "EndToExposePrivateMember.h"
#include <QTabBar>

class RibbonMenuTest : public PlainTestSuite {
  Q_OBJECT
 private slots:
  void test_ribbon_menu_basic_behavior_ok() {
    // precondition: default table widget=0, expand=true
    Configuration().setValue(MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.name, 0);
    Configuration().setValue(MemoryKey::EXPAND_OFFICE_STYLE_MENUBAR.name, true);

    RibbonMenu rm;
    QVERIFY(rm.count() > 0);
    // when initialized index=0, expand=true;
    QCOMPARE(rm.currentIndex(), 0);
    QVERIFY(rm._EXPAND_RIBBONS != nullptr);
    QCOMPARE(rm._EXPAND_RIBBONS->isChecked(), true);

    // switch view toolbar
    rm.whenViewTypeChanged(ViewTypeTool::ViewType::MOVIE);
    const int movieIndexInTabBar = rm.currentIndex();
    emit rm.currentChanged(movieIndexInTabBar);
    // memory updated
    Configuration().setValue(MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.name, movieIndexInTabBar);
    // tab widget toggled
    auto* pTabBar = rm.tabBar();
    QVERIFY(pTabBar != nullptr);
    QVERIFY(pTabBar->tabText(movieIndexInTabBar).contains("movie", Qt::CaseInsensitive));

    // hide widget under tab widget
    rm._EXPAND_RIBBONS->setChecked(false);
    emit rm._EXPAND_RIBBONS->toggled(false);
    QCOMPARE(rm.maximumHeight(), pTabBar->height());

    // show widget under tab widget
    rm._EXPAND_RIBBONS->setChecked(true);
    emit rm._EXPAND_RIBBONS->toggled(true);
    QVERIFY(rm.maximumHeight() > pTabBar->height());
  }
};

#include "RibbonMenuTest.moc"
REGISTER_TEST(RibbonMenuTest, false)
