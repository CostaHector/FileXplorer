#include <QCoreApplication>
#include <QtTest>
#include "OnScopeExit.h"
#include "PlainTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "CommandsPreview.h"
#include "EndToExposePrivateMember.h"
#include <QClipboard>

class CommandsPreviewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void copyTextAction_ok() {
    QClipboard* clipboard = QApplication::clipboard();
    QVERIFY(clipboard != nullptr);
    const QString beforeContentsInClipboard = clipboard->text();
    ON_SCOPE_EXIT{
      clipboard->setText(beforeContentsInClipboard);
    };

    CommandsPreview preview("TestPreview CopyText");
    const QString contentsInBrowser {"This is a test text"};
    preview.setPlainText(contentsInBrowser);

    QVERIFY(preview.COPY_TEXT != nullptr);
    emit preview.COPY_TEXT->triggered();

    const QString afterCopyActionTextInClipboard = clipboard->text();
    QCOMPARE(afterCopyActionTextInClipboard, contentsInBrowser);
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
