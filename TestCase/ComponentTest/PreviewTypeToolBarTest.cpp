#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QSignalSpy>

#include "BeginToExposePrivateMember.h"
#include "PreviewDockWidget.h"
#include "EndToExposePrivateMember.h"

#include <QMetaType>
#include <QMainWindow>

Q_DECLARE_METATYPE(PreviewTypeTool::PREVIEW_TYPE_E)
class PreviewTypeToolBarTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {
    qRegisterMetaType<PreviewTypeTool::PREVIEW_TYPE_E>("PreviewTypeTool::PREVIEW_TYPE_E");  //
    QVERIFY(QMetaType::type("PreviewTypeTool::PREVIEW_TYPE_E") != 0);
  }

  void preview_type_changed_signal_ok() {
    QMainWindow mw;
    PreviewDockWidget previewDockWid("Preview Type previewDockWid");
    mw.addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, &previewDockWid);

    QSignalSpy spy(&previewDockWid, &PreviewDockWidget::previewTypeChanged);
    QCOMPARE(previewDockWid.GetCurrentPreviewType(), PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY);
    previewDockWid.PROGRESSIVE_LOAD_PRE->setChecked(true);
    emit previewDockWid.PROGRESSIVE_LOAD_PRE->triggered();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeLast(), (QVariantList{(int)PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD}));
    QCOMPARE(previewDockWid.GetCurrentPreviewType(), PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD);

    previewDockWid.CAROUSEL_PRE->setChecked(true);
    emit previewDockWid.CAROUSEL_PRE->triggered();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeLast(), (QVariantList{(int)PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL}));
    QCOMPARE(previewDockWid.GetCurrentPreviewType(), PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL);

    previewDockWid.CATEGORY_PRE->setChecked(true);
    emit previewDockWid.CATEGORY_PRE->triggered();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeLast(), (QVariantList{(int)PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY}));
    QCOMPARE(previewDockWid.GetCurrentPreviewType(), PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY);

    QCOMPARE(previewDockWid.isFloating(), false);
    previewDockWid.m_floatingPanel->toggle();
    QCOMPARE(previewDockWid.isFloating(), true);

    QCOMPARE(previewDockWid.isMinimized(), false);
    emit previewDockWid.m_minimizePanel->triggered();
    QCOMPARE(previewDockWid.isMinimized(), true);
  }
};

#include "PreviewTypeToolBarTest.moc"
REGISTER_TEST(PreviewTypeToolBarTest, false)
