#include <QCoreApplication>
#include <QtTest>
#include "pub/BeginToExposePrivateMember.h"
#include "Tools/ThumbnailProcesser.h"
#include "pub/EndToExposePrivateMember.h"

const QString rootpath = QFileInfo(__FILE__).absolutePath() + "/test/TestEnv_ThumbnailProcesserTest";
const QDir rootDir(rootpath);

class ThumbnailProcesserTest : public QObject {
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
    QVERIFY(ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 91"));
  }

  void test_IsImageNameLooksLikeThumbnail_no() {
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 11"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 12"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 13"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 14"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 23"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image 24"));
    QVERIFY(!ThumbnailProcesser::IsImageNameLooksLikeThumbnail("image thumbnail"));
  }

  void test_CutThumbnails9ImgsOk() {
    QVERIFY2(rootDir.exists("Images 33.png"), "env must met");

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

// QTEST_MAIN(ThumbnailProcesserTest)
// #include "ThumbnailProcesserTest.moc"
