#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "PathTool.h"

class ValueCheckerTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void fileExtensionBasic() {
    QCOMPARE(PathTool::GetFileExtension("AAA.mp4"), ".mp4");
    QCOMPARE(PathTool::GetFileExtension("AAA.json"), ".json");
    QCOMPARE(PathTool::GetFileExtension("AAA.z01"), ".z01");
  }

  void fileExtension_1Char() {
    QCOMPARE(PathTool::GetFileExtension("AAA.h"), ".h");
    QCOMPARE(PathTool::GetFileExtension("AAA.m"), ".m");
  }

  void fileExtension_NoExtension() {
    QCOMPARE(PathTool::GetFileExtension("AAA.5"), ".5");
    QCOMPARE(PathTool::GetFileExtension("AAA.51"), ".51");
  }
};

#include "ValueCheckerTest.moc"
REGISTER_TEST(ValueCheckerTest, false)
