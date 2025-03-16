#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include <QFileInfo>
#include "pub/MyTestSuite.h"
#include "Tools/PathTool.h"

using namespace ::PATHTOOL;

class PathToolTest : public MyTestSuite {
  Q_OBJECT

 public:
 private slots:
  void initTestCase() { qDebug() << "PathToolTest start"; }

  void cleanupTestCase() { qDebug() << "PathToolTest end"; }

  void test_GetWinStdPath() {
#ifdef WIN32
    QCOMPARE(GetWinStdPath("C:"), "C:/");
    QCOMPARE(GetWinStdPath("X:"), "X:/");
    QCOMPARE(GetWinStdPath("F1:"), "F1:/");
#else
    QCOMPARE(GetWinStdPath("/"), "/");
    QCOMPARE(GetWinStdPath("/home"), "/home");
#endif
  }
  void test_StripTrailingSlash() {
    // Disk A to Disk Z
    QCOMPARE(StripTrailingSlash("A:/"), "A:/");
    QCOMPARE(StripTrailingSlash("C:/"), "C:/");
    QCOMPARE(StripTrailingSlash("Z:/"), "Z:/");
    QCOMPARE(StripTrailingSlash("XX:/A/"), "XX:/A");
    QCOMPARE(StripTrailingSlash("XX:/"), "XX:/");
    QCOMPARE(StripTrailingSlash("XX:"), "XX:");
    QCOMPARE(StripTrailingSlash("/home/user/"), "/home/user");
    QCOMPARE(StripTrailingSlash("/home/user"), "/home/user");
    QCOMPARE(StripTrailingSlash("/"), "/");
  }

  void test_linkAndLocalPath() {
    QCOMPARE(linkPath("anyfile.txt"), "file:///anyfile.txt");
    QCOMPARE(linkPath("kmeans.data"), "file:///kmeans.data");
    QCOMPARE(linkPath("henry cavill.png"), "file:///henry cavill.png");

    QCOMPARE(localPath("file:///anyfile.txt"), "anyfile.txt");
    QCOMPARE(localPath("file:///kmeans.data"), "kmeans.data");
    QCOMPARE(localPath("file:///henry cavill.png"), "henry cavill.png");
  }

  void test_forSearchPath() {
    QCOMPARE(forSearchPath("C:/home/movie/vids/file.mp4"), "movie/vids/file.mp4");
    QCOMPARE(forSearchPath("C:/file.mp4"), "C:/file.mp4");
  }

  void test_commonprefixLongePath() {
    const QStringList paths{"E:/py/NameStandardlizeTestFolder/New Folder 20231020222814/New Text Document 20231125234056.txt",  //
                            "E:/py/NameStandardlizeTestFolder/New Text Document 20231020222955.txt"};
    const QString expectPrepath = "E:/py/NameStandardlizeTestFolder";
    const QString actualPrepath = longestCommonPrefix(paths);
    QCOMPARE(actualPrepath, expectPrepath);
  }

  void test_longestCommonPrefix_path_scene() {
    QString actualPrepath = longestCommonPrefix({"E:/115/ABC", "E:/115/d"});
    QString expectPrepath = "E:/115";
    QCOMPARE(actualPrepath, expectPrepath);
  }

  void test_longestCommonPrefix_samepath_scene() {
    QString actualPrepath = longestCommonPrefix({"E:/115/ABC", "E:/115/ABC"});
    QString expectPrepath = "E:/115";
    QCOMPARE(actualPrepath, expectPrepath);
  }

  void test_commonPrefix_EmptyElementList() {
    const QStringList emptyPaths;
    const QString expectPrepath = "";
    QString prepath;
    QStringList rels;
    std::tie(prepath, rels) = GetLAndRels(emptyPaths);
    QCOMPARE(prepath, expectPrepath);
    QVERIFY(rels.isEmpty());
  }

  void test_commonprefix_OneElementList() {
    const QStringList paths{"E:/115/0303/12"};
    const QString expectPrepath = "E:/115/0303";
    QString prepath;
    QStringList rels;
    std::tie(prepath, rels) = GetLAndRels(paths);
    QCOMPARE(prepath, expectPrepath);
  }

  void test_commonprefix() {
    const QStringList paths{
        "E:/Chris Pine/Monday/12",    //
        "E:/Chris Pine/Feb/12",       //
        "E:/Chris Pine/Decemeber/13"  //
    };
    const QString expectPrepath = "E:/Chris Pine";
    const QStringList expectRels{
        "Monday/12",    //
        "Feb/12",       //
        "Decemeber/13"  //
    };
    QString prepath;
    QStringList rels;
    std::tie(prepath, rels) = GetLAndRels(paths);
    QCOMPARE(prepath, expectPrepath);
  }

  void test_commonprefixFusion() {
    const QStringList paths{
        "E:/Hemsworth Chris/0303/12",  //
        "E:/Hemsworth Liam/0303/12",   //
        "E:/Hemsworth Luke/0303/12",   //
        "E:/Hemsworth Daniel/0303/13"  //
    };
    const QString expectPrepath = "E:";
    const QStringList expectRels{
        "Hemsworth Chris/0303/12",   //
        "Hemsworth Liam/0303/12",    //
        "Hemsworth Luke/0303/12",    //
        "Hemsworth Daniel/0303/13",  //
    };
    QString prepath;
    QStringList rels;
    std::tie(prepath, rels) = GetLAndRels(paths);
    QCOMPARE(prepath, expectPrepath);
    QCOMPARE(rels, expectRels);
  }

  void test_dirName() {
    QCOMPARE(dirName("rootPath/Any/Relative/Path/File"), "Path");
    QCOMPARE(dirName("rootPath/Relative/File"), "Relative");
  }

  void test_fileName() {
    QCOMPARE(fileName("rootPath/Relative/File"), "File");
    QCOMPARE(fileName("rootPath/Relative/Chris Evans.png"), "Chris Evans.png");
    QCOMPARE(fileName("Chris Evans.jpeg"), "Chris Evans.jpeg");
  }

  void test_RelativePath2File_InvalidInput() {
    // path without slash
    QCOMPARE(RelativePath2File(0, "path with no slash"), "");
    // rootPathLen too large
    QCOMPARE(RelativePath2File(225, "C:/home/Huge Jackman"), "");
  }

  void test_RelativePath2File_ok() {
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

  void test_GetBaseName_folder() {
    QCOMPARE(GetBaseName("C:/home/folder"), "folder");
    QCOMPARE(GetBaseName("C:/home/any movie name folder sc.1 - performer 1, performer 2"),  //
             "any movie name folder sc.1 - performer 1, performer 2");
  }

  void test_GetBaseName_file() {
    QCOMPARE(GetBaseName("C:/home/file.m"), "file");
    QCOMPARE(GetBaseName("a.txt"), "a");
    QCOMPARE(GetBaseName("a"), "a");
    QCOMPARE(GetBaseName("any movie name sc.1 - performer 1, performer 2"), "any movie name sc.1 - performer 1, performer 2");
    QCOMPARE(GetBaseName("any movie name sc.1 - performer 1, performer 2.txt"), "any movie name sc.1 - performer 1, performer 2");
    // scene. \d should not be treated as file extension
    QCOMPARE(GetBaseName("Michael Fassbender sc.1"), "Michael Fassbender sc.1");
    QCOMPARE(GetBaseName("Michael Fassbender sc. 1"), "Michael Fassbender sc. 1");
  }

  void test_GetBaseNameExt() {
    QCOMPARE(GetBaseNameExt("C:/home/file.m"), std::make_pair(QString("file"), QString(".m")));
    QCOMPARE(GetBaseNameExt("a.txt"), std::make_pair(QString("a"), QString(".txt")));
    QCOMPARE(GetBaseNameExt("a"), std::make_pair(QString("a"), QString("")));
    QCOMPARE(GetBaseNameExt("C:/home/any movie name sc.1 - performer 1, performer 2.txt"), std::make_pair(QString("any movie name sc.1 - performer 1, performer 2"), QString(".txt")));
  }

  void test_GetBaseNameExt_Scene01() {
    QCOMPARE(GetBaseNameExt("Michael Fassbender sc. 1"), std::make_pair(QString("Michael Fassbender sc. 1"), QString("")));
    QCOMPARE(GetBaseNameExt("Michael Fassbender sc.1"), std::make_pair(QString("Michael Fassbender sc.1"), QString("")));
  }

  void test_FileNameRemoveExt() {
    QCOMPARE(GetFileNameExtRemoved("C:/home/file.m"), "C:/home/file");
    QCOMPARE(GetFileNameExtRemoved("a.txt"), "a");
  }

  void test_GetBaseNameDotBeforeSlash() {
    QCOMPARE(GetBaseName("C:/.a/b"), "b");
    QCOMPARE(GetBaseName("C:/.a/any movie name"), "any movie name");
  }

  void test_isLinuxRootOrWinEmpty() {
#ifdef WIN32
    QVERIFY(isLinuxRootOrWinEmpty(""));
#else
    QVERIFY(isLinuxRootOrWinEmpty("/"));
#endif
  }
  void test_isRootOrEmpty() {
#ifdef WIN32
    QVERIFY(isRootOrEmpty(""));
    QVERIFY(isRootOrEmpty("C:/"));
    QVERIFY(!isRootOrEmpty("C:/home"));
#else
    QVERIFY(isRootOrEmpty("/"));
    QVERIFY(!isRootOrEmpty("/home"));
#endif
  }
};

#include "PathToolTest.moc"

PathToolTest g_pathToolTest;
// QTEST_MAIN(PathToolTest)
