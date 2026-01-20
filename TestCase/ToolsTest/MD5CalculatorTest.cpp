#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "MD5Calculator.h"
#include "TDir.h"
using namespace MD5Calculator;

class MD5CalculatorTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir tDir;
private slots:
  void initTestCase() { QVERIFY(tDir.IsValid()); }

  void init() { QVERIFY(tDir.ClearAll()); }

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

    const QString actualHash = GetFileMD5(fileAbsPath, BytesRangeTool::BytesRangeE::ENTIRE_FILE, QCryptographicHash::Algorithm::Md5);
    QCOMPARE(actualHash, expectHash);
  }

  // 测试字节数组MD5计算
  void test_ByteArrayMD5() {
    // 空数组
    QByteArray s1 = MD5Calculator::GetByteArrayMD5(QByteArray{});
    QCOMPARE(s1, "d41d8cd98f00b204e9800998ecf8427e");

    // 短数组
    QByteArray s2 = MD5Calculator::GetByteArrayMD5(QByteArray{"Hello"});
    QCOMPARE(s2, "8b1a9953c4611296a827abf8c47804d7");

    // 相同内容应产生相同MD5
    QByteArray s3 = MD5Calculator::GetByteArrayMD5(QByteArray{"Hello"});
    QCOMPARE(s2, s3);

    // 不同内容应产生不同MD5
    QByteArray s4 = MD5Calculator::GetByteArrayMD5(QByteArray{"World"});
    QVERIFY(s2 != s4);
  }

  // 测试文件MD5计算（全文件）
  void test_FileMD5_Full() {
    // 创建测试文件
    QString filePath = "test.txt";
    QByteArray content = "This is a test file content";
    QVERIFY(tDir.touch(filePath, content));

    // 计算MD5
    QByteArray md5 = MD5Calculator::GetFileMD5(tDir.itemPath(filePath));

    // 验证结果
    QCOMPARE(md5, MD5Calculator::GetByteArrayMD5(content));
  }

  // 测试批量文件MD5计算
  void test_BatchFileMD5() {
    // 创建多个测试文件
    QStringList files = {"file1.txt", "file2.txt", "file3.txt"};
    QByteArray content1 = "Content for file 1";
    QByteArray content2 = "Content for file 2";
    QByteArray content3 = "Content for file 3";

    QVERIFY(tDir.touch(files[0], content1));
    QVERIFY(tDir.touch(files[1], content2));
    QVERIFY(tDir.touch(files[2], content3));

    // 获取文件绝对路径
    QStringList absPaths;
    for (const QString& file : files) {
      absPaths << tDir.itemPath(file);
    }

    // 计算批量MD5
    QList<QByteArray> md5s = MD5Calculator::GetBatchFileMD5(absPaths);

    // 验证结果
    QCOMPARE(md5s.size(), 3);
    QCOMPARE(md5s[0], MD5Calculator::GetByteArrayMD5(content1));
    QCOMPARE(md5s[1], MD5Calculator::GetByteArrayMD5(content2));
    QCOMPARE(md5s[2], MD5Calculator::GetByteArrayMD5(content3));
  }

  // 测试MD5展示功能
  void test_DisplayFilesMD5() {
    // 创建测试文件
    QString filePath = "display_test.txt";
    QByteArray content = "Content for display test";
    QVERIFY(tDir.touch(filePath, content));

    // 获取文件绝对路径
    QString absPath = tDir.itemPath(filePath);

    // 生成展示HTML
    QString html = MD5Calculator::DisplayFilesMD5({absPath});

    // 验证结果
    QByteArray expectedMD5 = MD5Calculator::GetByteArrayMD5(content);
    QVERIFY(html.contains(expectedMD5));
    QVERIFY(html.contains("file(s)"));
    QVERIFY(html.contains("<table>"));
    QVERIFY(html.contains("</table>"));
    QVERIFY(html.contains(filePath));
  }

  // 测试不存在的文件处理
  void test_NonExistingFile() {
    // 不存在的文件路径
    QString nonExistingPath = tDir.itemPath("non_existing.txt");

    // 计算MD5
    QString md5 = MD5Calculator::GetFileMD5(nonExistingPath);

    // 验证结果为空
    QVERIFY(md5.isEmpty());
  }
};

#include "MD5CalculatorTest.moc"
REGISTER_TEST(MD5CalculatorTest, false)
