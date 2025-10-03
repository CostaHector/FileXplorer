#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "CustomListView.h"
#include "EndToExposePrivateMember.h"

#include "AddressBarActions.h"
#include "ViewActions.h"
#include "StyleSheet.h"
#include "MouseKeyboardEventHelper.h"
using namespace MouseKeyboardEventHelper;

class CustomListViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {
    QVERIFY(QMetaType::type("QSize") != 0);
    Configuration().clear();
  }

  void cleanupTestCase() { Configuration().clear(); }

  void test_contextMenuEvent() {
    const QString keyMemoryName = "CustomListViewTest";
    Configuration().setValue(keyMemoryName + "_ICON_SIZE_INDEX", 5);
    CustomListView view(keyMemoryName);

    QMenu menu;
    view.BindMenu(nullptr);
    view.BindMenu(&menu);
    view.BindMenu(&menu);

    view.contextMenuEvent(nullptr);

    const QPoint posCenter = view.geometry().center();
    QContextMenuEvent rghContextEvent(QContextMenuEvent::Mouse, posCenter, view.mapToGlobal(posCenter));
    view.contextMenuEvent(&rghContextEvent);
    QCOMPARE(rghContextEvent.isAccepted(), true);

    {
      auto& addressInst = g_addressBarActions();
      auto& viewInst = g_viewActions();

      QSignalSpy backAddressSpy(addressInst._BACK_TO, &QAction::triggered);
      QSignalSpy forwardAddressSpy(addressInst._FORWARD_TO, &QAction::triggered);
      QSignalSpy backViewSpy(viewInst._VIEW_BACK_TO, &QAction::triggered);
      QSignalSpy forwardViewSpy(viewInst._VIEW_FORWARD_TO, &QAction::triggered);

      {  // accepted events
        QVERIFY(SendMousePressEvent<CustomListView>(view, Qt::BackButton, Qt::NoModifier));
        QCOMPARE(backAddressSpy.count(), 1);

        QVERIFY(SendMousePressEvent<CustomListView>(view, Qt::ForwardButton, Qt::NoModifier));
        QCOMPARE(forwardAddressSpy.count(), 1);

        QVERIFY(SendMousePressEvent<CustomListView>(view, Qt::BackButton, Qt::ControlModifier));
        QCOMPARE(backViewSpy.count(), 1);

        QVERIFY(SendMousePressEvent<CustomListView>(view, Qt::ForwardButton, Qt::ControlModifier));
        QCOMPARE(forwardViewSpy.count(), 1);
      }

      // Alt+back: nothing happen
      {
        SendMousePressEvent<CustomListView>(view, Qt::BackButton, Qt::AltModifier);
        QCOMPARE(backViewSpy.count(), 1);
        QCOMPARE(backAddressSpy.count(), 1);
      }

      // left click: nothing happen
      {
        SendMousePressEvent<CustomListView>(view, Qt::LeftButton, Qt::NoModifier);
        QCOMPARE(backAddressSpy.count(), 1);
        QCOMPARE(forwardViewSpy.count(), 1);
      }

      // all signal params ok
      QVERIFY(backAddressSpy.count() > 0);
      QVERIFY(forwardAddressSpy.count() > 0);
      QVERIFY(backViewSpy.count() > 0);
      QVERIFY(forwardViewSpy.count() > 0);

      QCOMPARE(backAddressSpy.back()[0].toBool(), false);
      QCOMPARE(forwardAddressSpy.back()[0].toBool(), false);
      QCOMPARE(backViewSpy.back()[0].toBool(), false);
      QCOMPARE(forwardViewSpy.back()[0].toBool(), false);
    }
  }

  void test_wheelEvent_zoom() {
    const QString keyMemoryName = "CustomListViewTest";
    Configuration().setValue(keyMemoryName + "_ICON_SIZE_INDEX", 5);
    CustomListView viewer(keyMemoryName);
    QCOMPARE(viewer.GetCurImageSizeScale(), 5);

    const QPoint downAngelDelta{0, 8 * 15};
    const QPoint upAngelDelta{0, -8 * 15};

    QSignalSpy spy(&viewer, &CustomListView::iconSizeChanged);
    {  // +1 accept
      QVERIFY(SendWheelEvent(viewer, downAngelDelta, Qt::KeyboardModifier::ControlModifier, true));
      QCOMPARE(viewer.GetCurImageSizeScale(), 6);
      QVERIFY(!spy.isEmpty());
      QList<QVariant> params = spy.back();
      QCOMPARE(params.size(), 1);
      QSize newSz = params[0].value<QSize>();
      QCOMPARE(IMAGE_SIZE::ICON_SIZE_CANDIDATES[6], newSz);  // iconSizeChanged
      spy.pop_back();
    }

    {  // -1 accept
      QVERIFY(SendWheelEvent(viewer, upAngelDelta, Qt::KeyboardModifier::ControlModifier, true));
      QCOMPARE(viewer.GetCurImageSizeScale(), 5);
      QVERIFY(!spy.isEmpty());
      QList<QVariant> params = spy.back();
      QCOMPARE(params.size(), 1);
      QSize newSz = params[0].value<QSize>();
      QCOMPARE(IMAGE_SIZE::ICON_SIZE_CANDIDATES[5], newSz);
      spy.pop_back();
    }

    {  // already 0, cannot -1. not accept
      viewer.setIconSizeScaledIndex(0);
      QVERIFY(SendWheelEvent(viewer, upAngelDelta, Qt::KeyboardModifier::ControlModifier, true));
      QCOMPARE(viewer.GetCurImageSizeScale(), 0);
      QCOMPARE(spy.count(), 2);
    }

    {
      // already max, cannot +1. not accept
      viewer.setIconSizeScaledIndex(IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1);
      QVERIFY(SendWheelEvent(viewer, downAngelDelta, Qt::KeyboardModifier::ControlModifier, true));
      QCOMPARE(viewer.GetCurImageSizeScale(), IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1);
      QCOMPARE(spy.count(), 2);
    }

    {  // not with control modifier, will not change icon size, don't expect here
      SendWheelEvent(viewer, downAngelDelta, Qt::KeyboardModifier::NoModifier, true);
      QCOMPARE(viewer.GetCurImageSizeScale(), IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1);
      QCOMPARE(spy.count(), 2);
    }
  }
};

#include "CustomListViewTest.moc"
REGISTER_TEST(CustomListViewTest, false)
