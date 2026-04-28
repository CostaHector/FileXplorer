#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "AccountInfo.h"
#include "EndToExposePrivateMember.h"

class AccountInfoTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_constructor_and_properties() {
    // 测试构造函数
    AccountInfo acc("Email", "Personal", "user@example.com", "password123", "Additional info");

    QCOMPARE(acc.typeStr, "Email");
    QCOMPARE(acc.nameStr, "Personal");
    QCOMPARE(acc.accountStr, "user@example.com");
    QCOMPARE(acc.pwdStr, "password123");
    QCOMPARE(acc.othersStr, "Additional info");

    // 测试默认构造函数
    AccountInfo defaultAcc;
    QVERIFY(defaultAcc.typeStr.isEmpty());
    QVERIFY(defaultAcc.nameStr.isEmpty());
    QVERIFY(defaultAcc.accountStr.isEmpty());
    QVERIFY(defaultAcc.pwdStr.isEmpty());
    QVERIFY(defaultAcc.othersStr.isEmpty());
  }

  void test_keyword_search() {
    AccountInfo acc("Email", "Personal Account", "user@example.com", "securePwd", "Additional notes");

    // 测试空关键字
    QVERIFY(acc.IsContainsKeyWords(""));

    // 测试类型匹配
    QVERIFY(acc.IsContainsKeyWords("email"));
    QVERIFY(acc.IsContainsKeyWords("Email"));

    // 测试名称匹配
    QVERIFY(acc.IsContainsKeyWords("personal"));
    QVERIFY(acc.IsContainsKeyWords("Account"));

    // 测试账号匹配
    QVERIFY(acc.IsContainsKeyWords("user@"));
    QVERIFY(acc.IsContainsKeyWords("example.com"));

    // 测试密码匹配
    QVERIFY(acc.IsContainsKeyWords("secure"));
    QVERIFY(acc.IsContainsKeyWords("Pwd"));

    // 测试其他信息匹配
    QVERIFY(acc.IsContainsKeyWords("notes"));
    QVERIFY(acc.IsContainsKeyWords("Additional"));

    // 测试不匹配情况
    QVERIFY(!acc.IsContainsKeyWords("facebook"));
    QVERIFY(!acc.IsContainsKeyWords("123456"));

    // 测试部分匹配
    QVERIFY(acc.IsContainsKeyWords("per"));
    QVERIFY(acc.IsContainsKeyWords("com"));
  }

  void test_csv_conversion() {
    // 创建测试数据（包含特殊字符）
    AccountInfo original("Social Media", "Facebook", "fb_user", "fb_pass", "Notes with, comma\nand newline");

    // 转换为CSV
    QString csvLine = original.toCsvLine();

    // 验证CSV格式
    QString expected = "Social Media,Facebook,fb_user,fb_pass,Notes with\\, comma\\nand newline";
    QCOMPARE(csvLine, expected);

    // 从CSV解析
    AccountInfo parsed;
    QVERIFY(AccountInfo::FromCsvLine(csvLine, parsed));

    // 验证解析结果
    QCOMPARE(parsed.typeStr, original.typeStr);
    QCOMPARE(parsed.nameStr, original.nameStr);
    QCOMPARE(parsed.accountStr, original.accountStr);
    QCOMPARE(parsed.pwdStr, original.pwdStr);
    QCOMPARE(parsed.othersStr, original.othersStr);
  }

  void test_csv_edge_cases() {
    // 测试空值
    AccountInfo empty("", "", "", "", "");
    QString emptyCsv = empty.toCsvLine();
    QCOMPARE(emptyCsv, ",,,,");

    AccountInfo parsedEmpty;
    QVERIFY(AccountInfo::FromCsvLine(emptyCsv, parsedEmpty));
    QVERIFY(parsedEmpty.typeStr.isEmpty());
    QVERIFY(parsedEmpty.nameStr.isEmpty());
    QVERIFY(parsedEmpty.accountStr.isEmpty());
    QVERIFY(parsedEmpty.pwdStr.isEmpty());
    QVERIFY(parsedEmpty.othersStr.isEmpty());

    // 测试转义字符
    AccountInfo escapeTest("Type", "Name", "Account", "Password", "Comma\\, Backslash\\\\ Newline\nCarriage\rReturn");
    QString escapeCsv = escapeTest.toCsvLine();
    QString expected = "Type,Name,Account,Password,Comma\\\\\\, Backslash\\\\\\\\ Newline\\nCarriage\\rReturn";
    QCOMPARE(escapeCsv, expected);

    AccountInfo parsedEscape;
    QVERIFY(AccountInfo::FromCsvLine(escapeCsv, parsedEscape));
    QCOMPARE(parsedEscape.othersStr, escapeTest.othersStr);

    // 测试无效CSV行
    AccountInfo invalid;
    QVERIFY(!AccountInfo::FromCsvLine("only,three,fields", invalid));            // 字段不足
    QVERIFY(!AccountInfo::FromCsvLine("one,two,three,four,five,six", invalid));  // 字段过多
    QVERIFY(!AccountInfo::FromCsvLine("", invalid));                             // 空行
  }

  void test_modification_flag() {
    AccountInfo acc;

    // 初始状态
    QVERIFY(!acc.IsDetailModified());

    // 设置修改标志
    acc.SetDetailModified();
    QVERIFY(acc.IsDetailModified());

    // 清除修改标志
    acc.ClearDetailModified();
    QVERIFY(!acc.IsDetailModified());

    // 多次设置
    acc.SetDetailModified();
    acc.SetDetailModified();
    QVERIFY(acc.IsDetailModified());

    acc.ClearDetailModified();
    QVERIFY(!acc.IsDetailModified());
  }

  void test_const_correctness() {
    // 测试const方法
    const AccountInfo constAcc("Type", "Name", "Account", "Password", "Others");

    // 调用const方法
    QVERIFY(constAcc.IsContainsKeyWords("Type"));
    QString csv = constAcc.toCsvLine();
    QVERIFY(!csv.isEmpty());
    QVERIFY(!constAcc.IsDetailModified());

    // 修改标志方法有const限定，但实际修改mutable成员
    constAcc.SetDetailModified();
    QVERIFY(constAcc.IsDetailModified());

    constAcc.ClearDetailModified();
    QVERIFY(!constAcc.IsDetailModified());
  }
};

#include "AccountInfoTest.moc"
REGISTER_TEST(AccountInfoTest, false)
