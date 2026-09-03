#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "AutoRollbackFileContentModify.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"

class AutoRollbackFileContentModifyTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir tDir;
private slots:
  void initTestCase() { QVERIFY(tDir.IsValid()); }

  void init() { tDir.ClearAll(); }

  void test_ReplaceMode_GlobalReplace() {
    const QString filePath = "test.txt";
    const QByteArray originalContent = "apple banana apple\nbanana apple";
    QVERIFY(tDir.touch(filePath, originalContent));

    {
      AutoRollbackFileContentModify modifier(tDir.itemPath(filePath), "apple", "orange");
      QVERIFY(modifier.Execute());

      // 验证全局替换：所有"apple"都被替换为"orange"
      QByteArray modified = readFileBytes(filePath);
      QCOMPARE(modified, QByteArray("orange banana orange\nbanana orange"));
    }

    // 验证回滚后内容完全一致
    QCOMPARE(readFileBytes(filePath), originalContent);
  }

  void test_ReplaceMode_CRLF_Preservation() {
    const QString filePath = "crlf.txt";
    const QByteArray originalContent = "line1\r\nline2\r\nline3\r\n";
    QVERIFY(tDir.touch(filePath, originalContent));

    {
      AutoRollbackFileContentModify modifier(tDir.itemPath(filePath), "line2", "replaced");
      QVERIFY(modifier.Execute());

      // 验证替换后CRLF保持不变
      QByteArray modified = readFileBytes(filePath);
      QCOMPARE(modified, QByteArray("line1\r\nreplaced\r\nline3\r\n"));
    }

    QCOMPARE(readFileBytes(filePath), originalContent);
  }

  void test_FullReplaceMode_LF_Preservation() {
    const QString filePath = "lf.py";
    const QByteArray originalContent = "def foo():\n    return 42\n";
    QVERIFY(tDir.touch(filePath, originalContent));

    {
      AutoRollbackFileContentModify modifier(tDir.itemPath(filePath), "print('Hello')");
      QVERIFY(modifier.Execute());

      // 验证完全替换为新内容
      QCOMPARE(readFileBytes(filePath), QByteArray("print('Hello')"));
    }

    QCOMPARE(readFileBytes(filePath), originalContent);
  }

  void test_ReplaceMode_SpecialCharacters() {
    const QString filePath = "special.txt";
    const QByteArray originalContent = "content with \t tabs \r\n and \0 nulls";
    QVERIFY(tDir.touch(filePath, originalContent));

    {
      AutoRollbackFileContentModify modifier(tDir.itemPath(filePath), "tabs", "spaces");
      QVERIFY(modifier.Execute());

      QByteArray modified = readFileBytes(filePath);
      QCOMPARE(modified, QByteArray("content with \t spaces \r\n and \0 nulls"));
    }

    QCOMPARE(readFileBytes(filePath), originalContent);
  }

  void test_FullReplaceMode_EmptyFile() {
    const QString filePath = "empty.txt";
    const QByteArray originalContent;
    QVERIFY(tDir.touch(filePath, originalContent));

    {
      AutoRollbackFileContentModify modifier(tDir.itemPath(filePath), "new content");
      QVERIFY(modifier.Execute());
      QCOMPARE(readFileBytes(filePath), QByteArray("new content"));
    }

    QVERIFY(readFileBytes(filePath).isEmpty());
  }

  void test_ReplaceMode_NoMatch() {
    const QString filePath = "nomatch.txt";
    const QByteArray originalContent = "original content";
    QVERIFY(tDir.touch(filePath, originalContent));

    {
      AutoRollbackFileContentModify modifier(tDir.itemPath(filePath), "missing", "replacement");
      QVERIFY(modifier.Execute());

      // 没有匹配项时内容应保持不变
      QCOMPARE(readFileBytes(filePath), originalContent);
    }

    QCOMPARE(readFileBytes(filePath), originalContent);
  }

private:
  // 辅助函数：二进制读取文件内容
  QByteArray readFileBytes(const QString& relativePath) const {
    QFile file(tDir.itemPath(relativePath));
    if (!file.open(QIODevice::ReadOnly))
      return QByteArray();
    return file.readAll();
  }
};

#include "AutoRollbackFileContentModifyTest.moc"
REGISTER_TEST(AutoRollbackFileContentModifyTest, false)
