#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "FontRegistry.h"
#include "EndToExposePrivateMember.h"

#include <QWidget>
#include <QAction>
template struct FontRegistry<QWidget>;
template struct FontRegistry<QAction>;

class FontRegistryTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() {
    FontRegistry<QWidget>::GetWidgetsSet(false)->clear();
    FontRegistry<QAction>::GetWidgetsSet(false)->clear();
  }

  void always_alive() {
    QWidget* pWid{nullptr};
    QAction* pAct{nullptr};
    {
      QWidget wid;
      QAction act;
      pWid = &wid;
      pAct = &act;

      const QFont beforeWidFont{wid.font()};
      const QFont beforeActFont{act.font()};

      QFont afterWidFont{beforeWidFont};
      afterWidFont.setPointSize(beforeWidFont.pointSize() + 1);
      QFont afterActFont{beforeWidFont};
      afterActFont.setPointSize(afterActFont.pointSize() + 1);

      // 手动注册, 指定长期存活
      QCOMPARE(FontRegistry<QWidget>::registerWidgetForFont(&wid, false, true), true);
      QCOMPARE(FontRegistry<QAction>::registerWidgetForFont(&act, false, true), true);
      QCOMPARE(FontRegistry<QWidget>::GetWidgetsSet(false)->size(), 1);
      QCOMPARE(FontRegistry<QAction>::GetWidgetsSet(false)->size(), 1);

      // 不允许重复注册
      QCOMPARE(FontRegistry<QWidget>::registerWidgetForFont(&wid, false, true), false);
      QCOMPARE(FontRegistry<QAction>::registerWidgetForFont(&act, false, true), false);

      // unchange
      QCOMPARE(FontRegistry<QWidget>::updateRegisteredWidgetsFont(beforeWidFont), 0);
      QCOMPARE(FontRegistry<QAction>::updateRegisteredWidgetsFont(beforeActFont), 0);
      QCOMPARE(wid.font(), beforeWidFont);
      QCOMPARE(act.font(), beforeActFont);

      // change
      QCOMPARE(FontRegistry<QWidget>::updateRegisteredWidgetsFont(afterWidFont), 1);
      QCOMPARE(FontRegistry<QAction>::updateRegisteredWidgetsFont(afterActFont), 1);
      QCOMPARE(wid.font(), afterWidFont);
      QCOMPARE(act.font(), afterActFont);
    }

    // will not removed after deleted
    QCOMPARE(FontRegistry<QWidget>::GetWidgetsSet(false)->size(), 1);
    QCOMPARE(FontRegistry<QAction>::GetWidgetsSet(false)->size(), 1);

    // unregister mannually not collapse
    QCOMPARE(FontRegistry<QWidget>::unregisterWidgetForFont(pWid), true);
    QCOMPARE(FontRegistry<QAction>::unregisterWidgetForFont(pAct), true);

    QCOMPARE(FontRegistry<QWidget>::GetWidgetsSet(false)->size(), 0);
    QCOMPARE(FontRegistry<QAction>::GetWidgetsSet(false)->size(), 0);

    QCOMPARE(FontRegistry<QWidget>::unregisterWidgetForFont(pWid), false);
    QCOMPARE(FontRegistry<QAction>::unregisterWidgetForFont(pAct), false);
  }

  void not_always_alive_ok() {
    {
      QWidget wid;
      QAction act;

      // 手动注册, 指定临时存活
      QCOMPARE(FontRegistry<QWidget>::registerWidgetForFont(&wid, false, false), true);
      QCOMPARE(FontRegistry<QAction>::registerWidgetForFont(&act, false, false), true);
      QCOMPARE(FontRegistry<QWidget>::GetWidgetsSet(false)->size(), 1);
      QCOMPARE(FontRegistry<QAction>::GetWidgetsSet(false)->size(), 1);
    }

    // will removed automatically after deleted
    QCOMPARE(FontRegistry<QWidget>::GetWidgetsSet(false)->size(), 0);
    QCOMPARE(FontRegistry<QAction>::GetWidgetsSet(false)->size(), 0);
  }
};

#include "FontRegistryTest.moc"
REGISTER_TEST(FontRegistryTest, false)
