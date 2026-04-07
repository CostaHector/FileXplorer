#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"

#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "CustomListView.h"
#include "EndToExposePrivateMember.h"

#include "AddressBarActions.h"
#include "ViewActions.h"
#include "ImageTool.h"
#include "MouseKeyboardEventHelper.h"
using namespace MouseKeyboardEventHelper;

class CustomListViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() { QVERIFY(QMetaType::type("QSize") != 0); }

  void cleanupTestCase() {  //
    Configuration().clear();
  }

  void init() { Configuration().clear(); }

  void save_settings_in_destructor_ok() {
    const QString listInstanceName{"saveSettingTest_ok"};
    {
      CustomListView view{listInstanceName};
      QVERIFY(!view.m_defaultFlowLeft2Right);
      QVERIFY(!view.m_defaultViewModeIcon);
      QVERIFY(!view.m_defaultWrapping);
      QVERIFY(!view._FLOW_ORIENTATION_LTR->isChecked());
      QVERIFY(!view._VIEW_MODE_LIST_ICON->isChecked());
      QVERIFY(!view._WRAPPING_ACTIONS->isChecked());
      QCOMPARE(view.flow(), QListView::Flow::TopToBottom);
      QCOMPARE(view.viewMode(), QListView::ViewMode::ListMode);
      QCOMPARE(view.isWrapping(), false);

      // 默认行为不会修改缺省值
      view.initExclusivePreferenceSetting();
      QVERIFY(!view.m_defaultFlowLeft2Right);
      QVERIFY(!view.m_defaultViewModeIcon);
      QVERIFY(!view.m_defaultWrapping);
      QVERIFY(!view._FLOW_ORIENTATION_LTR->isChecked());
      QVERIFY(!view._VIEW_MODE_LIST_ICON->isChecked());
      QVERIFY(!view._WRAPPING_ACTIONS->isChecked());
      QCOMPARE(view.flow(), QListView::Flow::TopToBottom);
      QCOMPARE(view.viewMode(), QListView::ViewMode::ListMode);
      QCOMPARE(view.isWrapping(), false);

      // 模拟子类override initExclusivePreferenceSetting body如下
      view.m_defaultFlowLeft2Right = true;
      view.m_defaultViewModeIcon = true;
      view.m_defaultWrapping = false;

      view.InitListView();
      QVERIFY(view._FLOW_ORIENTATION_LTR->isChecked());
      QVERIFY(view._VIEW_MODE_LIST_ICON->isChecked());
      QVERIFY(!view._WRAPPING_ACTIONS->isChecked());
      QCOMPARE(view.flow(), QListView::Flow::LeftToRight);
      QCOMPARE(view.viewMode(), QListView::ViewMode::IconMode);
      QCOMPARE(view.isWrapping(), false);
    }
    QVERIFY(Configuration().contains(listInstanceName + "/FLOW_ORIENTATION"));
    QVERIFY(Configuration().contains(listInstanceName + "/VIEW_MODE_LIST_ICON"));
    QVERIFY(Configuration().contains(listInstanceName + "/RESIZED_MODE_FIXED_OR_ADJUST"));
    QVERIFY(Configuration().contains(listInstanceName + "/WRAPPING_ACTIONS"));
    QVERIFY(Configuration().contains(listInstanceName + "/UNIFORM_ITEM_SIZES"));

    QCOMPARE(Configuration().value(listInstanceName + "/FLOW_ORIENTATION").toBool(), true);
    QCOMPARE(Configuration().value(listInstanceName + "/VIEW_MODE_LIST_ICON").toInt(), true);
    QCOMPARE(Configuration().value(listInstanceName + "/WRAPPING_ACTIONS").toBool(), false);
  }

  void contextMenuEvent_ok() {
    const QString keyMemoryName = "CustomListViewTest";
    IMAGE_SIZE::SaveInitialScaledSize(keyMemoryName, 5);
    CustomListView view(keyMemoryName);

    view.PushFrontExclusiveActions({});

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

  void wheelEvent_zoom_ok() {
    const QString keyMemoryName = "CustomListViewTest";
    IMAGE_SIZE::SaveInitialScaledSize(keyMemoryName, 5);
    CustomListView viewer(keyMemoryName);
    QCOMPARE(viewer._ICON_SIZE_MENU->GetScaledIndex(), 5);

    const QPoint downAngelDelta{0, 8 * 15};
    const QPoint upAngelDelta{0, -8 * 15};

    QSignalSpy spy(&viewer, &CustomListView::iconSizeChanged);
    {  // +1 accept
      QVERIFY(SendWheelEvent(viewer, downAngelDelta, Qt::KeyboardModifier::ControlModifier, true));
      QCOMPARE(viewer._ICON_SIZE_MENU->GetScaledIndex(), 6);
      QVERIFY(!spy.isEmpty());
      QVariantList params = spy.back();
      QCOMPARE(params.size(), 1);
      QSize newSz = params[0].value<QSize>();
      QCOMPARE(IMAGE_SIZE::ICON_SIZE_CANDIDATES[6], newSz);  // iconSizeChanged
      spy.pop_back();
    }

    {  // -1 accept
      QVERIFY(SendWheelEvent(viewer, upAngelDelta, Qt::KeyboardModifier::ControlModifier, true));
      QCOMPARE(viewer._ICON_SIZE_MENU->GetScaledIndex(), 5);
      QVERIFY(!spy.isEmpty());
      QVariantList params = spy.back();
      QCOMPARE(params.size(), 1);
      QSize newSz = params[0].value<QSize>();
      QCOMPARE(IMAGE_SIZE::ICON_SIZE_CANDIDATES[5], newSz);
      spy.pop_back();
    }

    {  // already 0, cannot -1. not accept
      viewer._ICON_SIZE_MENU->UpdateScaledIndexInWheelEvent(0);
      QVERIFY(SendWheelEvent(viewer, upAngelDelta, Qt::KeyboardModifier::ControlModifier, true));
      QCOMPARE(viewer._ICON_SIZE_MENU->GetScaledIndex(), 0);
      QCOMPARE(spy.count(), 2);
    }

    {
      // already max, cannot +1. not accept
      viewer._ICON_SIZE_MENU->UpdateScaledIndexInWheelEvent(IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1);
      QVERIFY(SendWheelEvent(viewer, downAngelDelta, Qt::KeyboardModifier::ControlModifier, true));
      QCOMPARE(viewer._ICON_SIZE_MENU->GetScaledIndex(), IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1);
      QCOMPARE(spy.count(), 2);
    }

    {  // not with control modifier, will not change icon size, don't expect here
      SendWheelEvent(viewer, downAngelDelta, Qt::KeyboardModifier::NoModifier, true);
      QCOMPARE(viewer._ICON_SIZE_MENU->GetScaledIndex(), IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1);
      QCOMPARE(spy.count(), 2);
    }
  }

  void iconScaledIndexChanged_ok() {
    Configuration().clear();

    QWidget wid;
    IconSizeMenu fsMenu{"iconMenuUsedIn_ListFileSystemView", "FileSystemListView", &wid};
    int index0 = fsMenu.GetScaledIndex();

    IconSizeMenu sceneMenu{"iconMenuUsedIn_ListFileSystemView", "SceneListView", &wid};
    int index1 = sceneMenu.GetScaledIndex();
    // 含有Scene, Image/img的无记录时的初始值会大于不含有的
    QVERIFY(index1 > index0);

    QSignalSpy iconScaledIndexChangedSpy{&sceneMenu, &IconSizeMenu::iconScaledIndexChanged};

    QCOMPARE(sceneMenu.EmitIconScaledIndexChanged(nullptr), false);

    QAction pAct;
    QCOMPARE(sceneMenu.EmitIconScaledIndexChanged(&pAct), false);  // no value in data
    QCOMPARE(iconScaledIndexChangedSpy.count(), 0);

    pAct.setData(index1);
    QCOMPARE(sceneMenu.EmitIconScaledIndexChanged(&pAct), false);  // unchange
    QCOMPARE(iconScaledIndexChangedSpy.count(), 0);

    pAct.setData(index0);
    QCOMPARE(sceneMenu.EmitIconScaledIndexChanged(&pAct), true);  // changed
    QCOMPARE(iconScaledIndexChangedSpy.count(), 1);
    QCOMPARE(iconScaledIndexChangedSpy.takeLast(), (QVariantList{index0}));
  }
};

#include "CustomListViewTest.moc"
REGISTER_TEST(CustomListViewTest, false)
