#include "MyTestSuite.h"
#include "Logger.h"
#include <QtTest/QtTest>

#define RUN_UT_MAIN_FILE 1
#ifdef RUN_UT_MAIN_FILE

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  auto& suite = MyTestSuite::suite();
  int succeedCnt = 0;
  int skipCnt = 0;
  QVector<MyTestSuite*> failedTests;
  for (auto it = suite.begin(); it != suite.end(); ++it) {
    if (MyTestSuite::bOnlyExecuteExculsive && !(*it)->mExclusive) {
      ++skipCnt;
      continue;
    }
    if (QTest::qExec(*it, argc, argv) == 0) {
      ++succeedCnt;
    } else {
      failedTests.push_back(*it);
    }
  }
  const int totalCnt = suite.size();
  const int shouldExecCnt = totalCnt - skipCnt;
  if (!failedTests.isEmpty()) {
    LOG_C("\n---------------ERROR: Following %d/%d TEST(s) FAILED:---------------", failedTests.size(), shouldExecCnt);
    for (int i = 0; i < failedTests.size(); ++i) {
      LOG_C("%dth testcase: %s", i+1, failedTests[i]->metaObject()->className());
    }
    LOG_C("\n---------------ERROR: Above %d TEST(s) FAILED.---------------", failedTests.size());
  } else {
    LOG_W("\n---------------\nAll %d/%d testcase passed (total:%d, skipCnt:%d)\n---------------\n", succeedCnt, shouldExecCnt, totalCnt, skipCnt);
  }
  return failedTests.size();
}

#endif
