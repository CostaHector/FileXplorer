#include <QCoreApplication>
#include <QtTest>
#include "ClickableTextBrowser.h"
#include "PlainTestSuite.h"

class ClickableTextBrowserTest : public PlainTestSuite {
  Q_OBJECT
public:
  ClickableTextBrowserTest() : PlainTestSuite{} {}
private slots:
  void test_search_a_and_b_ok() {
    QCOMPARE(ClickableTextBrowser::FormatSearchSentence("A and B"), "%A%B%");
  }

  void test_search_a_comma_space_b_ok() {
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

  void test_UpdateImagesSizeInHtmlSrc() {
    QString html1 = R"(<div><img src="pic.jpg" width="300" height="200"></div>)";
    QString expected1 = R"(<div><img src="pic.jpg" width="250" height="100"></div>)"; // also height if exist
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
};

#include "ClickableTextBrowserTest.moc"
REGISTER_TEST(ClickableTextBrowserTest, false)
