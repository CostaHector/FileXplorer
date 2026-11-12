#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "PopupWidgetManager.h"
#include "EndToExposePrivateMember.h"

#include "StyleSheet.h"
#include "Configuration.h"
#include "DevicesDrivesTV.h"
#include "DuplicatesImagesFinder.h"
#include "DuplicateVideosFinder.h"
#include "TorrentsManagerWidget.h"
#include "ConfigsMgr.h"
#include "Archiver.h"
#ifdef PASSVAULT_ENABLED
#include "PasswordBook.h"
#endif
#include "ResourceMonitorPanel.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class PopupWidgetManagerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  template <typename WIDGET_TYPE>
  void PopupWidgetManagerTestTemplate(const QString& configsBaseName, bool isCheckable = true) {
    {
      auto* pDynCreate = new (std::nothrow) PopupWidgetManager<WIDGET_TYPE>(nullptr, nullptr);
      if (pDynCreate != nullptr) {
        delete pDynCreate;
        pDynCreate = nullptr;
      }
    }

    {
      {  // invalid input. shoud not crash down
        QWidget parentWidget;
        QAction randomAct{configsBaseName, &parentWidget};
        randomAct.setCheckable(isCheckable);
        PopupWidgetManager<WIDGET_TYPE> actionNullptrPopupMgr{nullptr, &parentWidget};
        PopupWidgetManager<WIDGET_TYPE> parentNullptrPopupMgr{&randomAct, nullptr};
        PopupWidgetManager<WIDGET_TYPE> emptyCfgKeyPopupMgr{&randomAct, &parentWidget};
        emit randomAct.triggered(true);
        QVERIFY(!actionNullptrPopupMgr.isVisible());
        QVERIFY(!parentNullptrPopupMgr.isVisible());
        QVERIFY(!emptyCfgKeyPopupMgr.isVisible());
        emit randomAct.triggered(false);
      }

      auto fCloseCallback = [configsBaseName]() { LOG_D("Widget[%s] closing now", qPrintable(configsBaseName)); };

      QWidget parentWidget;
      QAction widAct{configsBaseName, &parentWidget};
      widAct.setCheckable(isCheckable);

      PopupWidgetManager<WIDGET_TYPE> wid{&widAct, &parentWidget};
      wid.setOnCloseCallback(fCloseCallback);
      wid.setWidgetCreator(nullptr);  // we usually don't need this. Default constructor is enough now
      QVERIFY(wid.widget() == nullptr);
      QCOMPARE(wid.isVisible(), false);
      QCOMPARE(widAct.isChecked(), false);

      // checked
      widAct.setChecked(true);
      emit widAct.triggered(true);
      QVERIFY(wid.widget() != nullptr);
      QCOMPARE(wid.isVisible(), true);

      // unchecked
      widAct.setChecked(false);
      emit widAct.triggered(false);
      QVERIFY(wid.widget() != nullptr);
      QCOMPARE(wid.isVisible(), false);

      // close
      wid.widget()->close();
      QCOMPARE(widAct.isChecked(), false);
      QCOMPARE(wid.isVisible(), false);
    }
  }

 private slots:
  void initTestCase() {
    GlobalMockObject::reset();
    Configuration().clear();
  }

  void cleanupTestCase() {
    GlobalMockObject::verify();
    Configuration().clear();
  }

  void checkable_popup_widget_geometry_state_ok() {
    PopupWidgetManagerTestTemplate<DevicesDrivesTV>("DevicesDrivesTV", true);
    {
      // showEvent should called at least once
      MOCKER(StyleSheet::UpdateTitleBar).expects(atLeast(1));
      PopupWidgetManagerTestTemplate<DuplicateVideosFinder>("DuplicateVideosFinder", true);
      // closeEvent should called at least once
      QCOMPARE(Configuration().contains("DuplicateVideosFinder/State"), true);
      QCOMPARE(Configuration().contains("DuplicateVideosFinder/LRSplitterState"), true);
    }
    PopupWidgetManagerTestTemplate<DuplicatesImagesFinder>("RedundantImageFinder", true);
    PopupWidgetManagerTestTemplate<TorrentsManagerWidget>("TorrentsManagerWidget", true);
    PopupWidgetManagerTestTemplate<ConfigsMgr>("ConfigsTable", true);
    PopupWidgetManagerTestTemplate<Archiver>("Archiver", true);
    #ifdef PASSVAULT_ENABLED
    PopupWidgetManagerTestTemplate<PasswordBook>("PasswordBook", true);
    #endif
    PopupWidgetManagerTestTemplate<ResourceMonitorPanel>("ResourceMonitorPanel", true);
  }

  void not_checkable_popup_widget_geometry_state_ok() {  //
    PopupWidgetManagerTestTemplate<QWidget>("QWidget", false);
  }
};

#include "PopupWidgetManagerTest.moc"
REGISTER_TEST(PopupWidgetManagerTest, false)
