#include <QCoreApplication>
#include <QtTest>

#include "TestCase/pub/MyTestSuite.h"
#include "TestCase/PathRelatedTool.h"
#include "public/PathTool.h"

using namespace ::PathTool;
class PathToolTest : public MyTestSuite {
  Q_OBJECT

 public:
  PathToolTest() : MyTestSuite{false} {}
 private slots:
  void test_project_name_from_marco() {
    QCOMPARE(PROJECT_NAME, QString("FileXplorer"));
  }

  void test_lib_files_path_correct() {
    QString libPath = GetPathByApplicationDirPath(FILE_REL_PATH::MEDIA_INFO_DLL);
    QVERIFY(QFile::exists(libPath));

    QString testRootDir = TestCaseRootPath();
    QString libPath2 = QDir::cleanPath(QDir(testRootDir).absoluteFilePath("../lib/MediaInfo.dll"));
    QCOMPARE(libPath, libPath2);

    QString batPath = GetPathByApplicationDirPath(FILE_REL_PATH::TERMINAL_OPEN_BATCH_FILE_PATH);
    QVERIFY(QFile::exists(batPath));
  }

  void test_cast_studio_file_path_exists() {
    QString perfPath = GetPathByApplicationDirPath(FILE_REL_PATH::PERFORMERS_TABLE);
    QVERIFY(QFile::exists(perfPath));
    QString akaPath = GetPathByApplicationDirPath(FILE_REL_PATH::AKA_PERFORMERS);
    QVERIFY(QFile::exists(akaPath));
    QString stdStudioPath = GetPathByApplicationDirPath(FILE_REL_PATH::STANDARD_STUDIO_NAME);
    QVERIFY(QFile::exists(stdStudioPath));
  }

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
#ifdef _WIN32
    // Disk A to Disk Z
    QCOMPARE(StripTrailingSlash("A:/"), "A:/");
    QCOMPARE(StripTrailingSlash("C:/"), "C:/");
    QCOMPARE(StripTrailingSlash("Z:/"), "Z:/");
    QCOMPARE(StripTrailingSlash("XX:/A/"), "XX:/A");
    QCOMPARE(StripTrailingSlash("XX:/"), "XX:/");
    QCOMPARE(StripTrailingSlash("XX:"), "XX:");
#endif
    QCOMPARE(StripTrailingSlash("/home/user/"), "/home/user");
    QCOMPARE(StripTrailingSlash("/home/user"), "/home/user");
    QCOMPARE(StripTrailingSlash("/"), "/");
  }

  void test_linkAndLocalPath() {
#ifdef _WIN32
    QCOMPARE(linkPath("anyfile.txt"), "file:///anyfile.txt");
    QCOMPARE(linkPath("C:/henry cavill.png"), "file:///C:/henry cavill.png");

    QCOMPARE(localPath("file:///anyfile.txt"), "anyfile.txt");
    QCOMPARE(localPath("file:///C:/henry cavill.png"), "C:/henry cavill.png");
#else
    // absolute path must
    QCOMPARE(linkPath("/home/henry cavill.png"), "file:///home/henry cavill.png");
    QCOMPARE(localPath("file:///home/henry cavill.png"), "/home/henry cavill.png");
#endif
  }

  void test_forSearchPath() {
    QCOMPARE(forSearchPath("C:/home/movie/vids/file.mp4"), "movie/vids/file.mp4");
    QCOMPARE(forSearchPath("C:/file.mp4"), "C:/file.mp4");
  }

  void test_StrCommonPrefix_basic() {
    QCOMPARE(StrCommonPrefix("", ""), "");
    QCOMPARE(StrCommonPrefix("C:/home", "C:/homie"), "C:/hom");
    QCOMPARE(StrCommonPrefix("C:/home/path", "C:/home/path/to"), "C:/home/path");
    QCOMPARE(StrCommonPrefix("C:/home/path", "D:/home/path/to"), "");
  }

  void test_longestCommonPrefix_3_path() {
    const QStringList paths{"E:/py/NameStandardlizeTestFolder/New Folder 20231020222814/New Text Document 20231125234056.txt",  //
                            "E:/py/NameStandardlizeTestFolder/New Text Document 20231020222955.txt",                            //
                            "E:/py/NameStandardlizeTestFolder/New Folder 20231020222814"};                                      //
    const QString expectPrepath = "E:/py/NameStandardlizeTestFolder";
    const QString actualPrepath = longestCommonPrefix(paths);
    QCOMPARE(actualPrepath, expectPrepath);
  }

  void test_longestCommonPrefix_2_path() {
    QString actualPrepath = longestCommonPrefix({"E:/115/ABC", "E:/115/A"});
    QString expectPrepath = "E:/115";
    QCOMPARE(actualPrepath, expectPrepath);
  }

  void test_longestCommonPrefix_same_path() {
    QString actualPrepath = longestCommonPrefix({"E:/115/ABC", "E:/115/ABC", "E:/115/ABC"});
    QString expectPrepath = "E:/115";
    QCOMPARE(actualPrepath, expectPrepath);
  }

  void test_GetLAndRels_empty() {
    const QStringList emptyPaths;
    const QString expectPrepath = "";
    QString prepath;
    QStringList rels;
    std::tie(prepath, rels) = GetLAndRels(emptyPaths);
    QCOMPARE(prepath, expectPrepath);
    QVERIFY(rels.isEmpty());
  }

  void test_GetLAndRels_1_path() {
    const QStringList paths{"E:/115/0303/12"};
    const QString expectCommonPrepath = "E:/115/0303";
    const QStringList expectRels{"12"};

    QString prepath;
    QStringList rels;
    std::tie(prepath, rels) = GetLAndRels(paths);
    QCOMPARE(prepath, expectCommonPrepath);
    QCOMPARE(rels, expectRels);
  }

  void test_GetLAndRels_BuiltInFileSystem_SelectionPath() {
    const QStringList paths{"E:/115/0303/12", "E:/115/0303/12.txt", "E:/115/0303/34.txt"};
    const QString expectCommonPrepath = "E:/115/0303";
    const QStringList expectRels{"12", "12.txt", "34.txt"};

    QString prepath;
    QStringList rels;
    std::tie(prepath, rels) = GetLAndRels(paths);
    QCOMPARE(prepath, expectCommonPrepath);
    QCOMPARE(rels, expectRels);
  }

  void test_commonprefix_basic() {
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
    QCOMPARE(GetBaseNameExt("C:/home/any movie name sc.1 - performer 1, performer 2.txt"),
             std::make_pair(QString("any movie name sc.1 - performer 1, performer 2"), QString(".txt")));
  }

  void test_GetBaseNameExt_Scene01() {
    QCOMPARE(GetBaseNameExt("Michael Fassbender sc. 1"), std::make_pair(QString("Michael Fassbender sc. 1"), QString("")));
    QCOMPARE(GetBaseNameExt("Michael Fassbender sc.1"), std::make_pair(QString("Michael Fassbender sc.1"), QString("")));
  }

  void test_FileNameRemoveExt() {
    QCOMPARE(GetFileNameExtRemoved("C:/home/file.m"), "C:/home/file");
    QCOMPARE(GetFileNameExtRemoved("a.txt"), "a");
  }

  void test_FileExtReplacedWithJson() {
    QString basename{"Kalman"};
    QStringList extensions{".m", ".rar", ".7z", ".mpeg"};
    const QString expectJsonName{basename + ".json"};
    for (const QString& extWithDot : extensions) {
      QString fileName{basename + extWithDot};
      QCOMPARE(FileExtReplacedWithJson(fileName), expectJsonName);
    }

    QCOMPARE(FileExtReplacedWithJson("file.utorrent"), "file.utorrent.json");
    QCOMPARE(FileExtReplacedWithJson("/home/to/file.txt"), "/home/to/file.json");
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

  void test_path_part_split() {
    QString preLeft, preRight;
    int lastSlashIndex = GetPrepathParts("C:/A/B/C.mp4", preLeft, preRight);
    QCOMPARE(QString{"C:/A/B/C.mp4"}.mid(lastSlashIndex + 1), "C.mp4");
    QCOMPARE(preLeft, "C:");
    QCOMPARE(preRight, "A/B");
    QCOMPARE(Path3Join("C:", "A/B", "C.mp4"), "C:/A/B/C.mp4");

    lastSlashIndex = GetPrepathParts("C:/A/C.mp4", preLeft, preRight);
    QCOMPARE(QString{"C:/A/C.mp4"}.mid(lastSlashIndex + 1), "C.mp4");
    QCOMPARE(preLeft, "");
    QCOMPARE(preRight, "C:/A");
    QCOMPARE(Path3Join("", "C:/A", "C.mp4"), "C:/A/C.mp4");

    lastSlashIndex = GetPrepathParts("C:/C.mp4", preLeft, preRight);
    QCOMPARE(QString{"C:/C.mp4"}.mid(lastSlashIndex + 1), "C.mp4");
    QCOMPARE(preLeft, "");
    QCOMPARE(preRight, "C:");
    QCOMPARE(Path3Join("", "C:", "C.mp4"), "C:/C.mp4");

    lastSlashIndex = GetPrepathParts("C.mp4", preLeft, preRight);
    QCOMPARE(QString{"C.mp4"}.mid(lastSlashIndex + 1), "C.mp4");
    QCOMPARE(preLeft, "");
    QCOMPARE(preRight, "");
    QCOMPARE(Path3Join("", "", "C.mp4"), "C.mp4");
  }

  void test_GetPrepathAndFileName() {
    QString prepath, name;
    QCOMPARE(GetPrepathAndFileName("C:/home/to/path", prepath), "path");
    QCOMPARE(prepath, "C:/home/to");
    QCOMPARE(Path2Join("C:/home/to", "path"), "C:/home/to/path");

    QCOMPARE(GetPrepathAndFileName("C:/", prepath), "");
    QCOMPARE(prepath, "C:");
    QCOMPARE(Path2Join("C:", ""), "C:/");

    QCOMPARE(GetPrepathAndFileName("", prepath), "");
    QCOMPARE(prepath, "");
    QCOMPARE(Path2Join("", ""), "");
  }

  void test_join() {
    QCOMPARE(join("C:/", "home/to/path"), "C:/home/to/path");
    QCOMPARE(join("C:", "home/to/path"), "C:/home/to/path");
    QCOMPARE(join("C:", ""), "C:");
    QCOMPARE(join("", "/home/to/path"), "/home/to/path");
  }
};

#include "PathToolTest.moc"
PathToolTest g_pathToolTest;
