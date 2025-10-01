#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"

// add necessary includes here
#include "OnScopeExit.h"

class OnScopeExitTest : public PlainTestSuite {
  Q_OBJECT

 public:
  static int EXEC_CNT;
  OnScopeExitTest() : PlainTestSuite{} {}
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

#include "OnScopeExitTest.moc"
REGISTER_TEST(OnScopeExitTest, false)
