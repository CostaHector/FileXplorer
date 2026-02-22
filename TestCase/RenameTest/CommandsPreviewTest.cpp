#include <QCoreApplication>
#include <QtTest>

#include "PlainTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "CommandsPreview.h"
#include "EndToExposePrivateMember.h"
#include "FileTool.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class CommandsPreviewTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void copyTextAction_ok() {
    const QString contentsInBrowser{"This is a test text"};
    MOCKER(FileTool::CopyTextToSystemClipboard) //
        .expects(exactly(1))
        .with(eq(contentsInBrowser)) //
        .will(returnValue(true));    //

    CommandsPreview preview("TestPreview CopyText");
    preview.setPlainText(contentsInBrowser);
    QVERIFY(preview.COPY_TEXT != nullptr);
    emit preview.COPY_TEXT->triggered();
  }

  void testStayOnTopAction() {
    CommandsPreview preview("TestPreview StayOnTop");
    preview.show();

    QVERIFY(!preview.windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    QVERIFY(!preview.STAY_ON_TOP->isChecked());

    preview.STAY_ON_TOP->setChecked(true);
    emit preview.STAY_ON_TOP->toggled(true);

    QVERIFY(preview.windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    QVERIFY(preview.STAY_ON_TOP->isChecked());

    preview.STAY_ON_TOP->setChecked(false);
    emit preview.STAY_ON_TOP->toggled(false);

    QVERIFY(!preview.windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    QVERIFY(!preview.STAY_ON_TOP->isChecked());
    preview.close();
  }
};

#include "CommandsPreviewTest.moc"
REGISTER_TEST(CommandsPreviewTest, false)
