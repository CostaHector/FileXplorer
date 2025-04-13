#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
#include "Tools/SplitterInsertIndexHelper.h"

class SplitterInsertIndexHelperTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_insert_in_order_ok() {
    SplitterInsertIndexHelper insertHelper{3};
    QCOMPARE(insertHelper(0), 0);
    QCOMPARE(insertHelper(1), 1);
    QCOMPARE(insertHelper(2), 2);
  }

  void test_insert_in_reverse_order_ok() {
    SplitterInsertIndexHelper insertHelper{3};
    QCOMPARE(insertHelper(2), 0);
    QCOMPARE(insertHelper(1), 0);
    QCOMPARE(insertHelper(0), 0);
  }

  void test_insert_in_random_order_ok() {
    SplitterInsertIndexHelper insertHelper{4};
    QCOMPARE(insertHelper(1), 0);
    QCOMPARE(insertHelper(3), 1);
    QCOMPARE(insertHelper(0), 0);
    QCOMPARE(insertHelper(2), 2);
  }
};

SplitterInsertIndexHelperTest g_SplitterInsertIndexHelperTest;
#include "SplitterInsertIndexHelperTest.moc"
