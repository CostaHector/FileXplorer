#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "TDir.h"

class ArchiveFilesTest : public PlainTestSuite {
  Q_OBJECT
 public:
  // TDir tDir;
 private slots:
  void initTestCase() {
    // QVERIFY(tDir.IsValid());
  }

 private:
};

#include "ArchiveFilesTest.moc"
REGISTER_TEST(ArchiveFilesTest, false)
