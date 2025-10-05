#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ViewsStackedWidget.h"
#include "EndToExposePrivateMember.h"

class ViewsStackedWidgetTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {}

  void cleanupTestCase() {}

  void default_constructor_will_not_crash_down() {
    CurrentRowPreviewer rowPreviewer;
    ViewsStackedWidget viewStkWid(&rowPreviewer);
    QCOMPARE(viewStkWid.GetVt(), ViewTypeTool::DEFAULT_VIEW_TYPE);
    viewStkWid.SetVt(ViewTypeTool::ViewType::LIST);
    QCOMPARE(viewStkWid.GetVt(), ViewTypeTool::ViewType::LIST);
    viewStkWid.SetVt(ViewTypeTool::DEFAULT_VIEW_TYPE);
    QCOMPARE(viewStkWid.GetVt(), ViewTypeTool::DEFAULT_VIEW_TYPE);

    viewStkWid.BindNavigationAddressBar(nullptr);
    viewStkWid.BindDatabaseSearchToolBar(nullptr);
    viewStkWid.BindAdvanceSearchToolBar(nullptr);
    viewStkWid.BindCastSearchToolBar(nullptr);
    viewStkWid.BindLogger(nullptr);

    viewStkWid.disconnectSelectionChanged();
  }
};

#include "ViewsStackedWidgetTest.moc"
REGISTER_TEST(ViewsStackedWidgetTest, false)
