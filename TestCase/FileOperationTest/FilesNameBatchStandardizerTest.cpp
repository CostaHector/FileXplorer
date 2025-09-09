#include <QtTest>
#include <QCoreApplication>

#include "FilesNameBatchStandardizer.h"
#include "PlainTestSuite.h"
#include "TDir.h"

/*
‌ASCII码表（字典序核心区间）‌

‌控制字符‌：0-31及127（不可见字符）14

示例：退格(8)、换行(10)、回车(13)
‌符号区间‌：

基础符号：32-47（含空格、!"#$%&'()*+,-./）16
中间符号：58-64（:;<=>?@）、91-96（[\]^_`）、123-126（{|}~）16
‌数字区间‌：48-57

对应字符：'0'(48) 至 '9'(57)35
‌大写字母‌：65-90

对应字符：'A'(65) 至 'Z'(90)35
‌小写字母‌：97-122

对应字符：'a'(97) 至 'z'(122)23
‌特殊说明‌

大小写字母差值固定为32（如'A'=65 → 'a'=97）46
字典序优先级：控制符 < 符号 < 数字 < 大写字母 < 小写字母15

 */

class FilesNameBatchStandardizerTest : public PlainTestSuite {
  Q_OBJECT

 public:
  FilesNameBatchStandardizerTest() : PlainTestSuite{} {}

 private slots:
  void test_files_batch_rename() {
    TDir tDir;
    QVERIFY(tDir.IsValid());
    const QString workPath{tDir.path()};
    const QList<FsNodeEntry> newEntryNodes{
        FsNodeEntry{"---C.txt", false, "files in C.txt"},
        FsNodeEntry{"A       B/C [A/C (A.txt", false, "files in A B/C-A/C-A.txt"},
        FsNodeEntry{"A - .txt", false, "files in A.txt"},
    };

    QVERIFY(tDir.createEntries(newEntryNodes) >= newEntryNodes.size());

    FilesNameBatchStandardizer frr;
    frr(workPath);

    const QList<FsNodeEntry> expectsEntryNodes{
        FsNodeEntry{"- C.txt", false, ""},              //
        FsNodeEntry{"A B", true, ""},                   //
        FsNodeEntry{"A B/C - A", true, ""},             //
        FsNodeEntry{"A B/C - A/C - A.txt", false, ""},  //
        FsNodeEntry{"A.txt", false, ""},                //
    };
    const auto& actualNodes = tDir.getEntries(false);
    QCOMPARE(actualNodes, expectsEntryNodes);
  }

  void test_files_folders_rename() {
    TDir tDir;
    QVERIFY(tDir.IsValid());
    const QString workPath{tDir.path()};
    const QList<FsNodeEntry> newEntryNodes{
        FsNodeEntry{"- .C", true, ""},
        FsNodeEntry{"A       B", true, ""},
        FsNodeEntry{"Movie – C", true, ""},
        FsNodeEntry{"A       B/C [A", true, ""},
    };

    QVERIFY(tDir.createEntries(newEntryNodes) >= newEntryNodes.size());

    FilesNameBatchStandardizer frr;
    frr(workPath);

    const QList<FsNodeEntry> expectsEntryNodes{
        FsNodeEntry{".C", true, ""},         //
        FsNodeEntry{"A B", true, ""},        //
        FsNodeEntry{"A B/C - A", true, ""},  //
        FsNodeEntry{"Movie - C", true, ""},   //
    };
    const auto& actualNodes = tDir.getEntries(false);
    QCOMPARE(actualNodes, expectsEntryNodes);
  }
};

#include "FilesNameBatchStandardizerTest.moc"
REGISTER_TEST(FilesNameBatchStandardizerTest, false)
