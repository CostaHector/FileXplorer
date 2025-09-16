#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>

#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "CastBrowserHelper.h"
#include "EndToExposePrivateMember.h"
#include "SqlRecordTestHelper.h"

class CastBrowserHelperTest : public PlainTestSuite {
  Q_OBJECT
 public:
  CastBrowserHelperTest() : PlainTestSuite{} { LOG_D("CastBrowserHelperTest object created\n"); }

 private slots:
  void test_CastHtmlParts_ok() {
    CastHtmlParts parts;
    parts.body = "TestBodyContent";
    parts.vidPart[0] = "VidHeader:%1";  // %1 将被替换为箭头
    parts.vidPart[1] = "VideoContent";
    parts.imgPart[0] = "ImgHeader:%1";  // %1 将被替换为箭头
    parts.imgPart[1] = "ImageContent";
    QVERIFY(parts.length() > 0);

    // 测试组合 1: 视频和图片均可见
    QString html1 = parts.fullHtml(true, true);
    QVERIFY(html1.contains(parts.body));      // 必须包含 body
    QVERIFY(html1.contains("VidHeader:▼"));   // 检查替换后的 vidPart[0]
    QVERIFY(html1.contains("VideoContent"));  // 检查 vidPart[1]
    QVERIFY(html1.contains("ImgHeader:▼"));   // 检查替换后的 imgPart[0]
    QVERIFY(html1.contains("ImageContent"));  // 检查 imgPart[1]

    // 测试组合 2: 视频可见，图片隐藏
    QString html2 = parts.fullHtml(true, false);
    QVERIFY(html2.contains(parts.body));
    QVERIFY(html2.contains("VidHeader:▼"));
    QVERIFY(html2.contains("VideoContent"));
    QVERIFY(html2.contains("ImgHeader:▶"));    // 图片隐藏时应显示▶
    QVERIFY(!html2.contains("ImageContent"));  // imgPart[1] 不应出现

    // 测试组合 3: 视频隐藏，图片可见
    QString html3 = parts.fullHtml(false, true);
    QVERIFY(html3.contains(parts.body));
    QVERIFY(html3.contains("VidHeader:▶"));    // 视频隐藏时应显示▶
    QVERIFY(!html3.contains("VideoContent"));  // vidPart[1] 不应出现
    QVERIFY(html3.contains("ImgHeader:▼"));
    QVERIFY(html3.contains("ImageContent"));

    // 测试组合 4: 视频和图片均隐藏
    QString html4 = parts.fullHtml(false, false);
    QVERIFY(html4.contains(parts.body));
    QVERIFY(html4.contains("VidHeader:▶"));
    QVERIFY(!html4.contains("VideoContent"));
    QVERIFY(html4.contains("ImgHeader:▶"));
    QVERIFY(!html4.contains("ImageContent"));
  }

  void test_GetCastHtmlParts_ok() {
    const QSqlRecord& henryCavillRecord =
        SqlRecordTestHelper::GetACastRecordLine("Henry Cavill", "MovieStar", "Image1.jpg\nImage2.jpg", "Movie1.mp4\nMovie2.mp4");
    const QString imageHost = "host";
    const QSize ICON_SIZE{400, 300};
    CastHtmlParts parts = CastBrowserHelper::GetCastHtmlParts(henryCavillRecord, imageHost, ICON_SIZE);

    QVERIFY(parts.body.contains("Henry Cavill"));

    QVERIFY(parts.vidPart[0].contains("hideRelatedVideos"));
    QVERIFY(parts.vidPart[1].contains("Movie1.mp4"));
    QVERIFY(parts.vidPart[1].contains("Movie2.mp4"));

    QVERIFY(parts.imgPart[0].contains("hideRelatedImages"));
    QVERIFY(parts.imgPart[1].contains("host/MovieStar/Henry Cavill/Image1.jpg"));
    QVERIFY(parts.imgPart[1].contains("host/MovieStar/Henry Cavill/Image2.jpg"));
  }
};

#include "CastBrowserHelperTest.moc"
REGISTER_TEST(CastBrowserHelperTest, false)
