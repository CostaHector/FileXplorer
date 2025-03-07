#include <QCoreApplication>
#include <QtTest>

#include "Tools/PathTool.h"

class ValueCheckerTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  void fileExtensionBasic() {
    QCOMPARE(PATHTOOL::GetFileExtension("AAA.mp4"), ".mp4");
    QCOMPARE(PATHTOOL::GetFileExtension("AAA.json"), ".json");
    QCOMPARE(PATHTOOL::GetFileExtension("AAA.z01"), ".z01");
  }

  void fileExtension_1Char() {
    QCOMPARE(PATHTOOL::GetFileExtension("AAA.h"), ".h");
    QCOMPARE(PATHTOOL::GetFileExtension("AAA.m"), ".m");
  }

  void fileExtension_NoExtension() {
    QCOMPARE(PATHTOOL::GetFileExtension("AAA.5"), "");
    QCOMPARE(PATHTOOL::GetFileExtension("AAA.51"), "");
  }
};

void ValueCheckerTest::initTestCase() {}

void ValueCheckerTest::cleanupTestCase() {}

void ValueCheckerTest::init() {}

void ValueCheckerTest::cleanup() {}

//QTEST_MAIN(ValueCheckerTest)

//#include "ValueCheckerTest.moc"
