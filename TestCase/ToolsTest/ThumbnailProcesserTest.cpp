#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "TDir.h"
#include "Logger.h"

#include "BeginToExposePrivateMember.h"
#include "ThumbnailProcesser.h"
#include "EndToExposePrivateMember.h"
#include "VideoTestPrecoditionTools.h"
#include "ImageTestPrecoditionTools.h"
#include "CastBrowserHelper.h"

#define MP4_33_VID_BASE_NAME "Big Buck Bunny (Project Peach) Official Trailer (2008, The Blender Foundation) 720p 33s"
#define FLV_10_VID_BASE_NAME "Big Buck Bunny SampleVideo_360x240_1mb 10s"

bool ColorsApproximatelyEqual(quint32 actualColor, quint32 expectColor, int tolerance = 5) {
  int r1 = (actualColor >> 16) & 0xFF;
  int g1 = (actualColor >> 8) & 0xFF;
  int b1 = actualColor & 0xFF;

  int r2 = (expectColor >> 16) & 0xFF;
  int g2 = (expectColor >> 8) & 0xFF;
  int b2 = expectColor & 0xFF;

  const bool approEqual = (abs(r1 - r2) <= tolerance) && (abs(g1 - g2) <= tolerance) && (abs(b1 - b2) <= tolerance);
  if (!approEqual) {
    LOG_D("actual: 0x06x, expect: 0x06x", actualColor, expectColor);
  }
  return approEqual;
}

class ThumbnailProcesserTest : public PlainTestSuite {
  Q_OBJECT
public:
  QDir dir{VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH};
  TDir tDir;
  static constexpr int MKV_WIDTH = 480;
  static constexpr int MKV_HEIGHT = 360;
private slots:
  void initTestCase() {
    QVERIFY(dir.exists(MP4_33_VID_BASE_NAME ".mp4"));
    QVERIFY(dir.exists(FLV_10_VID_BASE_NAME ".flv"));
    QVERIFY(tDir.IsValid());
  }

  void cleanupTestCase() {
    dir.setNameFilters({"Big Buck Bunny*.jpg", "Big Buck Bunny*.png"});
    for (const QString& img : dir.entryList()) {
      dir.remove(img);
    }
  }

  void test_CheckParameters() {
    QVERIFY(ThumbnailProcesser::CheckParameters(1, 1, 360));
    QVERIFY(ThumbnailProcesser::CheckParameters(1, 1, 480));
    QVERIFY(ThumbnailProcesser::CheckParameters(1, 1, 720));
    QVERIFY(ThumbnailProcesser::CheckParameters(1, 1, 1080));

    QVERIFY(ThumbnailProcesser::CheckParameters(3, 4, 360));
    QVERIFY(ThumbnailProcesser::CheckParameters(6, 8, 480));
    QVERIFY(ThumbnailProcesser::CheckParameters(2, 9, 720));
    QVERIFY(ThumbnailProcesser::CheckParameters(9, 4, 1080));

    QVERIFY(ThumbnailProcesser::CheckParameters(9, 9, 360));
    QVERIFY(ThumbnailProcesser::CheckParameters(9, 9, 480));
    QVERIFY(ThumbnailProcesser::CheckParameters(9, 9, 720));
    QVERIFY(ThumbnailProcesser::CheckParameters(9, 9, 1080));
  }

  void test_not_video_need_to_create_thumbnail_imgs() {
    QStringList invalidVideosPath{"", "inexist file", __FILE__};
    ThumbnailProcesser tp{true};
    QCOMPARE(tp.CreateThumbnailImages(invalidVideosPath, 2, 2, 720), 0);
  }

  void test_2_videos_need_to_create_thumbnail_imgs() {
    QStringList validVideosPath{"",                                                //
                                dir.absoluteFilePath(MP4_33_VID_BASE_NAME ".mp4"), //
                                dir.absoluteFilePath(FLV_10_VID_BASE_NAME ".flv")};
    ThumbnailProcesser tp{true};
    {
      QCOMPARE(tp.CreateThumbnailImages(validVideosPath, 2, 2, 720, true), 2);
      QVERIFY(dir.exists(MP4_33_VID_BASE_NAME " 22.jpg"));
      QVERIFY(dir.exists(FLV_10_VID_BASE_NAME " 22.jpg"));
      QCOMPARE(CastBrowserHelper::GetImageSize(dir.absoluteFilePath(MP4_33_VID_BASE_NAME " 22.jpg")).width(), 720 * 2);
    }

    {
      QCOMPARE(tp.CreateThumbnailImages(validVideosPath, 1, 3, 480, false), 2);
      QVERIFY(dir.exists(MP4_33_VID_BASE_NAME " 13.png"));
      QVERIFY(dir.exists(FLV_10_VID_BASE_NAME " 13.png"));
      QCOMPARE(CastBrowserHelper::GetImageSize(dir.absoluteFilePath(FLV_10_VID_BASE_NAME " 13.png")).width(), 480 * 3);
    }
  }

#define MKV_SEGEMENTS_BASE_NAME "Big Buck Bunny 5secondx3_RedGreenBlue"
  void screeshot_sample_time_correct() {
    tDir.ClearAll();

    const QString mkvPath = tDir.itemPath(MKV_SEGEMENTS_BASE_NAME ".mkv");
    bool bGenOk = false;
    QByteArray vidBa = VideoTestPrecoditionTools::CreateVideoFile(mkvPath,
                                                                  {0xFF0000, 0x00FF00, 0x0000FF},
                                                                  {5000, 5000, 5000},
                                                                  &bGenOk,
                                                                  MKV_WIDTH,
                                                                  MKV_HEIGHT);
    QVERIFY(bGenOk);
    QVERIFY(vidBa.size() > 0);

    ThumbnailProcesser itp4{true};
    QCOMPARE(itp4.CreateThumbnailImages(QStringList{mkvPath}, 3, 1, MKV_WIDTH, false), 1);
    const QString thumbnailImgPath = tDir.itemPath(MKV_SEGEMENTS_BASE_NAME " 31.png");
    const QImage thumbnailImg{thumbnailImgPath};
    QVERIFY(!thumbnailImg.isNull());
    QCOMPARE(thumbnailImg.width(), MKV_WIDTH * 1);
    QCOMPARE(thumbnailImg.height(), MKV_HEIGHT * 3);

    quint32 redRgb = ImageTestPrecoditionTools::GetPixelColorFromImage(thumbnailImg, 10, 10 + 0 * MKV_HEIGHT);
    quint32 blueRgb = ImageTestPrecoditionTools::GetPixelColorFromImage(thumbnailImg, 10, 10 + 2 * MKV_HEIGHT);
    QVERIFY(ColorsApproximatelyEqual(redRgb, 0xFF0000));
    QVERIFY(ColorsApproximatelyEqual(blueRgb, 0x0000FF));
  }
#undef MKV_SEGEMENTS_BASE_NAME

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
    tDir.ClearAll();

    const QString rootpath{tDir.path()};
    QVERIFY(ImageTestPrecoditionTools::CreateAndSaveAWhitePng(rootpath + "/Images 33.png"));
    QVERIFY(tDir.fileExists("Images 33.png"));

    {
      ThumbnailProcesser itp{true};
      int extractedImgsOut9 = itp(rootpath, 0, 9);

      QCOMPARE(extractedImgsOut9, 9);
      QVERIFY(itp.mErrImg.isEmpty());

      int extractedImgsOut0 = itp(rootpath, 0, 1);
      QCOMPARE(extractedImgsOut0, 0);
      QCOMPARE(itp.mRewriteImagesCnt, 0);
      QVERIFY(itp.mErrImg.isEmpty()); // 1 skip
    }

    {
      ThumbnailProcesser itp3{false};
      int extractedImgsOut1 = itp3(rootpath, 0, 4);
      QCOMPARE(extractedImgsOut1, 4); // 4 extract alse rewrite
      QCOMPARE(itp3.mRewriteImagesCnt, 4);
      QCOMPARE(itp3.mErrImg.size(), 4); // 4 rewrite msg
    }
  }

  void rename_thubnaim_image_generate_by_potplayer_ok() {
    tDir.ClearAll();

    const QByteArray widthNotAllowedImg = ImageTestPrecoditionTools::GetPNGImage(99, 79, "png");
    const QByteArray widthAllowedImg = ImageTestPrecoditionTools::GetPNGImage(720, 360, "png");

    const QList<FsNodeEntry> nodes{
        {"a need.mp4", false, ""},                                          //
        {"a need.mp4.jpg", false, widthAllowedImg},                         // 1
        {"b no need img.mp4.jpg", false, widthAllowedImg},                  //
        {"c no need vid.mp4", false, ""},                                   //
        {"d no need width invalid vid.mp4", false, ""},                     //
        {"d no need width invalid vid.mp4.png", false, widthNotAllowedImg}, //
        {"folder/need.mp4", false, ""},                                     //
        {"folder/need.mp4.png", false, widthAllowedImg},                    // 1
        {"folder/condition_need_occupied.mp4", false, ""},                  //
        {"folder/condition_need_occupied 11.jpg", false, widthAllowedImg},     // when skip 0, when not skip 1
        {"folder/condition_need_occupied.mp4.jpg", false, widthAllowedImg}, //
        {"folder/no_need_vid.mp4", false, ""},                              //
        {"folder/no_need_img.mp4.jpg", false, widthAllowedImg},             //
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());

    // not skip
    ThumbnailProcesser it{false};
    QVERIFY(it.RenameThumbnailGeneratedByPotPlayer(tDir.path()));

    const QSet<QString> expectSnapShots{
        "a need.mp4",
        "a need 11.jpg",
        "b no need img.mp4.jpg",
        "c no need vid.mp4",
        "d no need width invalid vid.mp4",
        "d no need width invalid vid.mp4.png",
        "folder/need.mp4",
        "folder/need 11.png",
        "folder/condition_need_occupied.mp4",
        "folder/condition_need_occupied 11.jpg",
        "folder/no_need_vid.mp4",
        "folder/no_need_img.mp4.jpg",
    };
    const QSet<QString> actualSnapshot = tDir.Snapshot(QDir::Filter::Files);
    QCOMPARE(actualSnapshot, expectSnapShots);
  }
};

constexpr int ThumbnailProcesserTest::MKV_WIDTH;
constexpr int ThumbnailProcesserTest::MKV_HEIGHT;

#undef FLV_10_VID_BASE_NAME
#undef MP4_33_VID_BASE_NAME

#include "ThumbnailProcesserTest.moc"
REGISTER_TEST(ThumbnailProcesserTest, false)
