#include <QCoreApplication>
#include <QtTest>

#include "PublicVariable.h"

class ValueCheckerTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  void fileExtensionBasic() {
    QCOMPARE(ValueChecker::GetFileExtension("AAA.mp4"), ".mp4");
    QCOMPARE(ValueChecker::GetFileExtension("AAA.json"), ".json");
    QCOMPARE(ValueChecker::GetFileExtension("AAA.z01"), ".z01");
  }

  void fileExtension_1Char() {
    QCOMPARE(ValueChecker::GetFileExtension("AAA.h"), ".h");
    QCOMPARE(ValueChecker::GetFileExtension("AAA.m"), ".m");
  }

  void fileExtension_NoExtension() {
    QCOMPARE(ValueChecker::GetFileExtension("AAA.5"), "");
    QCOMPARE(ValueChecker::GetFileExtension("AAA.51"), "");
  }
};

void ValueCheckerTest::initTestCase() {}

void ValueCheckerTest::cleanupTestCase() {}

void ValueCheckerTest::init() {}

void ValueCheckerTest::cleanup() {}

//QTEST_MAIN(ValueCheckerTest)

//#include "ValueCheckerTest.moc"
