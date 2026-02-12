#include "PlainTestSuite.h"
int PlainTestSuite::mTotalTestCaseCount{0};

PlainTestSuite::PlainTestSuite(QObject *parent): QObject{parent} {}

int PlainTestSuite::AddATestCase(FTestCaseObjCreater fObjCrt, bool bExclusive, const QString& locatedIn) {
  if (bExclusive) {
    exclusiveSuite().push_back({fObjCrt, locatedIn.mid(PARENT_PATH_LEN)});
  } else {
    sharedSuite().push_back({fObjCrt, locatedIn.mid(PARENT_PATH_LEN)});
  }
  return ++mTotalTestCaseCount;
}

QVector<TestCaseHelper>& PlainTestSuite::sharedSuite() {
  static QVector<TestCaseHelper> sharedSuiteCrtInst;
  return sharedSuiteCrtInst;
}

QVector<TestCaseHelper>& PlainTestSuite::exclusiveSuite() {
  static QVector<TestCaseHelper> exclusiveSuiteCrtInst;
  return exclusiveSuiteCrtInst;
}
