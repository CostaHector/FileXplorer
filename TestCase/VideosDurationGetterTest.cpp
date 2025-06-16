#include <QCoreApplication>
#include <QtTest>

#include "pub/MyTestSuite.h"
#include <chrono>
#ifdef _WIN32
#include "Tools/QMediaInfo.h"
#endif

const QString VIDEOS_DURATION_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_VideosDurationGetter");

class VideosDurationGetterTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_duration_of_mp4_files() {
    QStringList mp4VidsLst;
    mp4VidsLst << "Big Buck Bunny (Project Peach) Official Trailer (2008, The Blender Foundation) 144p 33s.mp4"
               << "Big Buck Bunny (Project Peach) Official Trailer (2008, The Blender Foundation) 360p 33s.mp4"
               << "Big Buck Bunny (Project Peach) Official Trailer (2008, The Blender Foundation) 720p 33s.mp4"
               << "Big Buck Bunny sample - XenForo community 720p 26s.mp4"
               << "Big Buck Bunny SampleVideo_360x240_1mb 13s.mp4";
    QList<int> durationsLst{33, 33, 33, 26, 13};
    // precondition 1: "list length should same"
    QCOMPARE(durationsLst.size(), mp4VidsLst.size());
    // precondition 2: "video file should exist"
    const QDir mp4Dir{VIDEOS_DURATION_DIR};
    foreach (const QString mp4FileName, mp4VidsLst) {
      QVERIFY2(mp4Dir.exists(mp4FileName), qPrintable(mp4FileName));
    }

#ifdef _WIN32
    QMediaInfo mi;
    QVERIFY2(mi.StartToGet(), "Should start succeed");
    for (int i = 0; i < mp4VidsLst.size(); ++i) {
      const QString& name = mp4VidsLst[i];
      const QString& vidPath = mp4Dir.absoluteFilePath(name);
      const int duration = mi.VidDurationLengthQuick(vidPath);  // msec
      // duration should not be zero
      QVERIFY2((duration > 0), qPrintable(name));
      // duration differs value should not large than 3s
      QVERIFY2((std::abs(duration / 1000 - durationsLst[i]) < 3), qPrintable(vidPath));
    }
#endif
  }

  void test_all_video_types() {
    QStringList mp4VidsLst;
    mp4VidsLst << "Big Buck Bunny SampleVideo_320x240_1mb 15s.3gp"
               << "Big Buck Bunny SampleVideo_360x240_1mb 9s.mkv"
               << "Big Buck Bunny SampleVideo_360x240_1mb 10s.flv"
               << "Big Buck Bunny SampleVideo_360x240_1mb 13s.mp4";
    QList<int> durationsLst{15, 9, 10, 13};
    // precondition 1: "list length should same"
    QCOMPARE(durationsLst.size(), mp4VidsLst.size());
    // precondition 2: "video file should exist"
    const QDir mp4Dir{VIDEOS_DURATION_DIR};
    foreach (const QString mp4FileName, mp4VidsLst) {
      QVERIFY2(mp4Dir.exists(mp4FileName), qPrintable(mp4FileName));
    }
#ifdef _WIN32
    QMediaInfo mi;
    QVERIFY2(mi.StartToGet(), "Should start succeed");
    for (int i = 0; i < mp4VidsLst.size(); ++i) {
      const QString& name = mp4VidsLst[i];
      const QString& vidPath = mp4Dir.absoluteFilePath(name);
      const int duration = mi.VidDurationLengthQuick(vidPath);  // msec
      // duration should not be zero
      QVERIFY2((duration > 0), qPrintable(name));
      // duration differs value should not large than 3s
      QVERIFY2((std::abs(duration / 1000 - durationsLst[i]) < 3), qPrintable(vidPath));
    }
#endif
  }

  //  void test_compareSpeed() {
  //    auto traditionWay = [dir = mp4Dir](int loopCount = 1) -> int {
  //      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  //      const QStringList& names = dir.entryList();
  //      QMediaInfo mi;
  //      for (int ind = 0; ind < loopCount; ++ind) {
  //        for (const QString& name : names) {
  //          const QString& vidPath = dir.absoluteFilePath(name);
  //          mi.VidDurationLength(vidPath);
  //        }
  //      }
  //      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  //      return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
  //    };
  //    auto quickWay = [dir = mp4Dir](int loopCount = 1) -> int {
  //      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  //      const QStringList& names = dir.entryList();
  //      QMediaInfo mi;
  //      mi.StartToGet();
  //      for (int ind = 0; ind < loopCount; ++ind) {
  //        for (const QString& name : names) {
  //          const QString& vidPath = dir.absoluteFilePath(name);
  //          mi.VidDurationLengthQuick(vidPath);
  //        }
  //      }
  //      mi.EndToGet();
  //      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  //      return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
  //    };
  //    int tradCost = traditionWay(100);
  //    int quickCost = quickWay(100);
  //    QVERIFY(quickCost <= tradCost);
  //  }
};

#include "VideosDurationGetterTest.moc"
VideosDurationGetterTest g_VideosDurationGetterTest;
