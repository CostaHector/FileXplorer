#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "HarMgr.h"
#include "EndToExposePrivateMember.h"

#include "FileTool.h"

#include <QShowEvent>

class HarMgrTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void initTestCase() { HarFilesMocker::mockHarFiles().clear(); }

  void default_ok() {
    HarMgr harMgr;
    QVERIFY(harMgr.testAttribute(Qt::WidgetAttribute::WA_DeleteOnClose));

    harMgr.showEvent(nullptr);
    harMgr.closeEvent(nullptr);

    QShowEvent se;
    QCloseEvent ce;
    harMgr.showEvent(&se);
    harMgr.closeEvent(&ce);

    QCOMPARE(harMgr.OpenHar("inexist har file"), 0);

    QVERIFY(harMgr.m_searchLineEdit != nullptr);
    QVERIFY(harMgr.m_searchAction != nullptr);
    QCOMPARE(harMgr.m_searchLineEdit->actions(), (QList<QAction *>{harMgr.m_searchAction}));

    harMgr.m_searchLineEdit->setText("Kaka");
    harMgr.onStartFilter();

    harMgr.m_searchAction->triggered();
  }

  void pixmapByteArrayChanged_signal_connected_ok() {
    HarMgr harMgr;
    QVERIFY(harMgr.m_harView != nullptr);
    QVERIFY(harMgr.m_imgPreview != nullptr);
    QVERIFY(harMgr.m_imgPreview->mLabel != nullptr);
    QVERIFY(harMgr.m_imgPreview->mLabel->movie() == nullptr);

    const QByteArray gifBa{FileTool::ByteArrayReader(":/image_test/sample_Blink_WhiteBlackRed")};
    QVERIFY(gifBa.size() > 0);

    emit harMgr.m_harView->pixmapByteArrayChanged(gifBa, "gif");

    QVERIFY(harMgr.m_imgPreview->mLabel->movie() != nullptr);
  }
};

#include "HarMgrTest.moc"
REGISTER_TEST(HarMgrTest, false)
