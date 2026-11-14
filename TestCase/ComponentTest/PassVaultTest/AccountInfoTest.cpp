#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "AccountInfo.h"

class AccountInfoTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void constructor_properties_swap_ok() {
    // 测试构造函数
    AccountInfo acc("Email", "Personal", "user@example.com", "password123", "Additional info");
    {
      QCOMPARE(acc.typeStr, "Email");
      QCOMPARE(acc.nameStr, "Personal");
      QCOMPARE(acc.accountStr, "user@example.com");
      QCOMPARE(acc.pwdStr, "password123");
      QCOMPARE(acc.othersStr, "Additional info");
      QVERIFY(!acc.IsModified());
      QVERIFY(!acc.IsDeleted());
    }
    const AccountInfo acc_bkp{acc};

    // 测试默认构造函数
    AccountInfo defaultAcc;
    {
      QVERIFY(defaultAcc.typeStr.isEmpty());
      QVERIFY(defaultAcc.nameStr.isEmpty());
      QVERIFY(defaultAcc.accountStr.isEmpty());
      QVERIFY(defaultAcc.pwdStr.isEmpty());
      QVERIFY(defaultAcc.othersStr.isEmpty());
      QVERIFY(!defaultAcc.IsModified());
      QVERIFY(!defaultAcc.IsDeleted());
      defaultAcc.SetModified(true);
      defaultAcc.SetDeleted(true);
    }
    const AccountInfo defaultAccBkp{defaultAcc};

    defaultAcc.swap(acc);
    QCOMPARE(defaultAcc, acc_bkp);
    QCOMPARE(acc, defaultAccBkp);

    QVERIFY( AccountInfo::IsColumnEditableByUser(AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE)));
    QVERIFY( AccountInfo::IsColumnEditableByUser(AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE)));
    QVERIFY( AccountInfo::IsColumnEditableByUser(AccountInfo::Role2Column(AccountInfo::Role::ACCOUNT_STR_ROLE)));
    QVERIFY( AccountInfo::IsColumnEditableByUser(AccountInfo::Role2Column(AccountInfo::Role::PWD_STR_ROLE)));
    QVERIFY( AccountInfo::IsColumnEditableByUser(AccountInfo::Role2Column(AccountInfo::Role::OTHERS_STR_ROLE)));
    QVERIFY(!AccountInfo::IsColumnEditableByUser(AccountInfo::Role2Column(AccountInfo::Role::IS_MODIFIED_ROLE)));
    QVERIFY(!AccountInfo::IsColumnEditableByUser(AccountInfo::Role2Column(AccountInfo::Role::IS_DELETED_ROLE)));

    QCOMPARE(AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE), 0);
    QCOMPARE(AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE), 1);
    QCOMPARE(AccountInfo::Role2Column(AccountInfo::Role::ACCOUNT_STR_ROLE), 2);
    QCOMPARE(AccountInfo::Role2Column(AccountInfo::Role::PWD_STR_ROLE), 3);
    QCOMPARE(AccountInfo::Role2Column(AccountInfo::Role::OTHERS_STR_ROLE), 4);
    QCOMPARE(AccountInfo::Role2Column(AccountInfo::Role::IS_MODIFIED_ROLE), 5);
    QCOMPARE(AccountInfo::Role2Column(AccountInfo::Role::IS_DELETED_ROLE), 6);
  }

  void keyword_search_ok() {
    const AccountInfo acc("Email", "Personal Account", "user@example.com", "securePwd", "Additional notes");

    // 测试空关键字
    QVERIFY(acc.IsContainsKeyWords(""));

    // 匹配
    {
      QVERIFY(acc.IsContainsKeyWords("email"));
      QVERIFY(acc.IsContainsKeyWords("Email"));

      QVERIFY(acc.IsContainsKeyWords("personal"));
      QVERIFY(acc.IsContainsKeyWords("Account"));

      QVERIFY(acc.IsContainsKeyWords("user@"));
      QVERIFY(acc.IsContainsKeyWords("example.com"));

      QVERIFY(acc.IsContainsKeyWords("secure"));
      QVERIFY(acc.IsContainsKeyWords("Pwd"));

      QVERIFY(acc.IsContainsKeyWords("notes"));
      QVERIFY(acc.IsContainsKeyWords("Additional"));
    }

    // 不匹配
    {
      QVERIFY(!acc.IsContainsKeyWords("facebook"));
      QVERIFY(!acc.IsContainsKeyWords("123456"));
    }

    // 部分匹配
    {
      QVERIFY(acc.IsContainsKeyWords("per"));
      QVERIFY(acc.IsContainsKeyWords("com"));
    }
  }

  void csv_conversion_conversion() {
    const AccountInfo original("Social Media", "Facebook", "fb_user", "fb_pass", "Notes with, comma\nand newline");

    const QString expected = "Social Media,Facebook,fb_user,fb_pass,Notes with\\, comma\\nand newline";
    const QString csvLine = original.toCsvLine();
    QCOMPARE(csvLine, expected);

    AccountInfo parsed;
    QVERIFY(AccountInfo::FromCsvLine(csvLine, parsed));
    QCOMPARE(parsed, original);
  }

  void csv_border_cases_ok() {
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

  void modified_deleted_flag__ok() {
    AccountInfo acc;

    QVERIFY(!acc.IsModified());
    acc.SetModified(true);
    QVERIFY(acc.IsModified());
    acc.SetModified(false);
    QVERIFY(!acc.IsModified());

    QVERIFY(!acc.IsDeleted());
    acc.SetDeleted(true);
    QVERIFY(acc.IsDeleted());
    acc.SetDeleted(false);
    QVERIFY(!acc.IsDeleted());
  }
};

#include "AccountInfoTest.moc"
REGISTER_TEST(AccountInfoTest, false)
