#include <QCoreApplication>
#include <QtTest>

#include "FileSystemTestSuite.h"
#include <QDir>

class FileSystemHelperTest : public FileSystemTestSuite {
  Q_OBJECT
 public:
  FileSystemHelperTest() : FileSystemTestSuite{"TestEnv_FileSystemHelper", true, false} {}

 private slots:
  void test_FileSystemHelper() {
    const QString readMeContents{"jahhhhhhhhhhhhhhhhaaaworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworldworld"};
    m_rootHelper << FileSystemNode{"directory", true, ""}                 //
                 << FileSystemNode{"mdfile.md", false, "waaaa"}           //
                 << FileSystemNode{"readme.txt", false, readMeContents};  //
    QCOMPARE(m_rootHelper.GetFileContent("mdfile.md"), "waaaa");
    const FileSystemNode expectNode{"directory", true, ""};
    QCOMPARE(m_rootHelper.GetNode("directory", false), expectNode);

    FileSystemHelper subHelper = m_rootHelper.GetSubHelper("directory");
    subHelper << FileSystemNode{".gitignore", false, "somecontents"};
    QStringList lvl1Items = QDir{mTestPath, "", QDir::SortFlag::Name, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot}.entryList();
    QStringList expectItems;
    expectItems << "directory"
                << "mdfile.md"
                << "readme.txt";
    QCOMPARE(lvl1Items, expectItems);
    QCOMPARE(subHelper.GetFileContent(".gitignore"), "somecontents");

    QVERIFY(m_rootHelper.EraseFileSystemTree());
    QVERIFY(QDir{mTestPath}.isEmpty(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot));
  }
};

#include "FileSystemHelperTest.moc"
FileSystemHelperTest g_FileSystemHelperTest;
