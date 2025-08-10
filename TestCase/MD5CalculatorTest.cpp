#include <QCoreApplication>
#include <QtTest>
#include "MyTestSuite.h"
#include "MD5Calculator.h"
using namespace MD5Calculator;

class MD5CalculatorTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}
  void init() {}
  void cleanup() {}

  void test_First16ByteHash() {
    const QString& s1 = GetByteArrayMD5(QByteArray{});
    const QString& s2 = GetByteArrayMD5(QByteArray{"AAAAAAAAAAAAAAAA"}.left(5));
    const QString& s3 = GetByteArrayMD5(QByteArray{"AAAAAAAAAAAAAAAA"}.left(5));
    const QString& s4 = GetByteArrayMD5(QByteArray{"AAAAAFGHIJKLMN"}.left(5));
    QVERIFY(s1 != s2);
    QCOMPARE(s2, s3);
    QCOMPARE(s2, s4);
  }

  void test_FullSizeByteHash() {
    const QString& s1 = GetByteArrayMD5(QByteArray{});
    const QString& s2 = GetByteArrayMD5(QByteArray{"AAAAAAAAAAAAAAAA"});
    const QString& s3 = GetByteArrayMD5(QByteArray{"AAAAAAAAAAAAAAAA"});
    const QString& s4 = GetByteArrayMD5(QByteArray{"AAAAAFGHIJKLMN"});
    QCOMPARE(s2, s3);
    QVERIFY(s1 != s4);
    QVERIFY(s2 != s1);
    QVERIFY(s2 != s4);
  }
};

#include "MD5CalculatorTest.moc"
MD5CalculatorTest g_MD5CalculatorTest;
