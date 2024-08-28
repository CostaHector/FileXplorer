#include <QCoreApplication>
#include <QtTest>

#include "pub/BeginToExposePrivateMember.h"
#include "Tools/LongPathFinder.h"
#include "pub/EndToExposePrivateMember.h"

const QString LONG_PATH_TEST = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_VideosDurationGetter/TYPES");

class LongPathFinderTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}

  void init() {}
  void cleanup() {}

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

  void test_CheckLengthOk() {
    LongPathFinder lpf;
    lpf.SetDropSectionWhenTooLong(-1);
    lpf.pres.append("C:/home to path");
    lpf.olds.append("ABCDE FGHIJ ABCDE FGHIJ - 0123456789 0123456789 0123456789 0123456789 - 0123456789 0123456789 0123456789 0123456789");
    lpf.news.append("ABCDE FGHIJ ABCDE FGHIJ - 0123456789 0123456789 0123456789 0123456789");
    QCOMPARE(lpf.Check(), 0);
  }

  void test_CheckLengthStillTooOk() {
    LongPathFinder lpf;
    lpf.SetDropSectionWhenTooLong(-1);
    lpf.pres.append("C:/home");
    lpf.olds.append("A - B - C");
    lpf.news.append("A - B");

    lpf.pres.append("C:/home to path");
    lpf.olds.append("ABCDE FGHIJ ABCDE FGHIJ - 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 - 0123456789");
    lpf.news.append("ABCDE FGHIJ ABCDE FGHIJ - 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789");
    QCOMPARE(lpf.Check(), 1);
  }

};

QTEST_MAIN(LongPathFinderTest)
#include "LongPathFinderTest.moc"
