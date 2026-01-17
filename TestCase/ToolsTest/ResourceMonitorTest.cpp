#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "ResourceMonitor.h"
using namespace ResourceMonitor;

class ResourceMonitorTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void GetNumbersCore_ok() {
    QVERIFY(GetNumbersCore() >= 1);
  }
  void getMemoryUsage_ok() {
    QVERIFY(getMemoryUsage(true) > 0);
    QVERIFY(getMemoryUsage(false) > 0);
  }
  void getgetProcessCpuTime_ok() {
    QVERIFY(getProcessCpuTime() > 0);
  }
};

#include "ResourceMonitorTest.moc"
REGISTER_TEST(ResourceMonitorTest, false)
