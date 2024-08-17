#include <QCoreApplication>
#include <QtTest>

#include "TestCase/pub/BeginToExposePrivateMember.h"
#include "Tools/MD5Calculator.h"
#include "TestCase/pub/EndToExposePrivateMember.h"

const QString MD5_CALCULATOR_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_MD5Calculator");

using namespace MD5Calculator;

class MD5CalculatorTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}

  void init() {}
  void cleanup() {}

  void test_env_ok() {
    QDir dir(MD5_CALCULATOR_DIR, "", QDir::SortFlag::DirsFirst | QDir::SortFlag::Name, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
    const QStringList& files = dir.entryList();
    const QStringList& expect{"folder", "file0 empty.txt", "file1 same.txt", "file2 same.txt", "file3 unique.txt"};
    QCOMPARE(files, expect);
  }

  void test_First16ByteHash() {
    QDir dir{MD5_CALCULATOR_DIR};
    const QString& s1 = GetMD5(dir.absoluteFilePath("file0 empty.txt"), 16);
    const QString& s2 = GetMD5(dir.absoluteFilePath("file1 same.txt"), 16);
    const QString& s3 = GetMD5(dir.absoluteFilePath("file2 same.txt"), 16);
    const QString& s4 = GetMD5(dir.absoluteFilePath("file3 unique.txt"), 16);
    QCOMPARE(s2, s3);
    QCOMPARE(s2, s4);
    QVERIFY(s1 != s2);
  }

  void test_FullSizeByteHash() {
    QDir dir{MD5_CALCULATOR_DIR};
    const QString& s1 = GetMD5(dir.absoluteFilePath("file0 empty.txt"), -1);
    const QString& s2 = GetMD5(dir.absoluteFilePath("file1 same.txt"), -1);
    const QString& s3 = GetMD5(dir.absoluteFilePath("file2 same.txt"), -1);
    const QString& s4 = GetMD5(dir.absoluteFilePath("file3 unique.txt"), -1);
    QVERIFY(s1 != s4);
    QCOMPARE(s2, s3);
    QVERIFY(s2 != s1);
    QVERIFY(s2 != s4);
  }
};

//QTEST_MAIN(MD5CalculatorTest)
#include "MD5CalculatorTest.moc"
