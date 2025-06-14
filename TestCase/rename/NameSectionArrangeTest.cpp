#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pub/MyTestSuite.h"
#include "Tools/NameSectionArrange.h"

class NameSectionArrangeTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_SubscriptsStr2Int() {
    // seperated by comma, space-comma, space
    const QStringList strLst{"5,10,2,3,4", "5 10 2 3 4", "5 10 2 3 4", "5  10 2,3, 4"};
    const QList<int> expectLst{5, 10, 2, 3, 4};
    for (const QString& str : strLst) {
      QList<int> sortedIndLst;
      QVERIFY(SubscriptsStr2Int(str, sortedIndLst));
      QCOMPARE(sortedIndLst, expectLst);
    }
  }

  void test_SubscriptsDigitChar2Int() {
    // seperated by comma, space-comma, space
    const QString digitStrLst{"510234"};
    const QList<int> expectLst{5, 1, 0, 2, 3, 4};
    QList<int> sortedIndLst;
    QVERIFY(SubscriptsDigitChar2Int(digitStrLst, sortedIndLst));
    QCOMPARE(sortedIndLst, expectLst);
  }

  void test_0213_1digit_SortIndex_strict() {
    NameSectionArrange nss({0, 2, 1, 3});
    QCOMPARE(nss("1-2-3-4"), "1 - 3 - 2 - 4");
  }

  void test_4012_1digit_SortIndex_notStrict() {
    NameSectionArrange nss({4, 0, 1, 2}, false);
    QCOMPARE(nss("1-2-3-4"), "1 - 2 - 3");
    // is_strictMode=false, ignore name that contains wasted
    QVERIFY(!nss.HasWasted());
  }

  void test_40213_1digit_SortIndex_notStrict() {
    NameSectionArrange nss({4, 0, 2, 1, 3}, false);
    QCOMPARE(nss("1-2-3-4"), "1 - 3 - 2 - 4");
  }

  void test_4021_1digitSortIndex() {
    NameSectionArrange nss{{4, 0, 2, 1}, true};  // section 3 wasted
    QStringList actual = nss.BatchSwapper({"1-2-3-4", "A-B-C-D", "X - Y - Z"});
    QStringList expect{"1 - 3 - 2", "A - C - B", "X - Z - Y"};
    QCOMPARE(nss.GetWastedNames(), "1-2-3-4\nA-B-C-D");
    QCOMPARE(actual, expect);
  }

  void test_arrangeSwapper12() {
    NameSectionArrange nss(1, 2, false);
    QStringList actual = nss.BatchSwapper({"1-2-3-4", "A-B-C-D", "X - Y - Z"});
    QStringList expect{"1 - 3 - 2 - 4", "A - C - B - D", "X - Z - Y"};
    QVERIFY(!nss.HasWasted());
    QCOMPARE(expect, actual);
  }
};

NameSectionArrangeTest g_NameSectionArrangeTest;
#include "NameSectionArrangeTest.moc"
