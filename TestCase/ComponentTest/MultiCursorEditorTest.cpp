#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "MultiCursorEditor.h"
#include "EndToExposePrivateMember.h"

class MultiCursorEditorTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:

  void default_ok() {
    MultiCursorEditor mce;
    QVERIFY(!mce.mBlinkTimer.isActive());
  }
};

#include "MultiCursorEditorTest.moc"
REGISTER_TEST(MultiCursorEditorTest, false)
