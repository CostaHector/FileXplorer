#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
#include "Tools/SplitterInsertIndexHelper.h"

class SplitterInsertIndexHelperTest : public MyTestSuite {
  Q_OBJECT
 public:
  SplitterInsertIndexHelperTest() : MyTestSuite{false} {}
 private slots:
  void test_move_front() {
    QVector<int> v1{0, 1, 2};
    MoveElementFrontOf(v1, 0, 0);
    QCOMPARE(v1, (QVector<int>{0, 1, 2}));

    QVector<int> v2{0, 1, 2};
    MoveElementFrontOf(v2, 1, 0);
    QCOMPARE(v2, (QVector<int>{1, 0, 2}));

    QVector<int> v3{0, 1, 2};
    MoveElementFrontOf(v3, 2, 1);
    QCOMPARE(v3, (QVector<int>{0, 2, 1}));
  }

  void test_move_back() {
    QVector<int> v1{0, 1, 2};
    MoveElementFrontOf(v1, 0, 1);
    QCOMPARE(v1, (QVector<int>{0, 1, 2}));

    QVector<int> v2{0, 1, 2};
    MoveElementFrontOf(v2, 0, 2);
    QCOMPARE(v2, (QVector<int>{1, 0, 2}));

    QVector<int> v3{0, 1, 2};
    MoveElementFrontOf(v3, 0, 3);
    QCOMPARE(v3, (QVector<int>{1, 2, 0}));
  }

  void test_0123_ok() {
    QVector<int> v;
    QVERIFY(IsValidMediaTypeSeq("0123", v));
    QCOMPARE(v, (QVector<int>{0, 1, 2, 3}));
  }

  void test_012_ok() {
    QVector<int> v;
    QVERIFY(IsValidMediaTypeSeq("012", v));
    QCOMPARE(v, (QVector<int>{0, 1, 2}));
  }

  void test_10_ok() {
    QVector<int> v;
    QVERIFY(IsValidMediaTypeSeq("10", v));
    QCOMPARE(v, (QVector<int>{1, 0}));
  }

  void test_013_nok() {
    QVector<int> v;
    QVERIFY(!IsValidMediaTypeSeq("013", v));
  }

  void test_vector_int_tostr_ok() { //
    QCOMPARE(MediaTypeSeqStr(QVector<int>{0, 1, 3, 2}), "0132");
    QCOMPARE(MediaTypeSeqStr(QVector<int>{1, 0}), "10");
  }
};

SplitterInsertIndexHelperTest g_SplitterInsertIndexHelperTest;
#include "SplitterInsertIndexHelperTest.moc"
