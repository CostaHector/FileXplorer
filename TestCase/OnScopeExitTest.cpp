#include <QCoreApplication>
#include <QtTest>
#include "MyTestSuite.h"

// add necessary includes here
#include "OnScopeExit.h"

class OnScopeExitTest : public MyTestSuite {
  Q_OBJECT

 public:
  static int EXEC_CNT;
  OnScopeExitTest() : MyTestSuite{false} {}
 private slots:
  void test_auto_exit() {
    EXEC_CNT = 0;
    {
      QCOMPARE(EXEC_CNT, 0);
      ON_SCOPE_EXIT {
        ++EXEC_CNT;
      };
      QCOMPARE(EXEC_CNT, 0);
    }
    QCOMPARE(EXEC_CNT, 1);
    {
      ON_SCOPE_EXIT {
        ++EXEC_CNT;
      };
      QCOMPARE(EXEC_CNT, 1);
      --EXEC_CNT;
      QCOMPARE(EXEC_CNT, 0);
    }
    QCOMPARE(EXEC_CNT, 1);
  }
};

int OnScopeExitTest::EXEC_CNT = 0;
OnScopeExitTest g_OnScopeExitTest;
#include "OnScopeExitTest.moc"
