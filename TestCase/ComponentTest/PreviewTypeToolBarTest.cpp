#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QSignalSpy>

#include "BeginToExposePrivateMember.h"
#include "PreviewTypeToolBar.h"
#include "EndToExposePrivateMember.h"

#include <QMetaType>

Q_DECLARE_METATYPE(PreviewTypeTool::PREVIEW_TYPE_E)
class PreviewTypeToolBarTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {
    qRegisterMetaType<PreviewTypeTool::PREVIEW_TYPE_E>("PreviewTypeTool::PREVIEW_TYPE_E");  //
    QVERIFY(QMetaType::type("PreviewTypeTool::PREVIEW_TYPE_E") != 0);
  }

  void test_preview_type_changed_signal() {
    PreviewTypeToolBar toolbar("Preview Type Toolbar");
    QSignalSpy spy(&toolbar, &PreviewTypeToolBar::previewTypeChanged);
    QCOMPARE(toolbar.mCurrentPreviewType, PreviewTypeTool::DEFULT_PREVIEW_TYPE_E);
    toolbar.CATEGORY_PRE->trigger();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> parms = spy.back();
    QCOMPARE(parms.size(), 1);
    QCOMPARE(parms.back().value<PreviewTypeTool::PREVIEW_TYPE_E>(), PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY);
    QCOMPARE(toolbar.mCurrentPreviewType, PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY);
  }
};

#include "PreviewTypeToolBarTest.moc"
REGISTER_TEST(PreviewTypeToolBarTest, false)
