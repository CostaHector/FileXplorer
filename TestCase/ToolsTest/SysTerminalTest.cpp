#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "SysTerminal.h"
#include "EndToExposePrivateMember.h"
#include <QFileInfo>

class SysTerminalTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void system_terminal_only_works_for_linux_exists_path() {
    SysTerminal sysTerm;
    QVERIFY(!sysTerm("inexist path"));

    bool expectReturnValue = true;
    QCOMPARE(sysTerm(__FILE__), expectReturnValue);
    QCOMPARE(sysTerm(QFileInfo(__FILE__).absolutePath()), expectReturnValue);
  }
};

#include "SysTerminalTest.moc"
REGISTER_TEST(SysTerminalTest, false)
