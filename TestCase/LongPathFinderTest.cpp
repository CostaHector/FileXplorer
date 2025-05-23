#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
#include "pub/BeginToExposePrivateMember.h"
#include "Tools/LongPathFinder.h"
#include "pub/EndToExposePrivateMember.h"

class LongPathFinderTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
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

  void test_CheckTooLongPathCountLengthOk() {
    LongPathFinder lpf;
    lpf.SetDropSectionWhenTooLong(-1);
    QString s49{49, '0'};
    lpf.pres.append(s49);              // 1
    lpf.olds.append(s49 + '-' + s49);  // 2
    lpf.news.append(s49);              // 1
    // old: (1+2+2)*50 = 300
    // new: (1+1+2)*50 = 250 already ok
    QCOMPARE(lpf.CheckTooLongPathCount(), 0);
  }

  void test_CheckTooLongPathCountLengthStillTooOk() {
    LongPathFinder lpf;
    lpf.SetDropSectionWhenTooLong(-1);
    lpf.pres.append("C:/home");
    lpf.olds.append("A - B - C");
    lpf.news.append("A - B");

    QString s49{49, '0'};
    lpf.pres.append(s49);                          // 1
    lpf.olds.append(s49 + '-' + s49 + '-' + s49);  // 3
    lpf.news.append(s49 + '-' + s49);              // 2
    // before: (1+3+3) * 50  = 350
    // after: (1+2+3) * 50 = 300 still too long
    QCOMPARE(lpf.CheckTooLongPathCount(), 1);
  }
};

#include "LongPathFinderTest.moc"
LongPathFinderTest g_LongPathFinderTest;
