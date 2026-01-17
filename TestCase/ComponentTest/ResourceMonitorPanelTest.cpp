#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "ResourceMonitorPanel.h"
#include "EndToExposePrivateMember.h"
#include "PublicVariable.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>

USING_MOCKCPP_NS

namespace {
 QString gTimeArray;
}

void get_timestamp(char* buf, size_t len);
void invoke_get_timestamp(char* buf, size_t len) {
  memcpy_s(buf, len, gTimeArray.toStdString().c_str(), gTimeArray.size());
}

class ResourceMonitorPanelTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void init() {
    GlobalMockObject::reset();
  }

  void cleanup() {
    GlobalMockObject::verify();
  }

  void sample_period_changed_ok() {
    ResourceMonitorPanel rmp;
    QCOMPARE(rmp.GetInterval(), 1000); // default sample period: 1000ms
    QVERIFY(!rmp.IsPaused());
    emit rmp.mInterval->valueChanged(1000); // unchange
    QCOMPARE(rmp.mTimer.interval(), 1000);
    QVERIFY(rmp.mTimer.isActive());
    QVERIFY(rmp.mIntervalLabel->text().contains("1000"));

    rmp.mInterval->setValue(5000);
    emit rmp.mInterval->valueChanged(5000); // changed
    QCOMPARE(rmp.mTimer.interval(), 5000);
    QVERIFY(rmp.mTimer.isActive());
    QVERIFY(rmp.mIntervalLabel->text().contains("5000"));

    // period:0 (stop sample right now)
    rmp.mInterval->setValue(0);
    emit rmp.mInterval->valueChanged(0);
    QCOMPARE(rmp.mTimer.interval(), 0);
    QVERIFY(!rmp.mTimer.isActive());
    QVERIFY(rmp.mIntervalLabel->text().contains("0"));
    // reopen timer
    rmp.mInterval->setValue(8000);
    emit rmp.mInterval->valueChanged(8000); // changed
    QCOMPARE(rmp.mTimer.interval(), 8000);
    QVERIFY(rmp.mTimer.isActive());
    QVERIFY(rmp.mIntervalLabel->text().contains("8000"));
  }

  void cpu_memory_switch_control_ok() {
    MOCKER(get_timestamp).stubs().will(invoke(invoke_get_timestamp));
    gTimeArray = "2026/01/31 00:00:00.123";

    ResourceMonitorPanel rmp;
    QVERIFY(!rmp.IsPaused());
    QCOMPARE(rmp.mExportCSVName, "usage_monitor_2026/01/31 00:00:00.123");

    QVERIFY(rmp.mCpuSwitch->isChecked());
    QVERIFY(rmp.mMemorySwitch->isChecked());
    // uncheck both of them
    rmp.mCpuSwitch->setChecked(false);
    rmp.mMemorySwitch->setChecked(false);
    emit rmp.mCpuSwitch->toggled(false);
    emit rmp.mMemorySwitch->toggled(false);
    QVERIFY(!rmp.mCpuChartView->isVisible());
    QVERIFY(!rmp.mMemoryChartView->isVisible());

    gTimeArray = "2026/01/31 23:59:59.123";
    rmp.onTimeout();
    const QString& contentStr = QString::fromUtf8(rmp.mUsageReports);

    // expect: "Time,Memory(kB),CPU(%)\n2026/01/31 23:59:59.123,0,0\n"
    // actual: "Time,Memory(kB),CPU(%)\n2026/01/31 23:59:59.123,2.9e-29,0\n"
    const QString expectPrefix{"Time,Memory(kB),CPU(%)\n2026/01/31 23:59:59.123,"};
    QVERIFY(contentStr.startsWith(expectPrefix));
    const QString& twoFloatsStr = contentStr.mid(expectPrefix.size());
    const QStringList& twoFloats = twoFloatsStr.split(',', Qt::SplitBehaviorFlags::SkipEmptyParts);
    QCOMPARE(twoFloats.size(), 2);

    bool isFloat{false};
    float memoryUsed = twoFloats[0].toFloat(&isFloat);
    QVERIFY(isFloat);
    QVERIFY(std::abs(memoryUsed) < 1E-6);

    isFloat = false;
    float cpuUsage = twoFloats[1].toFloat(&isFloat);
    QVERIFY(isFloat);
    QVERIFY(std::abs(cpuUsage) < 1E-6);
  }

  void export_ok() {
    QByteArray content;
    const QString expectCsvAbsFilePath{SystemPath::HOME_PATH() //
                                       + "/Downloads/" //
                                       + "usage_monitor_2026_01_01 00_00_00.123_to_2026_01_01 00_00_02.456.csv"};
    MOCKER(get_timestamp).stubs().will(invoke(invoke_get_timestamp));
    MOCKER(QFileDialog::saveFileContent).expects(once()).with(spy(content), eq(expectCsvAbsFilePath));
    gTimeArray = "2026/01/01 00:00:00.123";

    ResourceMonitorPanel rmp;
    QCOMPARE(rmp.GetInterval(), 1000); // default sample period: 1000 ms
    gTimeArray = "2026/01/01 00:00:01.000";
    rmp.onTimeout();
    gTimeArray = "2026/01/01 00:00:02.000";
    rmp.onTimeout();
    gTimeArray = "2026/01/01 00:00:02.456";

    rmp.onExportUsageToLocalFile();
    const QString contentStr = QString::fromUtf8(content);
    QVERIFY(contentStr.startsWith("Time,Memory(kB),CPU(%)\n"));
    QVERIFY(contentStr.contains("\n2026/01/01 00:00:01.000,"));
    QVERIFY(contentStr.contains("\n2026/01/01 00:00:02.000,"));
    QVERIFY(contentStr.endsWith("\n"));
    QCOMPARE(contentStr.count('\n'), 1+2); // 1行表头, 两行记录, 共计3个

     // replace '/' and ':' with '_' after FillReportFileEndTime called
    QCOMPARE(rmp.mExportCSVName, "usage_monitor_2026/01/01 00:00:02.456");
    QCOMPARE(rmp.mUsageReports, "Time,Memory(kB),CPU(%)\n");
  }
};

#include "ResourceMonitorPanelTest.moc"
REGISTER_TEST(ResourceMonitorPanelTest, false)
