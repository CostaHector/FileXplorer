#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ResourceMonitor.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"

class ResourceMonitorTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir tDir;
  const QString csvLocatedIn{tDir.path()};
private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());
  }

  void test_ResourceMonitor_BasicFunction() {
    // 1. 创建 ResourceMonitor 对象
    ResourceMonitor resMonitor{csvLocatedIn};
    QVERIFY(resMonitor); // 验证对象有效
    const QString csvFileName = resMonitor.GetCsvFileName();
    QVERIFY(tDir.exists(csvFileName));
    QVERIFY(csvFileName.endsWith(".csv", Qt::CaseInsensitive));

    // 获取初始文件大小
    QFileInfo csvFi(resMonitor.mCsvFile);
    const qint64 initialSize = csvFi.size();
    QVERIFY(initialSize > 0); // 初始大小应大于0（包含标题行）

    // 2. 第一次手动调用测量函数, 验证文件大小增长
    resMonitor.onStart();
    resMonitor.onMeasureUsage();
    csvFi.refresh();
    const qint64 sizeAfterFirstCall = csvFi.size();
    QVERIFY2(sizeAfterFirstCall > initialSize, "File size should increase after first measurement");

    // 3. 第二次手动调用测量函数, 验证文件大小继续增长
    resMonitor.onMeasureUsage();
    csvFi.refresh();
    const qint64 sizeAfterSecondCall = csvFi.size();
    QVERIFY2(sizeAfterSecondCall > sizeAfterFirstCall, "File size should increase after second measurement");

    // 4. 验证文件内容格式
    bool bReadOk{false};
    const QByteArray contentBA = tDir.readByteArray(csvFileName, &bReadOk);
    QVERIFY(bReadOk);
    const QString content = QString::fromUtf8(contentBA);
    QVERIFY(content.startsWith("Timestamp,CPU(%),Memory(KB)"));
    const int newLineCnt = content.count("\n");
    QCOMPARE(newLineCnt, 1 + 2); // 1标题 + 2次测量

    // 6. 停用
    resMonitor.onStop();
  }
};

#include "ResourceMonitorTest.moc"
REGISTER_TEST(ResourceMonitorTest, false)
