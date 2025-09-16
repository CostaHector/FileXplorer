#include <QtTest/QtTest>
#include <QTestEventList>

#include "PlainTestSuite.h"
#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "ClickableTextBrowser.h"
#include "EndToExposePrivateMember.h"
#include "StyleSheet.h"
#include "MemoryKey.h"

class ClickableTextBrowserTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_search_a_and_b_ok() { QCOMPARE(ClickableTextBrowser::FormatSearchSentence("A and B"), "%A%B%"); }

  void test_search_a_comma_space_b_ok() { QCOMPARE(ClickableTextBrowser::FormatSearchSentence("A, B"), "%A%B%"); }

  void test_sequence_insensitive() {
    bool needSearchDb{false};
    QStringList noCond{};
    QCOMPARE(ClickableTextBrowser::BuildMultiKeywordLikeCondition(noCond, needSearchDb), "1=1");
    QVERIFY(!needSearchDb);

    needSearchDb = false;
    QStringList oneCond{"A"};
    QCOMPARE(ClickableTextBrowser::BuildMultiKeywordLikeCondition(oneCond, needSearchDb), R"(`Name` LIKE "%A%")");
    QVERIFY(!needSearchDb);

    needSearchDb = false;
    QStringList twoCond{"A", "B"};
    QCOMPARE(ClickableTextBrowser::BuildMultiKeywordLikeCondition(twoCond, needSearchDb), R"(`Name` LIKE "%A%" AND `Name` LIKE "%B%")");
    QVERIFY(!needSearchDb);

    needSearchDb = false;
    QStringList threeCond{"hello", "guys", "thx"};
    QCOMPARE(ClickableTextBrowser::BuildMultiKeywordLikeCondition(threeCond, needSearchDb),
             R"(`Name` LIKE "%hello%" AND `Name` LIKE "%guys%" AND `Name` LIKE "%thx%")");
    QVERIFY(needSearchDb);
  }

  void test_UpdateImagesSizeInHtmlSrc() {
    QString html1 = R"(<div><img src="pic.jpg" width="300" height="200"></div>)";
    QString expected1 = R"(<div><img src="pic.jpg" width="250" height="100"></div>)";  // also height if exist
    ClickableTextBrowser::UpdateImagesSizeInHtmlSrc(html1, QSize{250, 100});
    QCOMPARE(html1, expected1);

    QString html2 = R"(<img src="1.png" width="100"><img src="2.jpg" width="200">)";
    QString expected2 = R"(<img src="1.png" width="250"><img src="2.jpg" width="250">)";
    ClickableTextBrowser::UpdateImagesSizeInHtmlSrc(html2, QSize{250, 100});
    QCOMPARE(html2, expected2);

    QString html3 = R"(<img id="myImg" class="photo" src="image.png" alt="altText" width="400" style="border:1px">)";
    QString expected3 = R"(<img id="myImg" class="photo" src="image.png" alt="altText" width="350" style="border:1px">)";
    ClickableTextBrowser::UpdateImagesSizeInHtmlSrc(html3, QSize{350, 100});
    QCOMPARE(html3, expected3);
  }

  static bool SendWheelEvent(ClickableTextBrowser& browser, const QPoint& angelDelta, Qt::KeyboardModifiers modifiers, bool bNeedAccepted) {
    QPoint widCenterPnt = browser.geometry().center();
    QPoint widCenterPntGlb = browser.mapToGlobal(widCenterPnt);
    QWheelEvent event(widCenterPnt, widCenterPntGlb, angelDelta, angelDelta, Qt::NoButton, modifiers, Qt::NoScrollPhase, false);
    browser.wheelEvent(&event);
    return event.isAccepted() == bNeedAccepted;
  }

  void ctrlshift_WheelEventChangesfontSize_browserDefaultBehavior_ok() {
    ClickableTextBrowser browser;
    // precondition: font size not the largest and not the lowesr
    auto curFont = browser.font();
    curFont.setPointSize(14);
    browser.setFont(curFont);

    const auto pointSize0 = browser.font().pointSize();

    const QPoint downAngelDelta{0, 8 * 15};
    const QPoint upAngelDelta{0, -8 * 15};

    // 1. zoom in font
    SendWheelEvent(browser, downAngelDelta, Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier, true);
    const auto pointSize1 = browser.font().pointSize();
    QVERIFY(pointSize1 > pointSize0);

    // 2. zoom in font again
    SendWheelEvent(browser, downAngelDelta, Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier, true);
    const auto pointSize2 = browser.font().pointSize();
    QVERIFY(pointSize2 > pointSize1);

    // 3. zoom out font
    SendWheelEvent(browser, upAngelDelta, Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier, true);
    const auto pointSize3 = browser.font().pointSize();
    QCOMPARE(pointSize3, pointSize1);

    // 3. zoom out font again
    SendWheelEvent(browser, upAngelDelta, Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier, true);
    const auto pointSize4 = browser.font().pointSize();
    QCOMPARE(pointSize4, pointSize0);
  }

  void ctrl_WheelEventChangesIconSize_ok() {
    ClickableTextBrowser browser;
    // precondition: icon size not the largest and not the lowest
    QVERIFY2(IMAGE_SIZE::ICON_SIZE_CANDIDATES_N >= 2, "At least two elements needed");
    static constexpr int FRONT_ICON_SIZE_INDEX = 0, BACK_ICON_SIZE_INDEX = IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1;

    browser.mCurIconSizeIndex = BACK_ICON_SIZE_INDEX - 1;

    const QPoint downAngelDelta{0, 8 * 15};
    const QPoint upAngelDelta{0, -8 * 15};

    QSignalSpy spy(&browser, &ClickableTextBrowser::iconSizeChanged);
    // 1. zoom in image
    SendWheelEvent(browser, downAngelDelta, Qt::KeyboardModifier::ControlModifier, true);
    QCOMPARE(browser.mCurIconSizeIndex, BACK_ICON_SIZE_INDEX);  // zoom in ok. now the largest icon size
    QCOMPARE(browser.mIconSize, IMAGE_SIZE::ICON_SIZE_CANDIDATES[BACK_ICON_SIZE_INDEX]);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> wheelCtrlDownParams = spy.back();
    QCOMPARE(wheelCtrlDownParams.size(), 1);
    QCOMPARE(wheelCtrlDownParams[0].toSize(), browser.mIconSize);

    // 2. try zoom in image again. but icon size is already the largest, nothing should change, no signal emit
    SendWheelEvent(browser, downAngelDelta, Qt::KeyboardModifier::ControlModifier, false);
    QCOMPARE(browser.mCurIconSizeIndex, BACK_ICON_SIZE_INDEX);
    QCOMPARE(spy.count(), 1);

    browser.mCurIconSizeIndex = FRONT_ICON_SIZE_INDEX + 1;
    // 3. try zoom out image ok
    SendWheelEvent(browser, upAngelDelta, Qt::KeyboardModifier::ControlModifier, true);
    QCOMPARE(browser.mCurIconSizeIndex, FRONT_ICON_SIZE_INDEX);  // now the least icon size
    QCOMPARE(spy.count(), 2);
    QList<QVariant> wheelCtrlUpParams = spy.back();
    QCOMPARE(wheelCtrlUpParams.size(), 1);
    QCOMPARE(wheelCtrlUpParams[0].toSize(), browser.mIconSize);

    // 4. try zoom out again nothing happened, no signal emit
    SendWheelEvent(browser, upAngelDelta, Qt::KeyboardModifier::ControlModifier, false);
    QCOMPARE(browser.mCurIconSizeIndex, FRONT_ICON_SIZE_INDEX);
    QCOMPARE(spy.count(), 2);
  }

  void hideOrShowRelatedVideosImages_ok() {
    Configuration().clear();
    Configuration().setValue(BrowserKey::CAST_PREVIEW_BROWSER_SHOW_RELATED_VIDEOS.name, true);
    Configuration().setValue(BrowserKey::CAST_PREVIEW_BROWSER_SHOW_RELATED_IMAGES.name, true);

    const CastHtmlParts parts{"body", {"vidTitle%1", "vidListStr"}, {"imgTitle%1", "imgListStr"}};

    ClickableTextBrowser browser;
    browser.SetCastHtmlParts(parts);
    QCOMPARE(browser.mCastVideosVisisble, true);
    QCOMPARE(browser.mCastImagesVisisble, true);

    QUrl clickHideShowVidUrl{"hideRelatedVideos"};
    QUrl clickHideShowImgUrl{"hideRelatedImages"};

    QVERIFY(browser.onAnchorClicked(clickHideShowVidUrl));
    QCOMPARE(browser.mCastVideosVisisble, false);
    QCOMPARE(browser.mCastImagesVisisble, true);
    QCOMPARE(browser.toPlainText(), "bodyvidTitle▶imgTitle▼imgListStr");

    QVERIFY(browser.onAnchorClicked(clickHideShowImgUrl));
    QCOMPARE(browser.mCastVideosVisisble, false);
    QCOMPARE(browser.mCastImagesVisisble, false);
    QCOMPARE(browser.toPlainText(), "bodyvidTitle▶imgTitle▶");

    QVERIFY(browser.onAnchorClicked(clickHideShowVidUrl));
    QCOMPARE(browser.mCastVideosVisisble, true);
    QCOMPARE(browser.mCastImagesVisisble, false);
    QCOMPARE(browser.toPlainText(), "bodyvidTitle▼vidListStrimgTitle▶");

    QVERIFY(browser.onAnchorClicked(clickHideShowImgUrl));
    QCOMPARE(browser.mCastVideosVisisble, true);
    QCOMPARE(browser.mCastImagesVisisble, true);
    QCOMPARE(browser.toPlainText(), "bodyvidTitle▼vidListStrimgTitle▼imgListStr");
  }
};

#include "ClickableTextBrowserTest.moc"
REGISTER_TEST(ClickableTextBrowserTest, false)
