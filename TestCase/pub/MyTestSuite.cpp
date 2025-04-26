#include "MyTestSuite.h"

bool MyTestSuite::bOnlyExecuteExculsive = false;

MyTestSuite::MyTestSuite(bool bExculsive)  //
    : QObject{}, mExclusive{bExculsive} {
  if (mExclusive) {
    bOnlyExecuteExculsive = true;
  }
  suite().push_back(this);
}

QVector<MyTestSuite*>& MyTestSuite::suite() {
  static QVector<MyTestSuite*> testSuiteInst;
  return testSuiteInst;
}
