#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "VidsDurationDisplayString.h"
#include "DataFormatter.h"

class VidsDurationDisplayStringTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_VideosDurationDetailHtmlTable() {
    QList<int> durations = {1000};  // 1秒
    QStringList fileNames = {"test.mp4"};
    QStringList fileDirs = {"/test/path"};
    QString result = VidsDurationDisplayString::VideosDurationDetailHtmlTable(durations, fileNames, fileDirs);

    QVERIFY(result.contains("Total duration: 1(s) of 1 video(s)"));
    QVERIFY(result.contains(">test.mp4</td>"));
    QVERIFY(result.contains(">/test/path</td>"));
    QVERIFY(result.contains(DataFormatter::formatDurationISOMs(1000)));
    QVERIFY(result.contains("<table>"));
    QVERIFY(result.contains("</table>"));
  }

  void displayVideosDuration_ok() {
    QStringList fileAbsPaths = {"/test/path/video.mp4"};
    QString result = VidsDurationDisplayString::DisplayVideosDuration({2000}, fileAbsPaths);
    QVERIFY(result.contains("Total duration: 2(s) of 1 video(s)"));
    QVERIFY(result.contains(">video.mp4</td>"));
    QVERIFY(result.contains("/test/path"));
    QVERIFY(result.contains(DataFormatter::formatDurationISOMs(2000)));
  }

  void emptyInput_ok() {
    QString result = VidsDurationDisplayString::DisplayVideosDuration({}, {});
    QVERIFY(result.contains("Total duration: 0(s) of 0 video(s)"));
  }
};

#include "VidsDurationDisplayStringTest.moc"
REGISTER_TEST(VidsDurationDisplayStringTest, false)
