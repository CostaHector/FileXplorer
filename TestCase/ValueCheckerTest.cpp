#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "ValueChecker.h"

class ValueCheckerTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void int_with_range_ok() {
    ValueChecker vc(0, 10);
    QCOMPARE(vc(0), true);
    QCOMPARE(vc(9), true);
    QCOMPARE(vc(10), false);

    QCOMPARE(vc.valueToString(5), "5");
    QCOMPARE(vc.strToQVariant("7"), QVariant(7));

    QCOMPARE(vc.strToQVariant("abc"), QVariant(0));
  }

  void switch_string_checker_ok() {
    ValueChecker vc({'0', '1'}, 3);

    QCOMPARE(vc("010"), true);
    QCOMPARE(vc("101010"), true);

    QCOMPARE(vc("01"), false);  // 太短
    QCOMPARE(vc("012"), false); // 包含非法字符
    QCOMPARE(vc("abc"), false); // 完全非法

    QCOMPARE(vc.valueToString("101"), "101");
    QCOMPARE(vc.strToQVariant("010"), QVariant("010"));
  }

  void candidate_string_checker_ok() {
    ValueChecker vc({"jpg", "png", "gif"}, VALUE_CHECKER_TYPE::CANDIDATE_STRING);

    // 有效候选
    QCOMPARE(vc("jpg"), true);
    QCOMPARE(vc("png"), true);

    // 无效候选
    QCOMPARE(vc("bmp"), false);
    QCOMPARE(vc(""), false);

    // 类型转换
    QCOMPARE(vc.valueToString("gif"), "gif");
    QCOMPARE(vc.strToQVariant("png"), QVariant("png"));
  }

  void ext_specified_file_checker_ok() {
    ValueChecker vc({".txt", ".cpp"}, VALUE_CHECKER_TYPE::EXT_SPECIFIED_FILE_PATH);

    // 有效文件
    QString currentFile = __FILE__;  // 当前源文件路径
    QCOMPARE(vc(currentFile), true); // 当前文件存在且扩展名匹配

    // 无效文件
    QString invalidFile = "random/path/to/nonexistent/file.bmp";
    QCOMPARE(vc(invalidFile), false); // 文件不存在
    QCOMPARE(vc(""), false);          // 空路径

    // 类型转换
    QCOMPARE(vc.valueToString(currentFile), currentFile);
    QCOMPARE(vc.strToQVariant(currentFile), QVariant(currentFile));
  }

  void file_exist_checker_ok() {
    ValueChecker vc(VALUE_CHECKER_TYPE::FILE_PATH);

    // 有效文件
    QString existingFile = __FILE__; // 当前源文件路径
    QCOMPARE(vc(existingFile), true);

    // 无效文件
    QString nonExistingFile = "random/path/to/nonexistent/file.txt";
    QCOMPARE(vc(nonExistingFile), false);
    QString folderPath = QFileInfo(__FILE__).absolutePath(); // 当前目录
    QCOMPARE(vc(folderPath), false);                         // 文件夹不是文件

    // 类型转换
    QCOMPARE(vc.valueToString(existingFile), existingFile);
    QCOMPARE(vc.strToQVariant(existingFile), QVariant(existingFile));
  }

  void folder_exist_checker_ok() {
    ValueChecker vc(VALUE_CHECKER_TYPE::FOLDER_PATH);

    // 有效文件夹
    QString existingFolder = QFileInfo(__FILE__).absolutePath(); // 当前目录
    QCOMPARE(vc(existingFolder), true);

    // 无效文件夹
    QString nonExistingFolder = "random/path/to/nonexistent/folder";
    QCOMPARE(vc(nonExistingFolder), false);
    QString filePath = __FILE__;   // 当前源文件路径
    QCOMPARE(vc(filePath), false); // 文件不是文件夹

    // 类型转换
    QCOMPARE(vc.valueToString(existingFolder), existingFolder);
    QCOMPARE(vc.strToQVariant(existingFolder), QVariant(existingFolder));
  }

  void value_conversion_ok() {
    // 测试布尔值
    ValueChecker boolVc(VALUE_CHECKER_TYPE::PLAIN_BOOL);
    QCOMPARE(boolVc.valueToString(true), "1");
    QCOMPARE(boolVc.valueToString(false), "0");
    QCOMPARE(boolVc.strToQVariant("true"), QVariant(true));
    QCOMPARE(boolVc.strToQVariant("false"), QVariant(false));

    // 测试浮点数
    ValueChecker floatVc(VALUE_CHECKER_TYPE::PLAIN_FLOAT);
    QCOMPARE(floatVc.valueToString(3.14f), "3.14");
    QCOMPARE(floatVc.strToQVariant("2.718"), QVariant(2.718f));

    // 测试双精度数
    ValueChecker doubleVc(VALUE_CHECKER_TYPE::PLAIN_DOUBLE);
    QCOMPARE(doubleVc.valueToString(3.1415), "3.1415");
    QCOMPARE(doubleVc.strToQVariant("2.7182"), QVariant(2.7182));

    // 测试字符串列表
    ValueChecker listVc(VALUE_CHECKER_TYPE::QSTRING_LIST);
    QStringList testList = {"item1", "item2", "item3"};
    QCOMPARE(listVc.valueToString(testList), "item1\nitem2\nitem3");
    QCOMPARE(listVc.strToQVariant("a\nb\nc"), QVariant(QStringList({"a", "b", "c"})));
  }

  void default_checker_behavior_ok() {
    // 测试未实现的类型
    ValueChecker errorVc(VALUE_CHECKER_TYPE::ERROR_TYPE);
    QCOMPARE(errorVc("anything"), true);
  }

  void boundary_conditions_ok() {
    // 空候选列表
    ValueChecker emptyCandidateVc(QStringList(), VALUE_CHECKER_TYPE::CANDIDATE_STRING);
    QCOMPARE(emptyCandidateVc("any"), true); // 空候选列表应接受任何输入

    // 最小整数边界
    ValueChecker minIntVc(INT_MIN, INT_MAX);
    QCOMPARE(minIntVc(INT_MIN), true);
    QCOMPARE(minIntVc(INT_MIN - 1LL), false); // 使用LL避免整数溢出

    // 最大整数边界
    QCOMPARE(minIntVc(INT_MAX - 1), true);
    QCOMPARE(minIntVc(INT_MAX), false);

    // 空路径
    ValueChecker fileVc(VALUE_CHECKER_TYPE::FILE_PATH);
    QCOMPARE(fileVc(""), false);

    ValueChecker folderVc(VALUE_CHECKER_TYPE::FOLDER_PATH);
    QCOMPARE(folderVc(""), false);

    // 无效路径
    ValueChecker extVc({"txt"}, VALUE_CHECKER_TYPE::EXT_SPECIFIED_FILE_PATH);
    QCOMPARE(extVc("invalid/path/with.txt"), false);
  }
};

#include "ValueCheckerTest.moc"
REGISTER_TEST(ValueCheckerTest, false)
