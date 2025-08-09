#include <QCoreApplication>
#include <QtTest>
#include "Component/FolderPreview/ClickableTextBrowser.h"
#include "pub/MyTestSuite.h"

class ClickableTextBrowserTest : public MyTestSuite {
  Q_OBJECT
public:
  ClickableTextBrowserTest() : MyTestSuite{true} {}
private slots:
  void test_search_a_and_b_ok() {
    QCOMPARE(ClickableTextBrowser::FormatSearchSentence("A and B"), "%A%B%");
  }

  void test_search_a_comma_space_b_ok() {
    QCOMPARE(ClickableTextBrowser::FormatSearchSentence("A, B"), "%A%B%");
  }

  void test_sequence_insensitive() {
    QStringList noCond{};
    QCOMPARE(ClickableTextBrowser::BuildMultiKeywordLikeCondition(noCond), "1=1");

    QStringList oneCond{"A"};
    QCOMPARE(ClickableTextBrowser::BuildMultiKeywordLikeCondition(oneCond), R"(`Name` LIKE "%A%")");

    QStringList twoCond{"A", "B"};
    QCOMPARE(ClickableTextBrowser::BuildMultiKeywordLikeCondition(twoCond), R"(`Name` LIKE "%A%" AND `Name` LIKE "%B%")");
  }
};

#include "ClickableTextBrowserTest.moc"
ClickableTextBrowserTest g_ClickableTextBrowserTest;
