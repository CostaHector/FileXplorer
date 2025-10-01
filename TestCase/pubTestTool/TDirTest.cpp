#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "TDir.h"
#include "EndToExposePrivateMember.h"

class TDirTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir tDir;
private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());
  }

  void init() {
    QVERIFY(tDir.ClearAll());
  }

  // 测试基本文件操作
  void test_FileOperations() {
    // 创建文件
    const QString fileName = "test.txt";
    const QByteArray content = "Hello World";
    QVERIFY(tDir.touch(fileName, content));

    // 验证文件存在
    QVERIFY(tDir.fileExists(fileName));

    // 验证文件内容
    QVERIFY(tDir.checkFileContents(fileName, {"Hello World"}));

    // 验证快照功能
    QSet<QString> snapshot = tDir.Snapshot();
    QVERIFY(snapshot.contains(fileName));

    // 验证条目列表
    QStringList entries = tDir.entryList();
    QVERIFY(entries.contains(fileName));
  }

  // 测试目录操作
  void test_DirectoryOperations() {
    // 创建目录
    const QString dirName = "test_dir";
    QVERIFY(tDir.mkdir(dirName));

    // 验证目录存在
    QVERIFY(tDir.dirExists(dirName));

    // 在目录中创建文件
    const QString filePath = dirName + "/file.txt";
    QVERIFY(tDir.touch(filePath));

    // 验证嵌套文件存在
    QVERIFY(tDir.fileExists(filePath));

    // 验证快照包含嵌套文件
    QSet<QString> snapshot = tDir.Snapshot();
    QVERIFY(snapshot.contains(filePath));
  }

  // 测试批量创建条目
  void test_CreateEntries() {
    // 准备批量条目
    QList<FsNodeEntry> entries = {
        {"dir1", true, ""},
        {"dir1/file1.txt", false, "Content 1"},
        {"dir2", true, ""},
        {"dir2/file2.txt", false, "Content 2"}
    };

    // 创建条目
    int created = tDir.createEntries(entries);
    QCOMPARE(created, 4);

    // 验证所有条目存在
    QVERIFY(tDir.dirExists("dir1"));
    QVERIFY(tDir.fileExists("dir1/file1.txt"));
    QVERIFY(tDir.dirExists("dir2"));
    QVERIFY(tDir.fileExists("dir2/file2.txt"));

    // 验证文件内容
    QVERIFY(tDir.checkFileContents("dir1/file1.txt", {"Content 1"}));
    QVERIFY(tDir.checkFileContents("dir2/file2.txt", {"Content 2"}));
  }

  // 测试获取条目
  void test_GetEntries() {
    // 创建测试结构
    QVERIFY(tDir.mkdir("folder"));
    QVERIFY(tDir.touch("folder/file.txt", "File content"));

    // 获取所有条目（不包含内容）
    QList<FsNodeEntry> entries = tDir.getEntries(false);
    QCOMPARE(entries.size(), 2);

    // 验证条目信息
    for (const auto& entry : entries) {
      if (entry.relativePathToNode == "folder") {
        QVERIFY(entry.isDir);
      } else if (entry.relativePathToNode == "folder/file.txt") {
        QVERIFY(!entry.isDir);
      }
    }

    // 获取所有条目（包含内容）
    entries = tDir.getEntries(true);
    for (const auto& entry : entries) {
      if (entry.relativePathToNode == "folder/file.txt") {
        QCOMPARE(entry.contents, QByteArray("File content"));
      }
    }
  }

  // 测试清理功能
  void test_ClearAll() {
    // 创建一些文件和目录
    QVERIFY(tDir.mkdir("dir1"));
    QVERIFY(tDir.touch("file1.txt"));
    QVERIFY(tDir.touch("dir1/file2.txt"));

    // 验证非空
    QVERIFY(!tDir.IsEmpty());

    // 清理
    QVERIFY(tDir.ClearAll());

    // 验证为空
    QVERIFY(tDir.IsEmpty());
  }

  // 测试路径构建功能
  void test_PathFunctions() {
    // 验证基础路径
    QVERIFY(!tDir.path().isEmpty());

    // 验证项目路径构建
    const QString itemName = "test.txt";
    QString itemPath = tDir.itemPath(itemName);
    QVERIFY(itemPath.endsWith("/" + itemName));

    // 创建文件验证路径正确
    QVERIFY(tDir.touch(itemName));
    QVERIFY(QFile::exists(itemPath));
  }

  // 测试空目录行为
  void test_EmptyDirectory() {
    // 初始应为空
    QVERIFY(tDir.IsEmpty());

    // 快照应为空
    QSet<QString> snapshot = tDir.Snapshot();
    QVERIFY(snapshot.isEmpty());

    // 条目列表应为空
    QStringList entries = tDir.entryList();
    QVERIFY(entries.isEmpty());
  }

  // 测试大小写敏感文件检查（Windows特定）
  void test_CaseSensitiveFileCheck() {
#ifdef _WIN32
    const QString fileName = "CaseTest.txt";
    QVERIFY(tDir.touch(fileName));

    // 验证大小写不敏感检查
    QVERIFY(tDir.fileExists("CASETEST.TXT", false));

    // 验证大小写敏感检查
    QVERIFY(tDir.fileExists(fileName, true));
    QVERIFY(!tDir.fileExists("CASETEST.TXT", true));
#endif
  }
};

#include "TDirTest.moc"
REGISTER_TEST(TDirTest, false)
