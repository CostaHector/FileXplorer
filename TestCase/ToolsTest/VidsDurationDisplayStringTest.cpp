#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "VidsDurationDisplayString.h"
#include "DataFormatter.h"

class VidsDurationDisplayStringTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  // 测试核心HTML表格生成功能
  void test_VideosDurationDetailHtmlTable() {
    // 准备最小测试数据
    QList<int> durations = {1000}; // 1秒
    QStringList fileNames = {"test.mp4"};
    QStringList fileDirs = {"/test/path"};

    // 调用函数
    QString result = VidsDurationDisplayString::VideosDurationDetailHtmlTable(durations, fileNames, fileDirs);

    // 验证核心结果
    QVERIFY(result.contains("Total duration: 1(s) of 1 video(s)"));
    QVERIFY(result.contains(">test.mp4</td>"));
    QVERIFY(result.contains(">/test/path</td>"));
    QVERIFY(result.contains(DataFormatter::formatDurationISOMs(1000)));
    QVERIFY(result.contains("<table>"));
    QVERIFY(result.contains("</table>"));
  }

  // 测试完整工作流程
  void test_DisplayVideosDuration() {
    // 准备最小测试数据
    QStringList fileAbsPaths = {"/test/path/video.mp4"};

    // 设置Mock数据
    VidsDurationDisplayString::MockFilesDurationLstReturn() = {2000};

    // 调用函数
    QString result = VidsDurationDisplayString::DisplayVideosDuration(fileAbsPaths);

    // 验证核心结果
    QVERIFY(result.contains("Total duration: 2(s) of 1 video(s)"));
    QVERIFY(result.contains(">video.mp4</td>"));
    QVERIFY(result.contains("/test/path"));
    QVERIFY(result.contains(DataFormatter::formatDurationISOMs(2000)));
  }

  // 测试空输入情况
  void test_EmptyInput() {
    // 测试空输入
    VidsDurationDisplayString::MockFilesDurationLstReturn() = {};
    QString result = VidsDurationDisplayString::DisplayVideosDuration({});
    QVERIFY(result.contains("Total duration: 0(s) of 0 video(s)"));
  }
};

#include "VidsDurationDisplayStringTest.moc"
REGISTER_TEST(VidsDurationDisplayStringTest, false)
