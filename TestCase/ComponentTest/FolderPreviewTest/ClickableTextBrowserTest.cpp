#include <QtTest/QtTest>
#include <QTestEventList>

#include "PlainTestSuite.h"
#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "ClickableTextBrowser.h"
#include "EndToExposePrivateMember.h"
#include "StyleSheet.h"
#include "MemoryKey.h"
#include "ClipboardGuard.h"
#include "SqlRecordTestHelper.h"
#include "MouseKeyboardEventHelper.h"
using namespace MouseKeyboardEventHelper;


QString setCurSelection(ClickableTextBrowser& browser, const int selectBegIndex, const int selectEndIndex) {
  QTextCursor cursor = browser.textCursor();
  cursor.setPosition(selectBegIndex);
  cursor.setPosition(selectEndIndex, QTextCursor::KeepAnchor);
  browser.setTextCursor(cursor);
  return cursor.selectedText();
}

QStringList setMultiCurSelection(ClickableTextBrowser& browser, const QList<std::pair<int, int>>& beLst) {
  browser.ClearAllSelections();
  QStringList textsLst;
  textsLst.reserve(beLst.size());
  for (const std::pair<int, int>& pr : beLst) {
    QTextCursor cursor = browser.textCursor();
    cursor.setPosition(pr.first);
    cursor.setPosition(pr.second, QTextCursor::KeepAnchor);
    browser.AppendASelection(cursor);
    textsLst.push_back(cursor.selectedText());
  }
  return textsLst;
}

class ClickableTextBrowserTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_search_ok() {
    QCOMPARE(ClickableTextBrowser::FormatSearchSentence("A and B"), "%A%B%");
    QCOMPARE(ClickableTextBrowser::FormatSearchSentence("A, B"), "%A%B%");
  }

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

  void CopySelectedTextToClipboard_ok() {
    ClipboardGuard clipboardGuard;
    QVERIFY(clipboardGuard);
    auto* pClip = clipboardGuard.clipBoard();
    QVERIFY(pClip != nullptr);
    QVERIFY(clipboardGuard.getCurText() != "Hello world");

    pClip->clear();

    ClickableTextBrowser ctb;
    ctb.setPlainText("Hello world");
    ctb.selectAll();
    QCOMPARE(ctb.GetCurrentSelectedText(), "Hello world");
    ctb.CopySelectedTextToClipboard();
    // QCOMPARE(clipboardGuard.getCurText(), "Hello world"); // clipboard always not stable
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

  void searchSelectionReq_ok() {
    const QString initalPlainTextContent = "Hello world";
    // simple search
    ClickableTextBrowser browser;
    browser.setPlainText(initalPlainTextContent);

    {  // zero char selected. too short, skip, nothing changed
      QCOMPARE(browser.GetCurrentSelectedText(), "");
      browser.onSearchSelectionReq();
      QCOMPARE(browser.toPlainText(), initalPlainTextContent);
    }

    {  // 2 char selected. too short, skip, nothing changed
      QCOMPARE(setCurSelection(browser, 0, 2), "He");
      QCOMPARE(browser.GetCurrentSelectedText(), "He");
      browser.onSearchSelectionReq();
      QCOMPARE(browser.toPlainText(), initalPlainTextContent);
    }

    {
      QCOMPARE(setCurSelection(browser, 0, 11), "Hello world");
      const QString whereClause = "%Hello world%";  // %all text selected%
      const QString textWhenNothing = ClickableTextBrowser::WHEN_SEARCH_RETURN_EMPTY_LIST_HINT_TEXT.arg(whereClause);

      // let search reture empty list, search result will append simply
      UserSpecifiedBrowerInteractMock::mockSqlRecordList().clear();
      browser.onSearchSelectionReq();
      QCOMPARE(browser.toPlainText().count(textWhenNothing), 1);

      QCOMPARE(setCurSelection(browser, 0, 11), "Hello world");
      browser.onSearchSelectionReq();
      QCOMPARE(browser.toPlainText().count(textWhenNothing), 2);

      QList<QSqlRecord> mockReturnList{
          SqlRecordTestHelper::GetAMovieRecordUsedInBrowser("", "/home/to", "A long description Hello world.mp4", 999),
          SqlRecordTestHelper::GetAMovieRecordUsedInBrowser("", "/home/to/reference", "A reference to Hello world.mp4", 1),
      };
      UserSpecifiedBrowerInteractMock::mockSqlRecordList() = mockReturnList;
      QCOMPARE(setCurSelection(browser, 0, 11), "Hello world");
      browser.onSearchSelectionReq();

      const QString htmlContentWhenSearchReturn2Lines = browser.toPlainText();
      QVERIFY(htmlContentWhenSearchReturn2Lines != initalPlainTextContent);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.count(textWhenNothing), 2);  // result is append to current contens

      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("A long description Hello world.mp4"), true);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("0'0'0'999"), true);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("/home/to"), true);

      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("A reference to Hello world.mp4"), true);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("0'0'0'1"), true);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("/home/to/reference"), true);
    }
  }
  void advance_chooseable_search_ok() {
    UserSpecifiedBrowerInteractMock::mockSqlRecordList().clear();
    const QString initalPlainTextContent = "Hello world";
    // simple search
    ClickableTextBrowser browser;
    browser.setPlainText(initalPlainTextContent);

    {  // 1. zero char selected. too short, skip, nothing changed
      std::pair<bool, QString> emptyKeyWords{true, ""};
      UserSpecifiedBrowerInteractMock::mockSearchKeyString() = emptyKeyWords;

      browser.onSearchSelectionAdvanceReq();  // skipped
      QCOMPARE(browser.toPlainText(), initalPlainTextContent);
    }

    {  // 2. User not accept candidates, skip, nothing changed
      browser.ClearAllSelections();
      std::pair<bool, QString> chars10KeyWords{false, "Hello World"};  // user not accept
      UserSpecifiedBrowerInteractMock::mockSearchKeyString() = chars10KeyWords;
      browser.onSearchSelectionAdvanceReq();  // skipped
      QCOMPARE(browser.toPlainText(), initalPlainTextContent);
    }

    {                                                                 // 3. all selected, user accept
      std::pair<bool, QString> chars10KeyWords{true, "Hello World"};  // user accept
      UserSpecifiedBrowerInteractMock::mockSearchKeyString() = chars10KeyWords;

      QCOMPARE(setCurSelection(browser, 0, 11), "Hello world");
      const QString whereClause = "%Hello world%";  // %all text selected%
      const QString textWhenNothing = ClickableTextBrowser::WHEN_SEARCH_RETURN_EMPTY_LIST_HINT_TEXT.arg(whereClause);

      QCOMPARE(browser.toPlainText().count(textWhenNothing), 0);  // let search reture empty list, search result will append simply
      UserSpecifiedBrowerInteractMock::mockSqlRecordList().clear();
      browser.onSearchSelectionAdvanceReq();
      QCOMPARE(browser.toPlainText().count(textWhenNothing), 1);

      QCOMPARE(setCurSelection(browser, 0, 11), "Hello world");
      browser.onSearchSelectionAdvanceReq();
      QCOMPARE(browser.toPlainText().count(textWhenNothing), 2);

      QList<QSqlRecord> mockReturnList{
          SqlRecordTestHelper::GetAMovieRecordUsedInBrowser("", "/home/to", "A long description Hello world.mp4", 999),
          SqlRecordTestHelper::GetAMovieRecordUsedInBrowser("", "/home/to/reference", "A reference to Hello world.mp4", 1),
      };
      UserSpecifiedBrowerInteractMock::mockSqlRecordList() = mockReturnList;
      QCOMPARE(setCurSelection(browser, 0, 11), "Hello world");
      browser.onSearchSelectionAdvanceReq();

      const QString htmlContentWhenSearchReturn2Lines = browser.toPlainText();
      QVERIFY(htmlContentWhenSearchReturn2Lines != initalPlainTextContent);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.count(textWhenNothing), 2);  // result is append to current contens

      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("A long description Hello world.mp4"), true);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("0'0'0'999"), true);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("/home/to"), true);

      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("A reference to Hello world.mp4"), true);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("0'0'0'1"), true);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("/home/to/reference"), true);
    }
  }

  void search_multiselection_search_ok() {
    UserSpecifiedBrowerInteractMock::mockSqlRecordList().clear();
    const QString initalPlainTextContent = "Hello world";
    // simple search
    ClickableTextBrowser browser;
    browser.setPlainText(initalPlainTextContent);

    {  // 1. zero words selected. too short, skip, nothing changed
      browser.ClearAllSelections();
      browser.onSearchMultiSelectionReq();
      QCOMPARE(browser.toPlainText(), initalPlainTextContent);
    }

    {  // 2. only 2 char selected. too short, skip, nothing changed
      QCOMPARE(setMultiCurSelection(browser, {{0, 2}}), (QStringList{"He"}));
      browser.onSearchMultiSelectionReq();
      QCOMPARE(browser.toPlainText(), initalPlainTextContent);
    }

    {  // 3. User not accept candidates, skip, nothing changed
      UserSpecifiedBrowerInteractMock::mockSqlRecordList().clear();

      QCOMPARE(setMultiCurSelection(browser, {{0, 5}, {6, 11}}), (QStringList{"Hello", "world"}));
      browser.onSearchMultiSelectionReq();
      QCOMPARE(setMultiCurSelection(browser, {{0, 5}, {6, 11}}), (QStringList{"Hello", "world"}));
      browser.onSearchMultiSelectionReq();
      QCOMPARE(browser.toPlainText().count("Not in database"), 2);

      QList<QSqlRecord> mockReturnList{
          SqlRecordTestHelper::GetAMovieRecordUsedInBrowser("", "/home/to", "A long description Hello world.mp4", 999),
          SqlRecordTestHelper::GetAMovieRecordUsedInBrowser("", "/home/to/reference", "A reference to Hello world.mp4", 1),
      };
      UserSpecifiedBrowerInteractMock::mockSqlRecordList() = mockReturnList;
      QCOMPARE(setMultiCurSelection(browser, {{0, 5}, {6, 11}}), (QStringList{"Hello", "world"}));
      QCOMPARE(setCurSelection(browser, 0, 11), "Hello world");
      browser.onSearchMultiSelectionReq();

      const QString htmlContentWhenSearchReturn2Lines = browser.toPlainText();
      QVERIFY(htmlContentWhenSearchReturn2Lines != initalPlainTextContent);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.count("Not in database"), 2);  // result is append to current contens

      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("A long description Hello world.mp4"), true);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("0'0'0'999"), true);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("/home/to"), true);

      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("A reference to Hello world.mp4"), true);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("0'0'0'1"), true);
      QCOMPARE(htmlContentWhenSearchReturn2Lines.contains("/home/to/reference"), true);
    }
  }

  void appendMultiSelectionToCastDbReq_ok() {
    UserSpecifiedBrowerInteractMock::mockSqlRecordList().clear();
    const QString initalPlainTextContent = "Hello world";
    // simple search
    ClickableTextBrowser browser;
    browser.setPlainText(initalPlainTextContent);

    browser.ClearAllSelections();
    std::pair<bool, QString> castZeroNotAccept{false, ""};
    UserSpecifiedBrowerInteractMock::mockMultiLinePerfsText() = castZeroNotAccept;
    QCOMPARE(browser.onAppendMultiSelectionToCastDbReq(), 0);

    QCOMPARE(setMultiCurSelection(browser, {{0, 5}, {6, 11}}), (QStringList{"Hello", "world"}));
    QCOMPARE(browser.GetMultiSelectedTexts(), (QStringList{"Hello", "world"}));

    std::pair<bool, QString> cast2Accept{true, "Hello\nworld"};
    UserSpecifiedBrowerInteractMock::mockMultiLinePerfsText() = cast2Accept;
    QCOMPARE(browser.onAppendMultiSelectionToCastDbReq(), 2);
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

  void mouser_event_ok() {
    const QString initalPlainTextContent = "This is a test sentence, cast a, cast b";
    ClickableTextBrowser browser;
    browser.setPlainText(initalPlainTextContent);

    // double click append selections to mMultiSelections
    {
      QCOMPARE(browser.mMultiSelections.size(), 0);

      QMouseEvent mouseDoubleClickEvent{QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
      QCOMPARE(setCurSelection(browser, 0, 4), "This");
      browser.mouseDoubleClickEvent(&mouseDoubleClickEvent);
      QCOMPARE(browser.mMultiSelections.size(), 1);

      QCOMPARE(setCurSelection(browser, 5, 7), "is");
      browser.mouseDoubleClickEvent(&mouseDoubleClickEvent);
      QCOMPARE(browser.mMultiSelections.size(), 2);
      QCOMPARE(browser.GetMultiSelectedTexts(), (QStringList{"This", "is"}));
    }
    browser.ClearAllSelections();

    {  // mousePressEvent ok
      QCOMPARE(browser.mbDragging, false);
      QMouseEvent pressEvent{QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
      browser.mousePressEvent(&pressEvent);
      QCOMPARE(browser.mbDragging, true);
      QCOMPARE(browser.mDraggingStartPos, pressEvent.pos());
    }

    {
      QCOMPARE(browser.mbDragging, true);

      QCOMPARE(setCurSelection(browser, 0, 4), "This");
      QMouseEvent releaseEvent{QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
      browser.mouseReleaseEvent(&releaseEvent);
      QCOMPARE(browser.mbDragging, false);
      QCOMPARE(browser.mMultiSelections.size(), 1);
      QCOMPARE(browser.GetMultiSelectedTexts(), (QStringList{"This"}));
    }

    {  // mouseMoveEvent ok
      QCOMPARE(browser.mbDragging, false);
      QMouseEvent dragEvent{QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
      browser.mouseMoveEvent(&dragEvent);
      QCOMPARE(dragEvent.isAccepted(), true);
    }

    const QPoint tbCenterPnt = browser.geometry().center();
    QContextMenuEvent contextMenuEvent(QContextMenuEvent::Mouse, tbCenterPnt, browser.mapToGlobal(tbCenterPnt));
    browser.contextMenuEvent(&contextMenuEvent);
    QVERIFY(contextMenuEvent.isAccepted());

    { // bounder test
      QVERIFY(browser.mBrowserMenu != nullptr);
      QVERIFY(browser.iconSize().isValid());
      browser.resizeEvent(nullptr); // should not crashdown
      QResizeEvent resizeEvent(QSize(300, 200), browser.size());
      browser.resizeEvent(&resizeEvent);
    }
  }
};

#include "ClickableTextBrowserTest.moc"
REGISTER_TEST(ClickableTextBrowserTest, false)
