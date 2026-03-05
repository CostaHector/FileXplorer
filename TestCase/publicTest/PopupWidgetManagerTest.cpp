#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "MemoryKey.h"

#include "BeginToExposePrivateMember.h"
#include "PopupWidgetManager.h"
#include "EndToExposePrivateMember.h"

#include "DevicesDrivesTV.h"
#include "RedundantImageFinder.h"
#include "DuplicateVideosFinder.h"
#include "TorrentsManagerWidget.h"
#include "ConfigsTable.h"
#include "Archiver.h"
#include "PasswordBook.h"

class PopupWidgetManagerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  template <typename WIDGET_TYPE>
  void PopupWidgetManagerTestTemplate(const QString& configsBaseName, bool isCheckable = true) {
    {
      auto* pDynCreate = new (std::nothrow) PopupWidgetManager<WIDGET_TYPE>(nullptr, nullptr, "");
      if (pDynCreate != nullptr) {
        delete pDynCreate;
        pDynCreate = nullptr;
      }
    }

    const QString widgetGeometryKey = configsBaseName + "Geometry";
    QVERIFY(!Configuration().contains(widgetGeometryKey));
    {
      {  // invalid input. shoud not crash down
        QWidget parentWidget;
        QAction randomAct{configsBaseName, &parentWidget};
        randomAct.setCheckable(isCheckable);
        PopupWidgetManager<WIDGET_TYPE> actionNullptrPopupMgr{nullptr, &parentWidget, "actionNullptrPopupMgr"};
        PopupWidgetManager<WIDGET_TYPE> parentNullptrPopupMgr{&randomAct, nullptr, "parentNullptrPopupMgr"};
        PopupWidgetManager<WIDGET_TYPE> emptyCfgKeyPopupMgr{&randomAct, &parentWidget, ""};
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

      PopupWidgetManager<WIDGET_TYPE> wid{&widAct, &parentWidget, widgetGeometryKey};
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
    QVERIFY(Configuration().contains(widgetGeometryKey));
  }

 private slots:
  void initTestCase() { Configuration().clear(); }

  void cleanupTestCase() { Configuration().clear(); }

  void checkable_popup_widget_geometry_state_ok() {
    PopupWidgetManagerTestTemplate<DevicesDrivesTV>("DevicesDrivesTV", true);
    PopupWidgetManagerTestTemplate<DuplicateVideosFinder>("DuplicateVideosFinder", true);
    PopupWidgetManagerTestTemplate<RedundantImageFinder>("RedundantImageFinder", true);
    PopupWidgetManagerTestTemplate<TorrentsManagerWidget>("TorrentsManagerWidget", true);
    PopupWidgetManagerTestTemplate<ConfigsTable>("ConfigsTable", true);
    PopupWidgetManagerTestTemplate<Archiver>("Archiver", true);
    PopupWidgetManagerTestTemplate<PasswordBook>("PasswordBook", true);
  }

  void not_checkable_popup_widget_geometry_state_ok() {  //
    PopupWidgetManagerTestTemplate<QWidget>("QWidget", false);
  }
};

#include "PopupWidgetManagerTest.moc"
REGISTER_TEST(PopupWidgetManagerTest, false)
