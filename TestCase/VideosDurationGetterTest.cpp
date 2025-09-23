#include <QCoreApplication>
#include <QtTest>
#include <chrono>
#include "BeginToExposePrivateMember.h"
#include "EndToExposePrivateMember.h"
#include "VideoDurationGetter.h"
#include "PlainTestSuite.h"
#include "VideoTestPrecoditionTools.h"

class VideosDurationGetterTest : public PlainTestSuite {
  Q_OBJECT
 public:
  VideosDurationGetterTest() : PlainTestSuite{} {}
  static constexpr int EPSILON_MILLIONSECOND{1000};  // 1000ms
 private slots:
  void test_GetLengthQuick() {
    const QString vidName = "Big Buck Bunny (Project Peach) Official Trailer (2008, The Blender Foundation) 144p 33s.mp4";
    const int expectDuration = 33000;
    const QDir mp4Dir{VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH};
    QVERIFY(mp4Dir.exists(vidName));

    VideoDurationGetter mi;
    QVERIFY(mi.StartToGet());
    int actualDuration = mi.GetLengthQuick(mp4Dir.absoluteFilePath(vidName));
    QVERIFY2((std::abs(actualDuration - expectDuration) < EPSILON_MILLIONSECOND), qPrintable(vidName));
  }

  void test_GetLengthsQuick() {
    const QStringList vidsName{"Big Buck Bunny (Project Peach) Official Trailer (2008, The Blender Foundation) 144p 33s.mp4",
                               "Big Buck Bunny (Project Peach) Official Trailer (2008, The Blender Foundation) 360p 33s.mp4"};
    QStringList vidsAbsPath;
    const QList<int> expectDurations{33000, 33000};
    const QDir mp4Dir{VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH};
    for (const QString& vidName : vidsName) {
      QVERIFY(mp4Dir.exists(vidName));
      vidsAbsPath.append(mp4Dir.absoluteFilePath(vidName));
    }

    VideoDurationGetter mi;
    QVERIFY(mi.StartToGet());
    QList<int> actualDurations = mi.GetLengthsQuick(vidsAbsPath);
    QCOMPARE(actualDurations.size(), expectDurations.size());
    for (int i = 0; i < actualDurations.size(); ++i) {
      QVERIFY2((std::abs(actualDurations[i] - expectDurations[i]) < EPSILON_MILLIONSECOND), qPrintable(vidsName[i]));
    }
  }

  void test_mp4_type_video_duration() {
    QStringList mp4VidsLst, mp4AbsPath;
    mp4VidsLst << "Big Buck Bunny (Project Peach) Official Trailer (2008, The Blender Foundation) 144p 33s.mp4"
               << "Big Buck Bunny (Project Peach) Official Trailer (2008, The Blender Foundation) 360p 33s.mp4"
               << "Big Buck Bunny (Project Peach) Official Trailer (2008, The Blender Foundation) 720p 33s.mp4"
               << "Big Buck Bunny sample - XenForo community 720p 26s.mp4"
               << "Big Buck Bunny SampleVideo_360x240_1mb 13s.mp4";
    mp4AbsPath.reserve(mp4VidsLst.size());
    const QList<int> expectDurationsLst{33000, 33000, 33000, 26000, 13000};  // units: ms
    // precondition 1: "list length should same"
    QCOMPARE(expectDurationsLst.size(), 5);
    QCOMPARE(mp4VidsLst.size(), 5);

    // precondition 2: "video file(s) should exist"
    const QDir mp4Dir{VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH};
    foreach (const QString mp4FileName, mp4VidsLst) {
      QVERIFY2(mp4Dir.exists(mp4FileName), qPrintable(mp4FileName));
      mp4AbsPath.append(mp4Dir.absoluteFilePath(mp4FileName));
    }

#ifdef _WIN32
    // QMediaInfo windows platform only
    QMediaInfo mi;
    QVERIFY2(mi.StartToGet(), "Should start succeed");
    for (int i = 0; i < mp4VidsLst.size(); ++i) {
      const QString& name = mp4VidsLst[i];
      const QString& vidPath = mp4Dir.absoluteFilePath(name);
      const int duration = mi.VidDurationLengthQuick(vidPath);
      // duration should not be zero
      QVERIFY2((duration > 0), qPrintable(name));
      // duration differs value should not large than 1s
      QVERIFY2((std::abs(duration - expectDurationsLst[i]) < EPSILON_MILLIONSECOND), qPrintable(vidPath));
    }
#endif
    // VideoDurationGetter cross-platform
    const QList<int> durationsFromFFmpeg = VideoDurationGetter::ReadVideos(mp4AbsPath);
    for (int i = 0; i < mp4AbsPath.size(); ++i) {
      QVERIFY2((std::abs(durationsFromFFmpeg[i] - expectDurationsLst[i]) < EPSILON_MILLIONSECOND), qPrintable(mp4AbsPath[i]));
    }
  }

  void test_other_type_video_duration() {
    QStringList mp4VidsLst, mp4AbsPath;
    mp4VidsLst << "Big Buck Bunny SampleVideo_320x240_1mb 15s.3gp"
               << "Big Buck Bunny SampleVideo_360x240_1mb 9s.mkv"
               << "Big Buck Bunny SampleVideo_360x240_1mb 10s.flv"
               << "Big Buck Bunny SampleVideo_360x240_1mb 13s.mp4";
    mp4AbsPath.reserve(mp4VidsLst.size());
    const QList<int> expectDurationsLst{15000, 9000, 10000, 13000};  // units: ms

    // precondition 1: "list length should same"
    QCOMPARE(mp4VidsLst.size(), 4);
    QCOMPARE(expectDurationsLst.size(), 4);
    // precondition 2: "video file(s) should all exist"
    const QDir mp4Dir{VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH};
    foreach (const QString mp4FileName, mp4VidsLst) {
      QVERIFY2(mp4Dir.exists(mp4FileName), qPrintable(mp4FileName));
      mp4AbsPath.append(mp4Dir.absoluteFilePath(mp4FileName));
    }

#ifdef _WIN32
    // QMediaInfo windows platform only
    QMediaInfo mi;
    QVERIFY2(mi.StartToGet(), "Should start succeed");
    for (int i = 0; i < mp4AbsPath.size(); ++i) {
      const QString& vidPath = mp4AbsPath[i];
      const int duration = mi.VidDurationLengthQuick(vidPath);
      // duration should not be zero
      QVERIFY2((duration > 0), qPrintable(vidPath));
      // duration differs value should not large than 1s
      QVERIFY2((std::abs(duration - expectDurationsLst[i]) < EPSILON_MILLIONSECOND), qPrintable(vidPath));
    }
#endif
    // VideoDurationGetter cross-platform
    const QList<int> durationsFromFFmpeg = VideoDurationGetter::ReadVideos(mp4AbsPath);
    for (int i = 0; i < mp4AbsPath.size(); ++i) {
      QVERIFY2((std::abs(durationsFromFFmpeg[i] - expectDurationsLst[i]) < EPSILON_MILLIONSECOND), qPrintable(mp4AbsPath[i]));
    }
  }

  void test_compareSpeed() {
    QDir dir{VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH};
    auto traditionWay = [&dir]() -> int {
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      const QStringList& names = dir.entryList();

      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    };
    auto quickWay = [&dir]() -> int {
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      const QStringList& names = dir.entryList();

      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    };
    int tradCost = traditionWay();
    int quickCost = quickWay();
    QVERIFY(tradCost >= 0);
    QVERIFY(quickCost >= 0);
  }
};
constexpr int VideosDurationGetterTest::EPSILON_MILLIONSECOND;

#include "VideosDurationGetterTest.moc"
REGISTER_TEST(VideosDurationGetterTest, false)
