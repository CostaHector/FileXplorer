#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "CustomTableView.h"
#include "EndToExposePrivateMember.h"
#include "ModelTestHelper.h"
#include <QHeaderView>
#include "AddressBarActions.h"
#include "ViewHelper.h"
#include "AddressBarActions.h"
#include "ViewActions.h"

#include "MouseKeyboardEventHelper.h"
using namespace MouseKeyboardEventHelper;

class CustomTableViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void cleanupTestCase() { Configuration().clear(); }

  void column_width_adjust_ok() {
    Configuration().clear();

    QStandardItemModel model;
    ModelTestHelper::InitStdItemModel(model, "Item At (%1, %2)\nSecondLine", 3, 2);

    CustomTableView ctv{"ctv width"};
    ctv.setModel(&model);
    ctv.InitTableView(true);

    ctv.onStretchLastSection(true);
    QCOMPARE(ctv.horizontalHeader()->stretchLastSection(), true);

    ctv.onEnableColumnSort(true);
    QCOMPARE(ctv.isSortingEnabled(), true);

    ctv.onResizeRowToContents(false);
    ctv.onResizeRowToContents(true);
    QCOMPARE(ctv.verticalHeader()->sectionResizeMode(0), QHeaderView::ResizeMode::ResizeToContents);

    ctv.onResizeColumnToContents(false);
    ctv.onResizeColumnToContents(true);
    QCOMPARE(ctv.horizontalHeader()->sectionResizeMode(0), QHeaderView::ResizeMode::ResizeToContents);

    ctv.onShowHorizontalHeader(true);
    ctv.onShowVerticalHeader(true);

    QCOMPARE(ctv.m_horizontalHeaderSectionClicked, CustomTableView::INVALID_SECTION_INDEX);
    QCOMPARE(ctv.isColumnHidden(0), false);
    QCOMPARE(ctv.onHideThisColumn(), false);

    // click on horizontal header will affect m_horizontalHeaderSectionClicked
    QHeaderView* horHeader = ctv.horizontalHeader();
    QVERIFY(horHeader != nullptr);
    const QRect horSection0Rect{HeaderRectHelper::GetHeaderRect(ctv, 0, Qt::Horizontal)};
    QVERIFY(horSection0Rect.isValid());
    QPoint horHeader0Center = horSection0Rect.center();
    emit horHeader->customContextMenuRequested(horHeader0Center);
    QCOMPARE(ctv.m_horizontalHeaderSectionClicked, 0);
    ctv.onHideThisColumn();
    QCOMPARE(ctv.m_horizontalHeaderSectionClicked, CustomTableView::INVALID_SECTION_INDEX);
    QCOMPARE(ctv.isColumnHidden(0), true);
    QCOMPARE(ctv.onHideThisColumn(), false);  // already invalidate cannot hide on a invalid index
    QCOMPARE(ctv.isColumnHidden(0), true);

    // click on vertical header will not affect m_horizontalHeaderSectionClicked
    QHeaderView* verHeader = ctv.verticalHeader();
    QVERIFY(verHeader != nullptr);
    const QRect verSection0Rect{HeaderRectHelper::GetHeaderRect(ctv, 0, Qt::Vertical)};
    QVERIFY(verSection0Rect.isValid());
    QPoint verHeader0Center = verSection0Rect.center();
    emit verHeader->customContextMenuRequested(verHeader0Center);
    QCOMPARE(ctv.m_horizontalHeaderSectionClicked, CustomTableView::INVALID_SECTION_INDEX);
  }

  void header_menu_ok() {
    CustomTableView ctv{"ctv menu"};
    QActionGroup extraVerAg{&ctv};
    QAction vExtraAct1{"vAct1", &ctv};
    QAction vExtraAct2{"vAct2", &ctv};
    extraVerAg.addAction(&vExtraAct1);
    extraVerAg.addAction(&vExtraAct2);
    QVERIFY(ctv.m_verMenu != nullptr);
    int beforeVCount = ctv.m_verMenu->actions().size();
    ctv.AppendVerticalHeaderMenuAGS(nullptr); // not crash down
    ctv.AppendVerticalHeaderMenuAGS(&extraVerAg);
    QCOMPARE(ctv.m_verMenu->actions().size(), beforeVCount + 1 + 2);

    QActionGroup extraHorAg{&ctv};
    QAction hExtraAct1{"hAct1", &ctv};
    QAction hExtraAct2{"hAct2", &ctv};
    extraHorAg.addAction(&hExtraAct1);
    extraHorAg.addAction(&hExtraAct2);
    QVERIFY(ctv.m_horMenu != nullptr);
    int beforeHCount = ctv.m_horMenu->actions().size();
    ctv.AppendHorizontalHeaderMenuAGS(nullptr); // not crash down
    ctv.AppendHorizontalHeaderMenuAGS(&extraHorAg);
    QCOMPARE(ctv.m_horMenu->actions().size(), beforeHCount + 1 + 2);

    QAction act{"act1", &ctv};
    QMenu menu{&ctv};
    menu.addAction(&act);
    ctv.BindMenu(&menu);
    QVERIFY(ctv.m_menu != nullptr);

    const QPoint posCenter = ctv.geometry().center();
    QTest::mouseClick(&ctv, Qt::RightButton, Qt::NoModifier, posCenter);
    QContextMenuEvent rghContextEvent(QContextMenuEvent::Mouse, posCenter, ctv.mapToGlobal(posCenter));
    ctv.contextMenuEvent(&rghContextEvent);
    QCOMPARE(rghContextEvent.isAccepted(), true);
  }

  void set_section_size_ok() {
    using namespace UserSpecifiedIntValueMock;
    static const int EXPECT_ROW_DEFAULT_SECTION_SIZE = 51;
    static const int EXPECT_ROW_MAX_SECTION_SIZE = 71;
    static const int EXPECT_COLUMN_DEFAULT_SECTION_SIZE = 121;
    {
      CustomTableView ctv{"ctv set section size ok"};
      {
        MockQInputDialogGetInt(false, 40);
        QCOMPARE(ctv.onSetRowMaxHeight(), false);
        QCOMPARE(ctv.onSetRowDefaultSectionSize(), false);
        QCOMPARE(ctv.onSetColumnDefaultSectionSize(), false);
      }
      // precondition
      QVERIFY(ctv.verticalHeader()->maximumSectionSize() != EXPECT_ROW_MAX_SECTION_SIZE);
      QVERIFY(ctv.verticalHeader()->defaultSectionSize() != EXPECT_ROW_DEFAULT_SECTION_SIZE);
      QVERIFY(ctv.horizontalHeader()->maximumSectionSize() != EXPECT_COLUMN_DEFAULT_SECTION_SIZE);
      // change section size
      {
        MockQInputDialogGetInt(true, EXPECT_ROW_MAX_SECTION_SIZE);
        QCOMPARE(ctv.onSetRowMaxHeight(), true);
      }
      {
        MockQInputDialogGetInt(true, EXPECT_ROW_DEFAULT_SECTION_SIZE);
        QCOMPARE(ctv.onSetRowDefaultSectionSize(), true);
      }
      {
        MockQInputDialogGetInt(true, EXPECT_COLUMN_DEFAULT_SECTION_SIZE);
        QCOMPARE(ctv.onSetColumnDefaultSectionSize(), true);
      }
      QCOMPARE(ctv.verticalHeader()->maximumSectionSize(), EXPECT_ROW_MAX_SECTION_SIZE);
      QCOMPARE(ctv.verticalHeader()->defaultSectionSize(), EXPECT_ROW_DEFAULT_SECTION_SIZE);
      QCOMPARE(ctv.horizontalHeader()->defaultSectionSize(), EXPECT_COLUMN_DEFAULT_SECTION_SIZE);
      QCOMPARE(ctv.m_defaultTableRowHeight, EXPECT_ROW_DEFAULT_SECTION_SIZE);
      QCOMPARE(ctv.m_defaultTableColumnWidth, EXPECT_COLUMN_DEFAULT_SECTION_SIZE);

      ctv.onVerticalHeaderChanged();
    }
    // QCOMPARE(ctv2.verticalHeader()->maximumSectionSize(), EXPECT_ROW_MAX_SECTION_SIZE); //  max row size here not record in service
    // QCOMPARE(ctv2.verticalHeader()->defaultSectionSize(), EXPECT_ROW_DEFAULT_SECTION_SIZE); // this two may be ovewrite by header()->restore
    // QCOMPARE(ctv1.horizontalHeader()->defaultSectionSize(), EXPECT_COLUMN_DEFAULT_SECTION_SIZE);
  }

  void column_visibility_set_ok() {
    static constexpr int COLUMN_CNT = 4;
    QStandardItemModel stdModel;
    ModelTestHelper::InitStdItemModel(stdModel, "cell (%1, %2)", 2, COLUMN_CNT);
    ModelTestHelper::InitHorizontalHeaderLabels(stdModel);

    CustomTableView ctv1{"ctv column visibility set ok"};
    ctv1.setModel(&stdModel);

    using namespace UserSpecifiedIntValueMock;
    mockColumnsShowSwitch() = std::string(COLUMN_CNT, '0').c_str();  // all hide
    QCOMPARE(ctv1.onColumnVisibilityAdjust(), true);
    QCOMPARE(ctv1.m_columnsShowSwitch, "0000");

    mockColumnsShowSwitch() = std::string(COLUMN_CNT, '1').c_str();  // all shown
    QCOMPARE(ctv1.onColumnVisibilityAdjust(), true);
    QCOMPARE(ctv1.m_columnsShowSwitch, "1111");

    {  // bounder test
      static constexpr int COLUMN_CNT_100 = 100;
      QVERIFY(ctv1.m_columnsShowSwitch.size() < COLUMN_CNT_100);
      QStandardItemModel std100ColumnModel;
      ModelTestHelper::InitStdItemModel(std100ColumnModel, "cell (%1, %2)", 1, COLUMN_CNT_100);
      ctv1.setModel(&std100ColumnModel);  // model switched

      mockColumnsShowSwitch() = std::string(COLUMN_CNT_100, '0').c_str();  // all hide
      QCOMPARE(ctv1.onColumnVisibilityAdjust(), true);
      QCOMPARE(ctv1.m_columnsShowSwitch, std::string(COLUMN_CNT_100, '0').c_str());
    }
  }

  void mouseSideClick_NavigationSignals() {
    CustomTableView view("CustomTableViewMouseSideKey");

    auto& addressInst = g_addressBarActions();
    auto& viewInst = g_viewActions();

    QSignalSpy backAddressSpy(addressInst._BACK_TO, &QAction::triggered);
    QSignalSpy forwardAddressSpy(addressInst._FORWARD_TO, &QAction::triggered);
    QSignalSpy backViewSpy(viewInst._VIEW_BACK_TO, &QAction::triggered);
    QSignalSpy forwardViewSpy(viewInst._VIEW_FORWARD_TO, &QAction::triggered);

    {  // accepted events
      QVERIFY(SendMousePressEvent<CustomTableView>(view, Qt::BackButton, Qt::NoModifier));
      QCOMPARE(backAddressSpy.count(), 1);

      QVERIFY(SendMousePressEvent<CustomTableView>(view, Qt::ForwardButton, Qt::NoModifier));
      QCOMPARE(forwardAddressSpy.count(), 1);

      QVERIFY(SendMousePressEvent<CustomTableView>(view, Qt::BackButton, Qt::ControlModifier));
      QCOMPARE(backViewSpy.count(), 1);

      QVERIFY(SendMousePressEvent<CustomTableView>(view, Qt::ForwardButton, Qt::ControlModifier));
      QCOMPARE(forwardViewSpy.count(), 1);
    }

            // Alt+back: nothing happen
    {
      SendMousePressEvent<CustomTableView>(view, Qt::BackButton, Qt::AltModifier);
      QCOMPARE(backViewSpy.count(), 1);
      QCOMPARE(backAddressSpy.count(), 1);
    }

            // left click: nothing happen
    {
      SendMousePressEvent<CustomTableView>(view, Qt::LeftButton, Qt::NoModifier);
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
};

#include "CustomTableViewTest.moc"
REGISTER_TEST(CustomTableViewTest, false)
