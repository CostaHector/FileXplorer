#include <QCoreApplication>
#include <QtTest>

#include "TestCase/pub/BeginToExposePrivateMember.h"
#include "Tools/MD5Calculator.h"
#include "TestCase/pub/EndToExposePrivateMember.h"
#include "pub/FileSystemRelatedTest.h"
using namespace MD5Calculator;

class MD5CalculatorTest : public FileSystemRelatedTest {
  Q_OBJECT
 public:
  MD5CalculatorTest():FileSystemRelatedTest{"TestEnv_MD5Calculator", false}{}
 private slots:
  void initTestCase() {
    m_rootHelper<<FileSystemNode{"folder"}
                <<FileSystemNode{"file0 empty.txt", false, ""}
                <<FileSystemNode{"file1 same.txt", false, "AAAAAAAAAAAAAAAA"}
                <<FileSystemNode{"file2 same.txt", false, "AAAAAAAAAAAAAAAA"}
                <<FileSystemNode{"file3 unique.txt", false, "AAAAAFGHIJKLMN"};
  }
  void cleanupTestCase() {
    QVERIFY(FileSystemHelper(ROOT_DIR).EraseFileSystemTree(true));
  }

  void init() {}
  void cleanup() {}

  void test_First16ByteHash() {
    const QString& s1 = GetMD5(ROOT_DIR + "/file0 empty.txt", 16);
    const QString& s2 = GetMD5(ROOT_DIR + "/file1 same.txt", 5);
    const QString& s3 = GetMD5(ROOT_DIR + "/file2 same.txt", 5);
    const QString& s4 = GetMD5(ROOT_DIR + "/file3 unique.txt", 5);
    QVERIFY(s1 != s2);
    QCOMPARE(s2, s3);
    QCOMPARE(s2, s4);
  }

  void test_FullSizeByteHash() {
    const QString& s1 = GetMD5(ROOT_DIR + "/file0 empty.txt", -1);
    const QString& s2 = GetMD5(ROOT_DIR + "/file1 same.txt", -1);
    const QString& s3 = GetMD5(ROOT_DIR + "/file2 same.txt", -1);
    const QString& s4 = GetMD5(ROOT_DIR + "/file3 unique.txt", -1);
    QCOMPARE(s2, s3);
    QVERIFY(s1 != s4);
    QVERIFY(s2 != s1);
    QVERIFY(s2 != s4);
  }
};

//QTEST_MAIN(MD5CalculatorTest)
#include "MD5CalculatorTest.moc"
