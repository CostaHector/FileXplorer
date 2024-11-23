#include <QCoreApplication>
#include <QtTest>
#include "pub/BeginToExposePrivateMember.h"
#include "Tools/ImgThumbnailProcesser.h"
#include "pub/EndToExposePrivateMember.h"

QString rootpath = QFileInfo(__FILE__).absolutePath() + "/test/TestEnv_ImgThumbnailProcesserTest";
const QDir rootDir(rootpath);

class ImgThumbnailProcesserTest : public QObject {
  Q_OBJECT
 private slots:

  void cleanupTestCase() {
    for (auto& imgName : rootDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name)) {
      if (imgName == "Images 33.png") {
        continue;
      }
      QVERIFY(QFile::remove(rootDir.absoluteFilePath(imgName)));
    }
  }

  void test_IsImageNameLooksLikeThumbnail_ok() {
    QVERIFY(ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 22"));
    QVERIFY(ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 33"));
    QVERIFY(ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 44"));

    QVERIFY(ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 21"));
    QVERIFY(ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 31"));
    QVERIFY(ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 41"));
    QVERIFY(ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 51"));
    QVERIFY(ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 61"));
    QVERIFY(ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 71"));
    QVERIFY(ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 81"));
    QVERIFY(ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 91"));
  }

  void test_IsImageNameLooksLikeThumbnail_no() {
    QVERIFY(!ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 11"));
    QVERIFY(!ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 12"));
    QVERIFY(!ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 13"));
    QVERIFY(!ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 14"));
    QVERIFY(!ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 23"));
    QVERIFY(!ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 24"));
    QVERIFY(!ImgThumbnailProcesser::IsImageNameLooksLikeThumbnail("image thumbnail"));
  }

  void test_CutThumbnails9ImgsOk() {
    QVERIFY2(rootDir.exists("Images 33.png"), "env must met");

    ImgThumbnailProcesser itp{true};
    int extractedImgsOut9 = itp(rootpath, 0, 9);

    QCOMPARE(extractedImgsOut9, 9);
    QVERIFY(itp.mErrImg.isEmpty());

    int extractedImgsOut0 = itp(rootpath, 0, 1);
    QCOMPARE(extractedImgsOut0, 0);
    QCOMPARE(itp.mRewriteImagesCnt, 0);
    QVERIFY(itp.mErrImg.isEmpty());  // 1 skip

    ImgThumbnailProcesser itp3{false};
    int extractedImgsOut1 = itp3(rootpath, 0, 4);
    QCOMPARE(extractedImgsOut1, 4);  // 4 extract alse rewrite
    QCOMPARE(itp3.mRewriteImagesCnt, 4);
    QCOMPARE(itp3.mErrImg.size(), 4);  // 4 rewrite msg
  }
};

QTEST_MAIN(ImgThumbnailProcesserTest)
#include "ImgThumbnailProcesserTest.moc"
