#include <QCoreApplication>
#include <QtTest>

#include "PlainTestSuite.h"
#include "PathTool.h"

using namespace ::PathTool;
class PathToolTest : public PlainTestSuite {
  Q_OBJECT

 public:
  PathToolTest() : PlainTestSuite{} {}
 private slots:
  void test_project_name_from_marco() { QCOMPARE(PROJECT_NAME, QString("FileXplorer")); }

  void test_lib_files_path_correct() {
    QString libPath = GetPathByApplicationDirPath(FILE_REL_PATH::MEDIA_INFO_DLL);
    QVERIFY(QFile::exists(libPath));

    QString testRootDir = TESTCASE_ROOT_PATH;
    QString libPath2 = QDir::cleanPath(QDir(testRootDir).absoluteFilePath("../lib/MediaInfo.dll"));
    QCOMPARE(libPath, libPath2);
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
#ifdef _WIN32
    QCOMPARE(GetWinStdPath("C:"), "C:/");
    QCOMPARE(GetWinStdPath("X:"), "X:/");
    QCOMPARE(GetWinStdPath("F1:"), "F1:/");
#else
    QCOMPARE(GetWinStdPath("/"), "/");
    QCOMPARE(GetWinStdPath("/home"), "/home");
#endif
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
    QCOMPARE(GetRelPathFromRootRelName(0, "path with no slash"), "");
    // rootPathLen too large
    QCOMPARE(GetRelPathFromRootRelName(225, "C:/home/Huge Jackman"), "");
  }

  void test_RelativePath2File_ok() {
    QCOMPARE(strlen("C:/home"), 7);
    // "C:", C:/home/file.txt = > /home/
    // "rootPath", rootPath/Any/Relative/Path/File = > /Any/Relative/Path/
    // "rootPath", rootPath/Relative/File = > /Relative/
    // "rootPath", rootPath/File = > /
    QCOMPARE(GetRelPathFromRootRelName(strlen("C:"), "C:/home/file.txt"), "/home/");
    QCOMPARE(GetRelPathFromRootRelName(strlen("rootPath"), "rootPath/Any/Relative/Path/File"), "/Any/Relative/Path/");
    QCOMPARE(GetRelPathFromRootRelName(strlen("rootPath"), "rootPath/Relative/File"), "/Relative/");
    QCOMPARE(GetRelPathFromRootRelName(strlen("rootPath"), "rootPath/File"), "/");

    QCOMPARE(GetRelPathFromRootRelName(strlen("C:"), "C:/home/file.txt", strlen("file.txt")), "/home/");
    QCOMPARE(GetAbsFilePathFromRootRelName("C:", "/home/", "file.txt"), "C:/home/file.txt");

    QCOMPARE(GetRelPathFromRootRelName(strlen("C:"), "C:/home/to/dest", strlen("dest")), "/home/to/");
    QCOMPARE(GetAbsFilePathFromRootRelName("C:", "/home/to/", "dest"), "C:/home/to/dest");
    QCOMPARE(GetRelPathFromRootRelName(strlen("C:"), "C:/dest", strlen("dest")), "/");
    QCOMPARE(GetAbsFilePathFromRootRelName("C:", "/", "dest"), "C:/dest");

    QCOMPARE(GetRelPathFromRootRelName(strlen("rootPath"), "rootPath/Any/Relative/Path/File", 4), "/Any/Relative/Path/");
    QCOMPARE(GetRelPathFromRootRelName(strlen("rootPath"), "rootPath/Relative/File", 4), "/Relative/");
    QCOMPARE(GetRelPathFromRootRelName(strlen("rootPath"), "rootPath/File", 4), "/");
    QCOMPARE(GetRelPathFromRootRelName(2, "C:/projects/src/main.cpp", 8), "/projects/src/");
    QCOMPARE(GetAbsFilePathFromRootRelName("C:", "/projects/src/", "main.cpp"), "C:/projects/src/main.cpp");
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
#ifdef _WIN32
    QVERIFY(isLinuxRootOrWinEmpty(""));
#else
    QVERIFY(isLinuxRootOrWinEmpty("/"));
#endif
  }
  void test_isRootOrEmpty() {
#ifdef _WIN32
    QVERIFY(isRootOrEmpty(""));
    QVERIFY(isRootOrEmpty("C:/"));
    QVERIFY(!isRootOrEmpty("C:/home"));
#else
    QVERIFY(isRootOrEmpty("/"));
    QVERIFY(!isRootOrEmpty("/home"));
#endif
  }

  void test_RMFComponent_and_Decompose() {
    RMFComponent median;

    median = RMFComponent::FromPath("D.mp4");  // 0 slash 无需不考虑, 因为不会有"a.mp4", 这样的绝对路径
    QCOMPARE(median.joinItself(), "D.mp4");
    QCOMPARE(median.joinParentPathItself(), "");

    median = RMFComponent::FromPath("C:/D.mp4");  // 1 slash
    QCOMPARE(median.joinItself(), "C:/D.mp4");
    QCOMPARE(median.joinParentPathItself(), "C:/");

    median = RMFComponent::FromPath("C:/A/C.mp4");  // 2 slash
    QCOMPARE(median.joinItself(), "C:/A/C.mp4");
    QCOMPARE(median.joinParentPathItself(), "C:/A");

    median = RMFComponent::FromPath("C:/long/enough/a.mp4");  // 3 slash
    QCOMPARE(median.joinItself(), "C:/long/enough/a.mp4");
    QCOMPARE(median.joinParentPathItself(), "C:/long/enough");

    median = RMFComponent::FromPath("/a.mp4");  // 1 slash
    QCOMPARE(median.joinItself(), "/a.mp4");
    QCOMPARE(median.joinParentPathItself(), "/");

    median = RMFComponent::FromPath("/tmp/a.mp4");  // 2 slash
    QCOMPARE(median.joinItself(), "/tmp/a.mp4");
    QCOMPARE(median.joinParentPathItself(), "/tmp");

    median = RMFComponent::FromPath("/tmp/FileXplorer-xxxx/a.mp4");
    QCOMPARE(median.joinItself(), "/tmp/FileXplorer-xxxx/a.mp4");
    QCOMPARE(median.joinParentPathItself(), "/tmp/FileXplorer-xxxx");
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

  void test_Basic() {  //
    // not contains special word, last 3 part
    QCOMPARE("123", PathTool::GetEffectiveName("123"));
    QCOMPARE("B/C.ext", PathTool::GetEffectiveName("C:/A/B/C.ext"));
    QCOMPARE("SampleVideos/C.ext", PathTool::GetEffectiveName("C:/A/SampleVideos/C.ext"));

    // contains special word, last 3 part
    QCOMPARE("A/Videos/C.ext", PathTool::GetEffectiveName("C:/A/Videos/C.ext"));
    QCOMPARE("A/Video/C.ext", PathTool::GetEffectiveName("C:/A/Video/C.ext"));
    QCOMPARE("A/Vid/C.ext", PathTool::GetEffectiveName("C:/A/Vid/C.ext"));
    QCOMPARE("A/VIDEO_TS/C.ext", PathTool::GetEffectiveName("C:/A/VIDEO_TS/C.ext"));
    QCOMPARE("A/VID/C.ext", PathTool::GetEffectiveName("C:/A/VID/C.ext"));
    QCOMPARE("A/video_ts/C.ext", PathTool::GetEffectiveName("C:/A/video_ts/C.ext"));
  }

  void fileExtensionBasic_test() {
    QCOMPARE(PathTool::GetDotFileExtension("AAA.mp4"), ".mp4");
    QCOMPARE(PathTool::GetDotFileExtension("AAA.json"), ".json");
    QCOMPARE(PathTool::GetDotFileExtension("AAA.z01"), ".z01");
  }

  void fileExtension_1Char_test() {
    QCOMPARE(PathTool::GetDotFileExtension("AAA.h"), ".h");
    QCOMPARE(PathTool::GetDotFileExtension("AAA.m"), ".m");
  }

  void fileExtension_NoExtension_test() {
    QCOMPARE(PathTool::GetDotFileExtension("AAA.5"), ".5");
    QCOMPARE(PathTool::GetDotFileExtension("AAA.51"), ".51");
  }

  void absolutePath_ok() {
    QCOMPARE(PathTool::absolutePath("C:/home/"), "C:/");
    QCOMPARE(PathTool::absolutePath("C:/home"), "C:/");
    QCOMPARE(PathTool::absolutePath("C:/"), "");
    QCOMPARE(PathTool::absolutePath("D:/home/"), "D:/");
    QCOMPARE(PathTool::absolutePath("D:/home"), "D:/");
    QCOMPARE(PathTool::absolutePath("D:/"), "");
    QCOMPARE(PathTool::absolutePath("/home/to/"), "/home");
    QCOMPARE(PathTool::absolutePath("/home/to"), "/home");
    QCOMPARE(PathTool::absolutePath("/home/"), "/");
    QCOMPARE(PathTool::absolutePath("/home"), "/");
    QCOMPARE(PathTool::absolutePath("/"), "");
  }
};

#include "PathToolTest.moc"
REGISTER_TEST(PathToolTest, false)
