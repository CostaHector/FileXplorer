#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include <QFileInfo>
#include "Tools/PathTool.h"

using namespace ::PATHTOOL;

class PathToolTest : public QObject {
  Q_OBJECT

 public:
 private slots:
  void initTestCase();
  void cleanupTestCase();

  void init();
  void cleanup();

  void test_commonprefixOfEmptyElementList();
  void test_commonprefixOfOneElementList();
  void test_commonprefix();
  void test_commonprefixFusion();
  void test_commonprefixLongePath();

  void test_longestCommonPrefix_path_scene();
  void test_longestCommonPrefix_samepath_scene();

  void test_dirName();
  void test_fileName();
  void test_RelativePath2File_InvalidInput();
  void test_RelativePath2File_ok();

  void test_GetBaseNameExt();
};

void PathToolTest::initTestCase() {
  qDebug() << "PathToolTest start to initTestCase";
}

void PathToolTest::cleanupTestCase() {
  qDebug() << "PathToolTest start to initTestCase";
}

void PathToolTest::init() {}

void PathToolTest::cleanup() {}

void PathToolTest::test_commonprefixOfEmptyElementList() {
  const QStringList paths;
  const QString expectPrepath = "";
  QString prepath;
  QStringList Rels;
  std::tie(prepath, Rels) = GetLAndRels(paths);
  QCOMPARE(prepath, expectPrepath);
}

void PathToolTest::test_commonprefixOfOneElementList() {
  const QStringList paths{"E:/115/0303/12"};
  const QString expectPrepath = "E:/115/0303";
  QString prepath;
  QStringList Rels;
  std::tie(prepath, Rels) = GetLAndRels(paths);
  QCOMPARE(prepath, expectPrepath);
}

void PathToolTest::test_commonprefix() {
  const QStringList paths{"E:/115/0303/12", "E:/115/03B03/12", "E:/115/0303/13"};
  const QString expectPrepath = "E:/115";
  QString prepath;
  QStringList Rels;
  std::tie(prepath, Rels) = GetLAndRels(paths);
  QCOMPARE(prepath, expectPrepath);
}

void PathToolTest::test_commonprefixFusion() {
  const QStringList paths{"E:/115/0303/12", "E:/11A5/0303/12", "E:/115/03B03/12", "E:/11A5/0303/13"};
  const QString expectPrepath = "E:";
  QString prepath;
  QStringList Rels;
  std::tie(prepath, Rels) = GetLAndRels(paths);
  QCOMPARE(prepath, expectPrepath);
}

void PathToolTest::test_commonprefixLongePath() {
  QString actualPrepath = longestCommonPrefix({"E:/py/NameStandardlizeTestFolder/New Folder 20231020222814/New Text Document 20231125234056.txt",
                                               "E:/py/NameStandardlizeTestFolder/New Text Document 20231020222955.txt"});
  QString expectPrepath = "E:/py/NameStandardlizeTestFolder";
  QCOMPARE(actualPrepath, expectPrepath);
}

void PathToolTest::test_longestCommonPrefix_path_scene() {
  QString actualPrepath = longestCommonPrefix({"E:/115/ABC", "E:/115/d"});
  QString expectPrepath = "E:/115";
  QCOMPARE(actualPrepath, expectPrepath);
}

void PathToolTest::test_longestCommonPrefix_samepath_scene() {
  QString actualPrepath = longestCommonPrefix({"E:/115/ABC", "E:/115/ABC"});
  QString expectPrepath = "E:/115";
  QCOMPARE(actualPrepath, expectPrepath);
}

void PathToolTest::test_dirName() {
  // rootPath/Any/Relative/Path/File = > Path
  // rootPath/Relative/File = > Relative
  QCOMPARE(dirName("rootPath/Any/Relative/Path/File"), "Path");
  QCOMPARE(dirName("rootPath/Relative/File"), "Relative");
}
void PathToolTest::test_fileName() {
  // rootPath/Relative/File = > File, just like QFileInfo("rootPath/Relative/File").fileName()
  QCOMPARE(fileName("rootPath/Relative/File"), "File");
}

void PathToolTest::test_RelativePath2File_InvalidInput() {
  QCOMPARE(RelativePath2File(strlen(""), "no slash path"), "");
}

void PathToolTest::test_RelativePath2File_ok() {
  QCOMPARE(strlen("C:/home"), 7);
  // "C:", C:/home/file.txt = > /home/
  // "rootPath", rootPath/Any/Relative/Path/File = > /Any/Relative/Path/
  // "rootPath", rootPath/Relative/File = > /Relative/
  // "rootPath", rootPath/File = > /
  QCOMPARE(RelativePath2File(strlen("C:"), "C:/home/file.txt"), "/home/");
  QCOMPARE(RelativePath2File(strlen("rootPath"), "rootPath/Any/Relative/Path/File"), "/Any/Relative/Path/");
  QCOMPARE(RelativePath2File(strlen("rootPath"), "rootPath/Relative/File"), "/Relative/");
  QCOMPARE(RelativePath2File(strlen("rootPath"), "rootPath/File"), "/");

  QCOMPARE(RelativePath2File(strlen("C:"), "C:/home/file.txt", strlen("file.txt")), "/home/");
  QCOMPARE(RelativePath2File(strlen("rootPath"), "rootPath/Any/Relative/Path/File", 4), "/Any/Relative/Path/");
  QCOMPARE(RelativePath2File(strlen("rootPath"), "rootPath/Relative/File", 4), "/Relative/");
  QCOMPARE(RelativePath2File(strlen("rootPath"), "rootPath/File", 4), "/");
}

void PathToolTest::test_GetBaseNameExt() {
  QCOMPARE(GetBaseNameExt("C:/home/file.m"), std::make_pair(QString("file"), QString(".m")));
  QCOMPARE(GetBaseNameExt("a.txt"), std::make_pair(QString("a"), QString(".txt")));
}

//QTEST_MAIN(PathToolTest)
#include "PathToolTest.moc"
