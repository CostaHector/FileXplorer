#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QSignalSpy>

#include "BeginToExposePrivateMember.h"
#include "ViewHelper.h"
#include "EndToExposePrivateMember.h"

#include "ViewActions.h"
#include "AddressBarActions.h"

class ViewHelperTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void onMouseSidekeyBackwardForward_ok() {
    auto& addressInst = g_addressBarActions();
    QSignalSpy backToSpy{addressInst._BACK_TO, &QAction::triggered};
    QSignalSpy forwardToSpy{addressInst._FORWARD_TO, &QAction::triggered};

    auto& viewInst = ViewActions::GetInst();
    QSignalSpy viewBackToSpy{viewInst._VIEW_BACK_TO, &QAction::triggered};
    QSignalSpy viewForwardToSpy{viewInst._VIEW_FORWARD_TO, &QAction::triggered};

    QVERIFY(ViewHelper::onMouseSidekeyBackwardForward(Qt::NoModifier, Qt::BackButton));
    QCOMPARE(backToSpy.count(), 1);
    backToSpy.takeLast();

    QVERIFY(ViewHelper::onMouseSidekeyBackwardForward(Qt::NoModifier, Qt::ForwardButton));
    QCOMPARE(forwardToSpy.count(), 1);
    forwardToSpy.takeLast();

    QVERIFY(ViewHelper::onMouseSidekeyBackwardForward(Qt::ControlModifier, Qt::BackButton));
    QCOMPARE(viewBackToSpy.count(), 1);
    viewBackToSpy.takeLast();

    QVERIFY(ViewHelper::onMouseSidekeyBackwardForward(Qt::ControlModifier, Qt::ForwardButton));
    QCOMPARE(viewForwardToSpy.count(), 1);
    viewForwardToSpy.takeLast();

    QVERIFY(!ViewHelper::onMouseSidekeyBackwardForward(Qt::NoModifier, Qt::MiddleButton));
    QVERIFY(!ViewHelper::onMouseSidekeyBackwardForward(Qt::ControlModifier, Qt::MiddleButton));
    QVERIFY(!ViewHelper::onMouseSidekeyBackwardForward(Qt::AltModifier, Qt::BackButton));
    QCOMPARE(backToSpy.count(), 0);
    QCOMPARE(forwardToSpy.count(), 0);
    QCOMPARE(viewBackToSpy.count(), 0);
    QCOMPARE(viewForwardToSpy.count(), 0);
  }

  void onDropMimeData_ok() {
    // nullptr
    QCOMPARE(ViewHelper::onDropMimeData(nullptr, Qt::DropAction::CopyAction, "path/to/destPath"), false);

    // no urls
    QMimeData noUrlMimedata;
    QCOMPARE(noUrlMimedata.hasUrls(), false);
    QCOMPARE(ViewHelper::onDropMimeData(&noUrlMimedata, Qt::DropAction::CopyAction, "path/to/destPath"), true);

    // dest path inexist, copy failed
    QMimeData urlMimedata;
    urlMimedata.setUrls({QUrl::fromLocalFile(__FILE__)});
    QCOMPARE(ViewHelper::onDropMimeData(&urlMimedata, Qt::DropAction::CopyAction, "path/to/destPath"), false);
  }
};

#include "ViewHelperTest.moc"
REGISTER_TEST(ViewHelperTest, false)
