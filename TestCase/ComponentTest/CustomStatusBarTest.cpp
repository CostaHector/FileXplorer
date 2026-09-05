#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "CustomStatusBar.h"
#include "EndToExposePrivateMember.h"

#include <QSignalSpy>

class CustomStatusBarTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void label_contents_ok() {
    CustomStatusBar statusBar;
    QCOMPARE(statusBar.mLabelsLst.size(), 3);
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

  void delgatee_ViewSwitchToolBar_ok() {
    CustomStatusBar statusBar;
    QVERIFY(statusBar.m_viewSwitcher != nullptr);
    QCOMPARE(statusBar._GetCurViewType(), statusBar.m_viewSwitcher->GetCurViewType());
    QSignalSpy _viewTypeChangedSpy{&statusBar, &CustomStatusBar::_viewTypeChanged};
    emit statusBar.m_viewSwitcher->viewTypeChanged(ViewTypeTool::ViewType::SCENE);
    QCOMPARE(_viewTypeChangedSpy.count(), 1);
    QVariantList sigParms = _viewTypeChangedSpy.takeLast();
    QCOMPARE(sigParms.size(), 1);
    QVariant sigParm0 = sigParms.front();
  }
};

#include "CustomStatusBarTest.moc"
REGISTER_TEST(CustomStatusBarTest, false)
