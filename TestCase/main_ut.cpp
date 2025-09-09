#include "PlainTestSuite.h"
#include "Logger.h"
#include <QtTest/QtTest>

#define RUN_UT_MAIN_FILE 1
#ifdef RUN_UT_MAIN_FILE

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
  for (const auto& testHelper: *p2Suite) {
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
  if (failedCnt != 0) {
    LOG_C("\n---------------ERROR: Following %d/%d TEST(s) FAILED (total:%d):---------------", failedCnt, shouldExecCnt, totalCnt);
    for (int i = 0; i < failedCnt; ++i) {
      LOG_C("%3d. %s", i+1, qPrintable(failedTestCaseNames[i]));
    }
    LOG_C("\n---------------ERROR: Above %d/%d TEST(s) FAILED (total:%d)---------------", failedCnt, shouldExecCnt, totalCnt);
  } else {
    LOG_W("\n--OK----------OK----------OK--\n"
          "All %d/%d TEST(s) PASSED (total:%d)"
          "\n--OK----------OK----------OK--\n", shouldExecCnt, shouldExecCnt, totalCnt);
  }
  return failedTestCaseNames.size();
}

#endif
