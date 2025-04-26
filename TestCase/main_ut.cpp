#include "TestCase/pub/MyTestSuite.h"
#include <QtTest/QtTest>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  auto& suite = MyTestSuite::suite();
  int succeedCnt = 0;
  int skipCnt = 0;
  for (auto it = suite.begin(); it != suite.end(); ++it) {
    if (MyTestSuite::bOnlyExecuteExculsive && !(*it)->mExclusive) {
      ++skipCnt;
      continue;
    }
    if (QTest::qExec(*it, argc, argv) == 0) {
      ++succeedCnt;
    }
  }
  const int totalCnt = suite.size();
  const int shouldExecCnt = totalCnt - skipCnt;
  const int failedCnt = shouldExecCnt - succeedCnt;
  qWarning("%d/%d testcase passed (total:%d, skipCnt:%d)", succeedCnt, shouldExecCnt, totalCnt, skipCnt);
  if (failedCnt != 0) {
    qCritical("ERROR: %d TEST(s) FAILED", failedCnt);
  }
  return failedCnt;
}
