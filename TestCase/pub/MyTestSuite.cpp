#include "MyTestSuite.h"

MyTestSuite::MyTestSuite() : QObject{} {
  suite().push_back(this);
}

QVector<QObject*>& MyTestSuite::suite() {
  static QVector<QObject*> testSuiteInst;
  return testSuiteInst;
}
