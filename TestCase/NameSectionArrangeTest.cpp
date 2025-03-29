#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
#include "Tools/NameSectionArrange.h"

class NameSectionArrangeTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}
  void init() {}
  void cleanup() {}

  void test_0213_1digit_SortIndex_strict() {
    NameSectionArrange nss({0, 2, 1, 3});

    QString actual = nss("1-2-3-4");
    QString expect("1 - 3 - 2 - 4");
    QCOMPARE(expect, actual);
  }

  void test_4012_1digit_SortIndex_strict() {
    NameSectionArrange nss({4, 0, 1, 2},  false);
    QString actual = nss("1-2-3-4");
    QString expect("1 - 2 - 3");
    QCOMPARE(expect, actual);
  }

  void test_40213_1digit_SortIndex() {
    NameSectionArrange nss({4, 0, 2, 1, 3}, false);
    QString actual = nss("1-2-3-4");
    QString expect("1 - 3 - 2 - 4");
    QCOMPARE(expect, actual);
  }

  void test_4021_1digitSortIndex() {
    NameSectionArrange nss{{4, 0, 2, 1}, true}; // section 3 wasted
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
