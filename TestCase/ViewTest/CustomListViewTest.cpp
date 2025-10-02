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
    CustomListView viewer(keyMemoryName);

    QMenu menu;
    viewer.BindMenu(nullptr);
    viewer.BindMenu(&menu);
    viewer.BindMenu(&menu);

    viewer.contextMenuEvent(nullptr);

    const QPoint posCenter = viewer.geometry().center();
    QContextMenuEvent rghContextEvent(QContextMenuEvent::Mouse, posCenter, viewer.mapToGlobal(posCenter));
    viewer.contextMenuEvent(&rghContextEvent);
    QCOMPARE(rghContextEvent.isAccepted(), true);

    {  // todo: here can be rafactor with mouseSideClick_NavigationSignals
      auto& addressInst = g_addressBarActions();
      auto& viewInst = g_viewActions();

      viewer.mousePressEvent(nullptr);
      QSignalSpy backAddressSpy(addressInst._BACK_TO, &QAction::triggered);
      QSignalSpy forwardAddressSpy(addressInst._FORWARD_TO, &QAction::triggered);
      QSignalSpy backViewSpy(viewInst._VIEW_BACK_TO, &QAction::triggered);
      QSignalSpy forwardViewSpy(viewInst._VIEW_FORWARD_TO, &QAction::triggered);
      {
        QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::BackButton, Qt::BackButton, Qt::NoModifier);
        viewer.mousePressEvent(&event);
        QCOMPARE(backAddressSpy.count(), 1);
        QCOMPARE(event.isAccepted(), true);
      }

      {
        QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::ForwardButton, Qt::ForwardButton, Qt::NoModifier);
        viewer.mousePressEvent(&event);
        QCOMPARE(forwardAddressSpy.count(), 1);
        QCOMPARE(event.isAccepted(), true);
      }

      {
        QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::BackButton, Qt::BackButton, Qt::ControlModifier);
        viewer.mousePressEvent(&event);
        QCOMPARE(backViewSpy.count(), 1);
        QCOMPARE(event.isAccepted(), true);
      }

      {
        QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::ForwardButton, Qt::ForwardButton, Qt::ControlModifier);
        viewer.mousePressEvent(&event);
        QCOMPARE(forwardViewSpy.count(), 1);
        QCOMPARE(event.isAccepted(), true);
      }

      // Alt+back: nothing happen
      {
        QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::BackButton, Qt::BackButton, Qt::AltModifier);
        viewer.mousePressEvent(&event);
        QCOMPARE(backAddressSpy.count(), 1);
        QCOMPARE(backViewSpy.count(), 1);
      }

      // left click: nothing happen
      {
        QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        viewer.mousePressEvent(&event);
        QCOMPARE(backAddressSpy.count(), 1);
        QCOMPARE(forwardViewSpy.count(), 1);
      }

      // all signal params ok
      QCOMPARE(backAddressSpy.first()[0].toBool(), false);
      QCOMPARE(forwardAddressSpy.first()[0].toBool(), false);
      QCOMPARE(backViewSpy.first()[0].toBool(), false);
      QCOMPARE(forwardViewSpy.first()[0].toBool(), false);
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
