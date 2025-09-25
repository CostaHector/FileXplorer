#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "StyleSheet.h"
#include "PreferenceActions.h"
#include "EndToExposePrivateMember.h"
#include <QToolBar>
#include <QLayout>

class StyleSheetTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void SetLayoutAlightment_for_toobar_ok() {
    QAction pAct1{"1"};
    QAction pAct2{"2 long enough text"};

    QToolBar tb;
    tb.setOrientation(Qt::Orientation::Vertical);
    tb.addAction(&pAct1);
    tb.addSeparator();
    tb.addAction(&pAct2);

    SetLayoutAlightment(tb.layout(), Qt::AlignmentFlag::AlignTop);
    for (int i = 0; i < tb.layout()->count(); ++i) {
      QLayoutItem* item = tb.layout()->itemAt(i);
      QWidget* widget = item->widget();
      if (widget && widget->metaObject()->className() == QLatin1String("QToolBarSeparator")) {
        QVERIFY2(!(item->alignment() & Qt::AlignTop), "Separator should not have alignment set");
        continue;
      }
      QVERIFY2(item->alignment() & Qt::AlignTop, qPrintable(QString("Item %1 not right-aligned").arg(i)));
    }

    {  // bounder test, should not crash down
      SetLayoutAlightment(nullptr, Qt::AlignmentFlag::AlignTop);
    }
  }

  void test_ui_theme_ok() {
    QToolBar tb;
    PreferenceActions& prefInst = g_PreferenceActions();
    QActionGroup* pAgGrp = prefInst.mStyleSheetIntAction.getActionGroup();
    QAction* pStyleSheetActBackUp = pAgGrp->checkedAction();
    QVERIFY(pStyleSheetActBackUp != nullptr);

    prefInst.STYLESHEET_DARK_THEME_MOON_FOG->setChecked(true);
    emit prefInst.STYLESHEET_DARK_THEME_MOON_FOG->triggered(true); // in test project qss file not exist
    ON_SCOPE_EXIT { // recover
      pStyleSheetActBackUp->setChecked(true);
      emit pStyleSheetActBackUp->triggered(true); // in test project qss file not exist
    };
    StyleSheet::UpdateTitleBar(&tb);

    StyleSheet::setGlobalDarkMode(true);
    {  // should not crash down
      StyleSheet::setGlobalDarkMode(false);

      StyleSheet::UpdateTitleBar(nullptr);
    }
  }
};

#include "StyleSheetTest.moc"
REGISTER_TEST(StyleSheetTest, false)
