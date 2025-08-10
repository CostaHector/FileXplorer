#include <QCoreApplication>
#include <QtTest>
#include "PathRelatedTool.h"
#include "MyTestSuite.h"
#include "TDir.h"
#include "OnScopeExit.h"
#include "BeginToExposePrivateMember.h"
#include "ThumbnailProcesser.h"
#include "EndToExposePrivateMember.h"

#include <QImage>
bool CreateAndSaveAWhitePng(const QString& filePath, int width = 1440, int height = 1080) {
  // 创建指定大小的白色图像（ARGB32格式）
  QImage image{width, height, QImage::Format_ARGB32};
  // 填充白色（RGBA：255,255,255,255）
  image.fill(Qt::white);
  if (!image.save(filePath)) {
    qWarning("Failed to save image to file[%s]", qPrintable(filePath));
    return false;
  }
  return true;
}

const QString VIDEOS_DURATION_DIR = TestCaseRootPath() + "/test/TestEnv_VideosDurationGetter";

class ThumbnailProcesserTest : public MyTestSuite {
  Q_OBJECT
 public:
  ThumbnailProcesserTest() : MyTestSuite{false} {}
 private slots:
  void cleanupTestCase() {}

  void test_no_video_need_to_create_thumbnail_imgs() {
    QStringList invalidVideosPath{"", "inexist file", __FILE__};
    QCOMPARE(ThumbnailProcesser::CreateThumbnailImages(invalidVideosPath, 2, 2, 720), 0);
  }

  void test_2_videos_need_to_create_thumbnail_imgs() {
    QDir dir{VIDEOS_DURATION_DIR};
    QVERIFY(dir.exists("Big Buck Bunny SampleVideo_360x240_1mb 9s.mkv"));
    QVERIFY(dir.exists("Big Buck Bunny SampleVideo_360x240_1mb 10s.flv"));
    ON_SCOPE_EXIT {
      if (dir.exists("Big Buck Bunny SampleVideo_360x240_1mb 9s 22.jpg")) {
        QVERIFY(dir.remove("Big Buck Bunny SampleVideo_360x240_1mb 9s 22.jpg"));
      }
      if (dir.exists("Big Buck Bunny SampleVideo_360x240_1mb 10s 22.jpg")) {
        QVERIFY(dir.remove("Big Buck Bunny SampleVideo_360x240_1mb 10s 22.jpg"));
      }
      if (dir.exists("Big Buck Bunny SampleVideo_360x240_1mb 9s 13.png")) {
        QVERIFY(dir.remove("Big Buck Bunny SampleVideo_360x240_1mb 9s 13.png"));
      }
      if (dir.exists("Big Buck Bunny SampleVideo_360x240_1mb 10s 13.png")) {
        QVERIFY(dir.remove("Big Buck Bunny SampleVideo_360x240_1mb 10s 13.png"));
      }
    };
    QStringList validVideosPath{"",                                                                     //
                                dir.absoluteFilePath("Big Buck Bunny SampleVideo_360x240_1mb 9s.mkv"),  //
                                dir.absoluteFilePath("Big Buck Bunny SampleVideo_360x240_1mb 10s.flv")};
    QCOMPARE(ThumbnailProcesser::CreateThumbnailImages(validVideosPath, 2, 2, 720, 2, true), 2);
    QVERIFY(dir.exists("Big Buck Bunny SampleVideo_360x240_1mb 9s 22.jpg"));
    QVERIFY(dir.exists("Big Buck Bunny SampleVideo_360x240_1mb 10s 22.jpg"));
    QCOMPARE(ThumbnailProcesser::CreateThumbnailImages(validVideosPath, 1, 3, 720, 2, false), 2);
    QVERIFY(dir.exists("Big Buck Bunny SampleVideo_360x240_1mb 9s 13.png"));
    QVERIFY(dir.exists("Big Buck Bunny SampleVideo_360x240_1mb 10s 13.png"));
  }

  void test_IsImageNameLooksLikeThumbnail_ok() {
    QVERIFY(ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 22"));
    QVERIFY(ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 33"));
    QVERIFY(ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 44"));

    QVERIFY(ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 21"));
    QVERIFY(ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 31"));
    QVERIFY(ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 41"));
    QVERIFY(ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 51"));
    QVERIFY(ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 61"));
    QVERIFY(ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 71"));
    QVERIFY(ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 81"));
  }

  void test_IsImageNameLooksLikeThumbnail_no() {
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 00"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 11"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 12"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 13"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 14"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 23"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 24"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image thumbnail"));
  }

  void test_CutThumbnails9ImgsOk() {
    TDir mDir;
    const QString rootpath{mDir.path()};
    QVERIFY(CreateAndSaveAWhitePng(rootpath + "/Images 33.png"));
    QVERIFY(mDir.fileExists("Images 33.png"));

    ThumbnailProcesser itp{true};
    int extractedImgsOut9 = itp(rootpath, 0, 9);

    QCOMPARE(extractedImgsOut9, 9);
    QVERIFY(itp.mErrImg.isEmpty());

    int extractedImgsOut0 = itp(rootpath, 0, 1);
    QCOMPARE(extractedImgsOut0, 0);
    QCOMPARE(itp.mRewriteImagesCnt, 0);
    QVERIFY(itp.mErrImg.isEmpty());  // 1 skip

    ThumbnailProcesser itp3{false};
    int extractedImgsOut1 = itp3(rootpath, 0, 4);
    QCOMPARE(extractedImgsOut1, 4);  // 4 extract alse rewrite
    QCOMPARE(itp3.mRewriteImagesCnt, 4);
    QCOMPARE(itp3.mErrImg.size(), 4);  // 4 rewrite msg
  }
};

#include "ThumbnailProcesserTest.moc"
ThumbnailProcesserTest g_ThumbnailProcesserTest;
