#include <QCoreApplication>
#include <QtTest>
#include "Component/FolderPreview/ClickableTextBrowser.h"
#include "pub/MyTestSuite.h"

class ClickableTextBrowserTest : public MyTestSuite {
  Q_OBJECT
public:
  ClickableTextBrowserTest() : MyTestSuite{false} {}
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
};

#include "ClickableTextBrowserTest.moc"
ClickableTextBrowserTest g_ClickableTextBrowserTest;
