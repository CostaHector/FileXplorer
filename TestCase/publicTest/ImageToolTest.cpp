#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "ImageTool.h"
#include "TDir.h"
#include "FileTool.h"
#include "VideoTestPrecoditionTools.h"
#include "OnScopeExit.h"
#include <QPixmapCache>

class ImageToolTest : public PlainTestSuite {
  Q_OBJECT
public:
private:
  const QString pngPath = TESTCASE_ROOT_PATH "/test/resolution_ratio.png";
  const QSize BEFORE_PNG_SIZE{24, 16};
  const QSize AFTER_PNG_SIZE{ImageTool::EXPECT_THUMBNAIL_SIDE, ImageTool::EXPECT_THUMBNAIL_SIDE};

  const QString gifPath = TESTCASE_ROOT_PATH "/test/sample_Blink_WhiteBlackRed.gif";
  const QSize BEFORE_GIF_SIZE{100, 100};

  QByteArray pngBa;
  QByteArray gifBa;

  const QString mExistVidPath{VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH + QString("/Big Buck Bunny SampleVideo_360x240_1mb 13s.mp4")};
  const int mStartPositionSecond{5};
  const int mIntervalSecond{2};
  const int mFramesCount{2};
  const QString mExistVidPathSceenshotAt5s{VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH + QString("/Big Buck Bunny SampleVideo_360x240_1mb 13s %1.jpg").arg(mStartPositionSecond)};
  const QString mExistVidPathSceenshotAt7s{VideoTestPrecoditionTools::VID_DUR_GETTER_SAMPLE_PATH + QString("/Big Buck Bunny SampleVideo_360x240_1mb 13s %1.jpg").arg(mStartPositionSecond + mIntervalSecond)};

  TDir mTDir;
  QString src{mTDir.itemPath("FolderKaka/FolderKaka.png")};
  QString dst{mTDir.itemPath("FolderKaka/FolderKaka_tn.jpg")};

private slots:
  void initTestCase() {
    QVERIFY(QFile::exists(pngPath));
    QVERIFY(QFile::exists(gifPath));

    QVERIFY(QFile::exists(":/image_test/resolution_ratio"));
    QVERIFY(QFile::exists(":/image_test/sample_Blink_WhiteBlackRed"));

    QVERIFY(mTDir.IsValid());

    pngBa = FileTool::ByteArrayReader(":/image_test/resolution_ratio");
    gifBa = FileTool::ByteArrayReader(":/image_test/sample_Blink_WhiteBlackRed");
    QVERIFY(pngBa.size() > 0);
    QVERIFY(gifBa.size() > 0);

    const QList<FsNodeEntry> nodes{
        {"FolderKaka/FolderKaka.png", false, pngBa},
        {"PretendSystemPath/ShouldNotCreateThumbnail.jpg", false, "contents in jpg"},
    };
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());
    if (QFile::exists(dst)) {
      QVERIFY(QFile::remove(dst));
    }
    if (QFile::exists(mExistVidPathSceenshotAt5s)) {
      QVERIFY(QFile::remove(mExistVidPathSceenshotAt5s));
    }
    if (QFile::exists(mExistVidPathSceenshotAt7s)) {
      QVERIFY(QFile::remove(mExistVidPathSceenshotAt7s));
    }
  }

  void cleanupTestCase() {
    if (QFile::exists(mExistVidPathSceenshotAt5s)) {
      QVERIFY(QFile::remove(mExistVidPathSceenshotAt5s));
    }
    if (QFile::exists(mExistVidPathSceenshotAt7s)) {
      QVERIFY(QFile::remove(mExistVidPathSceenshotAt7s));
    }
  }

  void GetPixmapFromCached_ok() {
    QPixmapCache::clear();
    QVERIFY(!QPixmapCache::find("_64x64_0", nullptr));
    QVERIFY(!QPixmapCache::find(":/image_test/resolution_ratio_64x64_0", nullptr));
    QVERIFY(!QPixmapCache::find(pngPath + "_64x64_0", nullptr));

    QVERIFY(ImageTool::GetPixmapFromCached("", 64, 64, false).isNull()); // empty path
    QVERIFY(!QPixmapCache::find("_64x64_0", nullptr));
    QVERIFY(ImageTool::GetPixmapFromCached("", 64, 64, false).isNull()); // load again

    // local file ok
    QVERIFY(!ImageTool::GetPixmapFromCached(pngPath, 64, 64, false).isNull());
    QVERIFY(QPixmapCache::find(pngPath + "_64x64_0", nullptr));
    QVERIFY(!ImageTool::GetPixmapFromCached(pngPath, 64, 64, false).isNull()); // load again

    // resource file ok
    QVERIFY(!ImageTool::GetPixmapFromCached(":/image_test/resolution_ratio", 64, 64, false).isNull());
    QVERIFY(QPixmapCache::find(":/image_test/resolution_ratio_64x64_0", nullptr));
    QVERIFY(!ImageTool::GetPixmapFromCached(":/image_test/resolution_ratio", 64, 64, false).isNull()); // load again
  }

  void GetIconFromCachedByFullPath_ok() {
    QIcon pngIcon1 = ImageTool::GetIconFromCachedByFullPath(pngPath);
    QVERIFY(!pngIcon1.isNull());

    QIcon pngIcon2 = ImageTool::GetIconFromCachedByFullPath(pngPath);
    QVERIFY(!pngIcon2.isNull());
  }

  void GetImageDimensionPixel_ok() {
    // proto type 1: ok
    QCOMPARE(ImageTool::GetImageDimensionPixel(pngPath), BEFORE_PNG_SIZE);
    QCOMPARE(ImageTool::GetImageDimensionPixel(gifPath), BEFORE_GIF_SIZE);

    // proto type 2: ok
    std::unique_ptr<QBuffer> buffer{new (std::nothrow) QBuffer{&pngBa}};
    QVERIFY(buffer != nullptr);
    QVERIFY(buffer->open(QIODevice::ReadOnly));
    QCOMPARE(ImageTool::GetImageDimensionPixel(buffer.get(), "PNG"), BEFORE_PNG_SIZE);

    buffer.reset(new (std::nothrow) QBuffer{&gifBa});
    QVERIFY(buffer != nullptr);
    QVERIFY(buffer->open(QIODevice::ReadOnly));
    QCOMPARE(ImageTool::GetImageDimensionPixel(buffer.get(), "GIF"), BEFORE_GIF_SIZE);
  }

  void IsFileAbsPathImage_ok() {
    QVERIFY(ImageTool::IsFileAbsPathImage("a.jpg"));
    QVERIFY(ImageTool::IsFileAbsPathImage("b.gif"));
    QVERIFY(ImageTool::IsFileAbsPathImage("c.webp"));
    QVERIFY(ImageTool::IsFileAbsPathImage("d.png"));
    QVERIFY(!ImageTool::IsFileAbsPathImage("e.txt"));

    QVERIFY(ImageTool::IsFileAbsPathImage("a.JPG"));
    QVERIFY(ImageTool::IsFileAbsPathImage("b.GIF"));
    QVERIFY(ImageTool::IsFileAbsPathImage("c.WEBP"));
    QVERIFY(ImageTool::IsFileAbsPathImage("d.PNG"));
    QVERIFY(!ImageTool::IsFileAbsPathImage("e.TXT"));
  }

  void IsGifFile_ok() {
    QVERIFY(!ImageTool::IsGifFile("a.jpg"));
    QVERIFY(ImageTool::IsGifFile("b.gif"));
    QVERIFY(!ImageTool::IsGifFile("c.webp"));
    QVERIFY(!ImageTool::IsGifFile("d.png"));

    QVERIFY(!ImageTool::IsGifFile("a.JPG"));
    QVERIFY(ImageTool::IsGifFile("b.GIF"));
    QVERIFY(!ImageTool::IsGifFile("c.WEBP"));
    QVERIFY(!ImageTool::IsGifFile("d.TXT"));
  }

  void CreateThumbnailCore_ok() {
    // inexist
    QCOMPARE(ImageTool::CreateThumbnailCore("inexist src image file", dst, true), false);
    QVERIFY(!QFile::exists(dst));

    // not image
    QCOMPARE(ImageTool::CreateThumbnailCore(__FILE__, dst, true), false);

    QVERIFY(!QFile::exists(dst));
    QCOMPARE(ImageTool::CreateThumbnailCore(src, dst, true), true); // skip if exist, current:not exist
    QVERIFY(QFile::exists(dst));

    QCOMPARE(ImageTool::CreateThumbnailCore(src, dst, true), true); // skip if exist, current:exist
    QVERIFY(QFile::exists(dst));
    QCOMPARE(ImageTool::CreateThumbnailCore(src, dst, false), true); // override if exist, current:exist
    QVERIFY(QFile::exists(dst));
  }

  void CreateThumbnailForAllDirectFoldersUnder_ok() { //
    QCOMPARE(ImageTool::CreateThumbnailForAllDirectFoldersUnder("inexist path"), 0);

    QCOMPARE(ImageTool::CreateThumbnailForAllDirectFoldersUnder(mTDir.path(), true), 1);
    QCOMPARE(ImageTool::CreateThumbnailForAllDirectFoldersUnder(mTDir.path(), true), 1); // again
  }

  void GrabFramesFromVideos_ok() { //
    QCOMPARE(ImageTool::GrabFramesFromVideos({}, mStartPositionSecond, mIntervalSecond, mFramesCount, true), 0);
    QCOMPARE(ImageTool::GrabFramesFromVideos({"inexist video.mp4"}, mStartPositionSecond, mIntervalSecond, mFramesCount, true), 0);

    QVERIFY(!QFile::exists(mExistVidPathSceenshotAt5s));
    QVERIFY(!QFile::exists(mExistVidPathSceenshotAt7s));
    QCOMPARE(ImageTool::GrabFramesFromVideos({mExistVidPath}, mStartPositionSecond, mIntervalSecond, mFramesCount, true), 1);  // skip if exist
    QVERIFY(QFile::exists(mExistVidPathSceenshotAt5s));
    QVERIFY(QFile::exists(mExistVidPathSceenshotAt7s));
    QCOMPARE(ImageTool::GrabFramesFromVideos({mExistVidPath}, mStartPositionSecond, mIntervalSecond, mFramesCount, true), 0);  // skip if exist
    QCOMPARE(ImageTool::GrabFramesFromVideos({mExistVidPath}, mStartPositionSecond, mIntervalSecond, mFramesCount, false), 1); // override if exist
  }
};

#include "ImageToolTest.moc"
REGISTER_TEST(ImageToolTest, false)
