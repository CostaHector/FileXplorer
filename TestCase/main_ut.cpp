#include "PlainTestSuite.h"
#include "Logger.h"
#include <QtTest/QtTest>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  QVector<TestCaseHelper>& sharedSuite = PlainTestSuite::sharedSuite();
  QVector<TestCaseHelper>& exclusiveSuite = PlainTestSuite::exclusiveSuite();
  const QVector<TestCaseHelper>* p2Suite = nullptr;
  if (exclusiveSuite.isEmpty()) {
    p2Suite = &sharedSuite;
  } else {
    p2Suite = &exclusiveSuite;
  }
  QStringList failedTestCaseNames;
  for (const TestCaseHelper& testHelper: *p2Suite) {
    std::unique_ptr<QObject> pTestObj(testHelper.pCreater());
    if (pTestObj == nullptr) {
      failedTestCaseNames.push_back("new testcase but return a nullptr:" + testHelper.locatedIn);
      continue;
    }
    if (QTest::qExec(pTestObj.get(), argc, argv) != 0) {
      failedTestCaseNames.push_back(pTestObj->metaObject()->className());
    }
  }
  const int totalCnt = PlainTestSuite::mTotalTestCaseCount;
  const int shouldExecCnt = p2Suite->size();
  const int failedCnt = failedTestCaseNames.size();
  const int skippedCnt = totalCnt - shouldExecCnt;
  if (failedCnt != 0) {
    fprintf(stderr, "\n----------Following %d/%d TEST(s) Failed (%d skipped)----------\n", failedCnt, shouldExecCnt, skippedCnt);
    for (int i = 0; i < failedCnt; ++i) {
      fprintf(stderr, "%3d. %s\n", i+1, qPrintable(failedTestCaseNames[i]));
    }
    fprintf(stderr, "\n----------Following %d/%d TEST(s) Failed (%d skipped)----------\n", failedCnt, shouldExecCnt, skippedCnt);
    fflush(stderr);
  } else {
    fprintf(stdout, "\n----------OK----------OK----------OK----------\n"
                    "All %d TEST(s) PASSED (%d skipped)"
                    "\n----------OK----------OK----------OK----------\n", shouldExecCnt, skippedCnt);
    fflush(stdout);
  }

  // explicit clear two testcase lambdas vector
  sharedSuite.clear();
  exclusiveSuite.clear();
  return failedTestCaseNames.size();
}
