#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "MD5Calculator.h"
using namespace MD5Calculator;


class MD5CalculatorTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:

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

  void calculation_correctness() {
    const QString fileAbsPath{__FILE__};
    // precondition:
    const QString expectHash = GetHashPlatformDependent(fileAbsPath);
    QVERIFY(!expectHash.isEmpty());

    const QString actualHash = GetFileMD5(fileAbsPath, -1, QCryptographicHash::Algorithm::Md5);
    QCOMPARE(actualHash, expectHash);
  }
};

#include "MD5CalculatorTest.moc"
REGISTER_TEST(MD5CalculatorTest, false)
