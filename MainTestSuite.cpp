#include "TestCase/pub/MyTestSuite.h"
#include <QtTest/QtTest>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  // run suite
  auto &suite = MyTestSuite::suite();
  qDebug("suite.size() = %d", suite.size());
  int status = 0;
  for (auto it = suite.begin(); it != suite.end(); ++it) {
    status |= QTest::qExec(*it, argc, argv);
  }
  return status;
}
