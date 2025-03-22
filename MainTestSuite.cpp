#include "TestCase/pub/MyTestSuite.h"
#include <QtTest/QtTest>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  // run suite
  auto& suite = MyTestSuite::suite();
  const int totalTestSuiteCnt = suite.size();
  qDebug("suite.size() = %d", totalTestSuiteCnt);
  int succeedCnt = 0;
  for (auto it = suite.begin(); it != suite.end(); ++it) {
    if (QTest::qExec(*it, argc, argv) == 0) {
      ++succeedCnt;
    }
  }
  if (succeedCnt < totalTestSuiteCnt) {
    qWarning("%d/%d testcase passed", succeedCnt, totalTestSuiteCnt);
  } else {
    qDebug("All %d/%d testcase passed", succeedCnt, totalTestSuiteCnt);
  }
  return totalTestSuiteCnt - succeedCnt;
}
