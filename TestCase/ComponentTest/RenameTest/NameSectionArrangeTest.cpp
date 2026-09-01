#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "NameSectionArrange.h"

class NameSectionArrangeTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void ChopPostfix_ok() {
    QCOMPARE(ChopPostfix("Not exist postfix"), "Not exist postfix");
    QCOMPARE(ChopPostfix("X - Men 2160p"), "X - Men");
    QCOMPARE(ChopPostfix("X - Men 4K"), "X - Men");
    QCOMPARE(ChopPostfix("Captain America 2020 - 04 - 01"), "Captain America");
    QCOMPARE(ChopPostfix("Captain America 20200401"), "Captain America");
    QCOMPARE(ChopPostfix("Captain America 2020"), "Captain America");

    // "Part/Scene/Pt./Sc./Pt/Sc \d{1,2}" no need chop. otherwise need chop
    QCOMPARE(ChopPostfix("Captain America Part 1 2020"), "Captain America Part 1");
    QCOMPARE(ChopPostfix("Captain America Pt. 1 2020"), "Captain America Pt. 1");
    QCOMPARE(ChopPostfix("Captain America Pt 1 2020"), "Captain America Pt 1");
    QCOMPARE(ChopPostfix("Captain America Part1 2020"), "Captain America Part1");
    QCOMPARE(ChopPostfix("Captain America Pt.1 2020"), "Captain America Pt.1");
    QCOMPARE(ChopPostfix("Captain America Pt1 2020"), "Captain America Pt1");

    QCOMPARE(ChopPostfix("Captain America Scene 1 2020"), "Captain America Scene 1");
    QCOMPARE(ChopPostfix("Captain America Sc. 1 2020"), "Captain America Sc. 1");
    QCOMPARE(ChopPostfix("Captain America Sc 1 2020"), "Captain America Sc 1");
    QCOMPARE(ChopPostfix("Captain America Scene1 2020"), "Captain America Scene1");
    QCOMPARE(ChopPostfix("Captain America Sc.1 2020"), "Captain America Sc.1");
    QCOMPARE(ChopPostfix("Captain America Sc1 2020"), "Captain America Sc1");

    QCOMPARE(ChopPostfix("Captain America 1 2020"), "Captain America");
    QCOMPARE(ChopPostfix("Captain America - 1 2020"), "Captain America");
    QCOMPARE(ChopPostfix("Captain America 12 2020"), "Captain America");
    QCOMPARE(ChopPostfix("Captain America - 12 2020"), "Captain America");

    QCOMPARE(ChopPostfix("Captain America 4K 1"), "Captain America");
    QCOMPARE(ChopPostfix("Captain America 1080p 1"), "Captain America");

    // example:
    QCOMPARE(ChopPostfix("Fox - Michael Fassbender, Jane Grey - XMen 2020 4k"), "Fox - Michael Fassbender, Jane Grey - XMen");
    QCOMPARE(ChopPostfix("Fox - Michael Fassbender, Jane Grey - XMen scene 1 2020"), "Fox - Michael Fassbender, Jane Grey - XMen scene 1");
    QCOMPARE(ChopPostfix("Fox - Michael Fassbender, Jane Grey - XMen part 1 4k"), "Fox - Michael Fassbender, Jane Grey - XMen part 1");
    QCOMPARE(ChopPostfix("Fox - Michael Fassbender, Jane Grey - XMen part 1 1"), "Fox - Michael Fassbender, Jane Grey - XMen part 1");
  }

  void SubscriptsStr2Int_ok() {
    // seperated by comma, space-comma, space
    const QStringList strLst{"5,10,2,3,4", "5 10 2 3 4", "5 10 2 3 4", "5  10 2,3, 4"};
    const QList<int> expectLst{5, 10, 2, 3, 4};
    for (const QString& str : strLst) {
      QList<int> sortedIndLst;
      QVERIFY(SubscriptsStr2Int(str, sortedIndLst));
      QCOMPARE(sortedIndLst, expectLst);
    }
  }

  void SubscriptsDigitChar2Int_ok() {
    // seperated by comma, space-comma, space
    const QString digitStrLst{"510234"};
    const QList<int> expectLst{5, 1, 0, 2, 3, 4};
    QList<int> sortedIndLst;
    QVERIFY(SubscriptsDigitChar2Int(digitStrLst, sortedIndLst));
    QCOMPARE(sortedIndLst, expectLst);
  }

  void test_0213_1digit_SortIndex_strict_ok() {
    NameSectionArrange nss(QList<int>{0, 2, 1, 3});
    QCOMPARE(nss("1-2-3-4"), "1 - 3 - 2 - 4");
  }

  void test_4012_1digit_SortIndex_notStrict_ok() {
    NameSectionArrange nss(QList<int>{4, 0, 1, 2}, false);
    QCOMPARE(nss("1-2-3-4"), "1 - 2 - 3");
    // is_strictMode=false, ignore name that contains wasted
    QVERIFY(!nss.HasWasted());
  }

  void test_40213_1digit_SortIndex_notStrict_ok() {
    NameSectionArrange nss(QList<int>{4, 0, 2, 1, 3}, false);
    QCOMPARE(nss("1-2-3-4"), "1 - 3 - 2 - 4");
  }

  void test_4021_1digitSortIndex_ok() {
    NameSectionArrange nss{QList<int>{4, 0, 2, 1}, true};  // section 3 wasted
    QStringList actual = nss.BatchSwapper({"1-2-3-4", "A-B-C-D", "X - Y - Z"});
    QStringList expect{"1 - 3 - 2", "A - C - B", "X - Z - Y"};
    QCOMPARE(nss.GetWastedNames(), "1-2-3-4\nA-B-C-D");
    QCOMPARE(actual, expect);
  }

  void test_arrangeSwapper12_noChopPostfix_ok() {
    NameSectionArrange nss(1, 2, false);
    QStringList actual = nss.BatchSwapper({"1-2-3-4", "A-B-C-D", "X - Y - Z"});
    QStringList expect{"1 - 3 - 2 - 4", "A - C - B - D", "X - Z - Y"};
    QVERIFY(!nss.HasWasted());
    QCOMPARE(expect, actual);
  }

  void arrangeSwapper12_chopPostfix_ok() {
    NameSectionArrange nss(1, 2, false, true);
    QStringList actual = nss.BatchSwapper({
        "Fox - Michael Fassbender, Jane Grey - XMen 2020 4k",
        "Fox - Michael Fassbender, Jane Grey - XMen scene 1 2020",
        "Fox - Michael Fassbender, Jane Grey - XMen part 1 4k",
        "Fox - Michael Fassbender, Jane Grey - XMen part 1 1", // number need chop
        "Fox - Michael Fassbender, Jane Grey - XMen part 1 1080p 1", //
    });
    const QStringList expect{"Fox - XMen - Michael Fassbender, Jane Grey 2020 4k",
                             "Fox - XMen scene 1 - Michael Fassbender, Jane Grey 2020",
                             "Fox - XMen part 1 - Michael Fassbender, Jane Grey 4k",
                             "Fox - XMen part 1 - Michael Fassbender, Jane Grey 1",
                             "Fox - XMen part 1 - Michael Fassbender, Jane Grey 1080p 1",
                             };
    QVERIFY(!nss.HasWasted());
    QCOMPARE(expect, actual);
  }
};

#include "NameSectionArrangeTest.moc"
REGISTER_TEST(NameSectionArrangeTest, false)
