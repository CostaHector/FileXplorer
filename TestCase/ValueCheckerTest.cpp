#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
#include "public/PathTool.h"

class ValueCheckerTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}
  void init() {}
  void cleanup() {}

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
    QCOMPARE(PATHTOOL::GetFileExtension("AAA.5"), ".5");
    QCOMPARE(PATHTOOL::GetFileExtension("AAA.51"), ".51");
  }
};

ValueCheckerTest g_ValueCheckerTest;

#include "ValueCheckerTest.moc"
