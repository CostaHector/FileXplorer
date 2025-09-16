#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "CustomStatusBar.h"
#include "EndToExposePrivateMember.h"

class CustomStatusBarTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void test_custom_statusbar_label_contents_ok() {
    CustomStatusBar statusBar;
    QCOMPARE(statusBar.mLabelsLst.size(), 3);
    QVERIFY(statusBar.m_viewsSwitcher != nullptr);

    // onPathInfoChanged - ITEMS
    statusBar.onPathInfoChanged(10, CustomStatusBar::ITEMS);
    QCOMPARE(statusBar.mLabelsLst[CustomStatusBar::ITEMS]->text(),
             QString("Total 10 item(s) |"));

    // onPathInfoChanged - SELECTED
    statusBar.onPathInfoChanged(3, CustomStatusBar::SELECTED);
    QCOMPARE(statusBar.mLabelsLst[CustomStatusBar::SELECTED]->text(),
             QString("3 selected |"));

    // onMsgChanged - normal
    statusBar.onMsgChanged("Operation completed");
    QCOMPARE(statusBar.mLabelsLst[CustomStatusBar::MSG]->text(),
             "Operation completed");

    // onMsgChanged - abnormal
    statusBar.onMsgChanged("Warning: Low disk space", STATUS_ALERT_LEVEL::ABNORMAL);
    QCOMPARE(statusBar.mLabelsLst[CustomStatusBar::MSG]->text(),
             "Warning: Low disk space");
  }
};

#include "CustomStatusBarTest.moc"
REGISTER_TEST(CustomStatusBarTest, false)
