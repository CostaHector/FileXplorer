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

class CustomTableViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
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

    ctv.onResizeRowToContents(true);
    QCOMPARE(ctv.verticalHeader()->sectionResizeMode(0), QHeaderView::ResizeMode::ResizeToContents);

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
    ctv.AppendVerticalHeaderMenuAGS(&extraVerAg);
    QCOMPARE(ctv.m_verMenu->actions().size(), beforeVCount + 1 + 2);

    QActionGroup extraHorAg{&ctv};
    QAction hExtraAct1{"hAct1", &ctv};
    QAction hExtraAct2{"hAct2", &ctv};
    extraHorAg.addAction(&hExtraAct1);
    extraHorAg.addAction(&hExtraAct2);
    QVERIFY(ctv.m_horMenu != nullptr);
    int beforeHCount = ctv.m_horMenu->actions().size();
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
};

#include "CustomTableViewTest.moc"
REGISTER_TEST(CustomTableViewTest, false)
