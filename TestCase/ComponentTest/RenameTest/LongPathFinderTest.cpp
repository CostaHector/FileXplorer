#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "LongPathFinder.h"
#include "EndToExposePrivateMember.h"

class LongPathFinderTest : public PlainTestSuite {
  Q_OBJECT
 public:
  LongPathFinderTest() : PlainTestSuite{} {}
 private slots:
  void test_NoChop() {
    LongPathFinder lpf;
    lpf.SetDropSectionWhenTooLong(0);
    QCOMPARE(lpf.GetNewFolderName("A"), "A");
    QCOMPARE(lpf.GetNewFolderName("A - B"), "A - B");
    QCOMPARE(lpf.GetNewFolderName("A - B - C"), "A - B - C");
    QCOMPARE(lpf.GetNewFolderName("A - B - C - D"), "A - B - C - D");
  }

  void test_ChopFirstSection() {
    LongPathFinder lpf;
    lpf.SetDropSectionWhenTooLong(1);
    QCOMPARE(lpf.GetNewFolderName("A"), "A");
    QCOMPARE(lpf.GetNewFolderName("A - B"), "B");
    QCOMPARE(lpf.GetNewFolderName("A - B - C"), "B - C");
    QCOMPARE(lpf.GetNewFolderName("A - B - C - D"), "B - C - D");
  }

  void test_ChopLastSection() {
    LongPathFinder lpf;
    lpf.SetDropSectionWhenTooLong(-1);
    QCOMPARE(lpf.GetNewFolderName("A"), "A");
    QCOMPARE(lpf.GetNewFolderName("A - B"), "A");
    QCOMPARE(lpf.GetNewFolderName("A - B - C"), "A - B");
    QCOMPARE(lpf.GetNewFolderName("A - B - C - D"), "A - B - C");
  }

  void test_ChopSecondToLastSection() {
    LongPathFinder lpf;
    lpf.SetDropSectionWhenTooLong(-2);
    QCOMPARE(lpf.GetNewFolderName("A"), "A");
    QCOMPARE(lpf.GetNewFolderName("A - B"), "B");
    QCOMPARE(lpf.GetNewFolderName("A - B - C"), "A - C");
    QCOMPARE(lpf.GetNewFolderName("A - B - C - D"), "A - B - D");
  }
};

#include "LongPathFinderTest.moc"
REGISTER_TEST(LongPathFinderTest, false)
