#include <QCoreApplication>
#include <QtTest>

#include "PublicVariable.h"
#include "Tools/QMediaInfo.h"
#include <chrono>

const QString VIDEOS_DURATION_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_VideosDurationGetter/TYPES");
const QString VIDEOS_DURATION_MP4_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_VideosDurationGetter/MP4");

class VideosDurationGetterTest : public QObject {
  Q_OBJECT
 public:
  QDir typesDir{VIDEOS_DURATION_DIR, "", QDir::SortFlag::NoSort, QDir::Filter::Files};
  QDir mp4Dir{VIDEOS_DURATION_MP4_DIR, "", QDir::SortFlag::NoSort, QDir::Filter::Files};
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}

  void init() {
    typesDir.setNameFilters(TYPE_FILTER::AI_DUP_VIDEO_TYPE_SET);
    mp4Dir.setNameFilters(TYPE_FILTER::AI_DUP_VIDEO_TYPE_SET);
  }
  void cleanup() {}

  void test_Basic() {
    const QStringList& names = typesDir.entryList();
    QVERIFY2(!names.isEmpty(), "should contains some videos in this path");

    QMediaInfo mi;
    QVERIFY2(mi.StartToGet(), "Should start succeed");
    for (const QString& name : names) {
      const QString& vidPath = typesDir.absoluteFilePath(name);
      QVERIFY2(mi.VidDurationLengthQuick(vidPath) > 0, qPrintable(vidPath));
    }
    mi.EndToGet();
  }

  void test_OnlyMp4() {
    const QStringList& names = mp4Dir.entryList();
    QVERIFY2(!names.isEmpty(), "should contains some videos in this path");

    QMediaInfo mi;
    for (const QString& name : names) {
      const QString& vidPath = mp4Dir.absoluteFilePath(name);
      QVERIFY2(mi.VidDurationLength(vidPath) > 0, qPrintable(vidPath));
    }
  }

  void test_compareSpeed() {
    auto traditionWay = [dir = mp4Dir](int loopCount = 1) -> int {
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      const QStringList& names = dir.entryList();
      QMediaInfo mi;
      for (int ind = 0; ind < loopCount; ++ind) {
        for (const QString& name : names) {
          const QString& vidPath = dir.absoluteFilePath(name);
          mi.VidDurationLength(vidPath);
        }
      }
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    };
    auto quickWay = [dir = mp4Dir](int loopCount = 1) -> int {
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      const QStringList& names = dir.entryList();
      QMediaInfo mi;
      mi.StartToGet();
      for (int ind = 0; ind < loopCount; ++ind) {
        for (const QString& name : names) {
          const QString& vidPath = dir.absoluteFilePath(name);
          mi.VidDurationLengthQuick(vidPath);
        }
      }
      mi.EndToGet();
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    };
    int tradCost = traditionWay(100);
    int quickCost = quickWay(100);
    QVERIFY(quickCost <= tradCost);
  }
};

QTEST_MAIN(VideosDurationGetterTest)
#include "VideosDurationGetterTest.moc"
