#ifndef PATHTOOL_H
#define PATHTOOL_H

#include <QString>
#include <QStringList>

namespace PathTool {
struct SelectionInfo {
  // 1. move(keep file system structure)
  /*
C:/home/to/path     		file				      toPath	file
C:/home/to/path     		folder				    toPath	folder
C:/home/to/path     		folder1/file11		toPath	folder1/file11
C:/home/to/path     		folder2/folder21	toPath	folder2/folder21
C:/home/to/path     		folder1				     ------------------ignore
   */
  QString rootPath;
  QStringList relSelections;
  // 2. move(flatten file system structure)
  // 3. only rename file name(not move)
  /*
C:/home/to/path           file                toPath	file
C:/home/to/path           folder              toPath	folder
C:/home/to/path/folder1		file11              toPath	file11
C:/home/to/path/folder2		folder21            toPath	folder21
C:/home/to/path           folder1				------------------ignore
   */
  QString rootPaths;
  QStringList selections;
};

struct RMFComponent {
  /* Used in movie table `PrePathLeft`, `PrePathRight`, `Name` field
   * Root+Middle+fileName                       <=> AbsFilePath
   * ["",     "",                      "C.mp4"] <=> C.mp4
   * ["",     "C:/",                   "C.mp4"] <=> C:/C.mp4
   * ["",     "C:/A/",                 "B.mp4"] <=> C:/A/B.mp4
   * ["C:/",  "A/B/",                  "C.mp4"] <=> C:/A/B/C.mp4
   * ["/",    "tmp/FileXplorer-xxxx/", "a.mp4"] <=> /tmp/FileXplorer-xxxx/a.mp4
   */
  QString rootPart;
  QString middlePart;
  QString fileName;

  QString joinItself() const {
    return join(rootPart, middlePart, fileName);
  }
  QString joinParentPathItself() const {
    return joinParentPath(rootPart, middlePart);
  }
  static QString join(QString root, const QString& middle, const QString& file) {
    root.reserve(root.size() + middle.size() + file.size());
    root += middle;
    root += file;
    return root;
  }
  static QString joinParentPath(QString root, const QString& middle) {
    root += middle;
    int n = root.size();
    if (n > 2 && root[n-1] == '/' && root[n-2] != ':') {
      root.chop(1);
    }
    return root;
  }
  static QString stdStyleSubstring(const QString& str, int start, int end) { return str.mid(start, end - start); }
  static RMFComponent FromPath(const QString& input);
};

namespace FILE_REL_PATH{
constexpr char MEDIA_INFO_DLL[] {"../../lib/MediaInfo.dll"};
constexpr char PERFORMERS_TABLE[]{"../../../CastStudioList/PERFORMERS_TABLE.txt"};
constexpr char AKA_PERFORMERS[]{"../../../CastStudioList/AKA_PERFORMERS.txt"};
constexpr char STANDARD_STUDIO_NAME[]{"../../../CastStudioList/STANDARD_STUDIO_NAME.txt"};
}

constexpr char PATH_SEP_CHAR = '/';
constexpr int EXTENSION_MAX_LENGTH = 5;  // ".json"

inline QString GetWinStdPath(const QString& path) {
#ifdef _WIN32
  if (!path.isEmpty() and path.back() == ':') {
    return path + '/';
  }
#endif
  return path;
}
QString GetPathByApplicationDirPath(const QString& relativePath);

QString linkPath(const QString& localPath);
QString localPath(const QString& linkPath);
QString sysPath(QString fullPath);
QString normPath(QString fullPath);
QString absolutePath(const QString& fullPath);
QString relativePath(const QString& fullPath, const int rootpathLen = 0);
QString forSearchPath(const QString& fullPath);  // get QString for database index last three sectors

// rootPath/Any/Relative/Path/File = > Path
// rootPath/Relative/File = > Relative
// dirName is located by last two slash
QString dirName(const QString& fullPath);

// rootPath/Relative/File = > File, just like QFileInfo("rootPath/Relative/File").fileName()
// fileName is located by the last slash
QString fileName(const QString& fullPath);

// "rootPath", rootPath/Any/Relative/Path/File = > /Any/Relative/Path/
// "rootPath", rootPath/Relative/File = > /Relative/
// "rootPath", rootPath/File = > /
QString GetRelPathFromRootRelName(int rootPathLen, const QString& fullPath, int fileNameLen = -1);
inline QString GetAbsFilePathFromRootRelName(const QString& root, const QString& rel, const QString& name) {
  QString ans;
  ans.reserve(root.size() + rel.size() + name.size());
  ans += root;
  ans += rel;
  ans += name;
  return ans;
}

// Get "baseName, extension with prefix dot" from fullpath
// a.txt => ("a", ".txt")
using BASE_NAME_2_DOT_EXT = std::pair<QString, QString>;
BASE_NAME_2_DOT_EXT GetBaseNameExt(const QString& fullpath);
QString GetBaseName(const QString& fullpath);
QString GetFileNameExtRemoved(const QString& fileName);
QString GetFileNameExtRemoved(QString&& fileName);
QString FileExtReplacedWithJson(QString fileName);
QString GetPrepathAndFileName(const QString& fullpath, QString& prepath);

QString Path2Join(const QString& a, const QString& b);
QString GetEffectiveName(const QString& itemPath);

QString join(const QString& prefix, const QString& relative);
QString driver(const QString& fullPath);
QString StrCommonPrefix(const QString& path1, const QString& path2);

inline bool isLinuxRootOrWinEmpty(const QString& path) {
#ifdef _WIN32
  return path.isEmpty();
#else
  return path.isEmpty() || path == "/";
#endif
}

bool isRootOrEmpty(const QString& path);          // strict

QString longestCommonPrefix(const QStringList& strs);

QStringList GetRels(int prefixLen, const QStringList& lAbsPathList);
std::pair<QString, QStringList> GetLAndRels(const QStringList& lAbsPathList);

QString GetFileExtension(const QString& path);

}  // namespace PathTool
#endif  // PATHTOOL_H
