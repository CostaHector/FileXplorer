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

QString StripTrailingSlash(QString path);
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
QString RelativePath2File(int rootPathLen, const QString& fullPath, int fileNameLen = -1);

// Get "baseName, extension with prefix dot" from fullpath
// a.txt => ("a", ".txt")
std::pair<QString, QString> GetBaseNameExt(const QString& fullpath);
QString GetBaseName(const QString& fullpath);
QString GetFileNameExtRemoved(const QString& fileName);
QString GetFileNameExtRemoved(QString&& fileName);
QString FileExtReplacedWithJson(QString fileName);
QString GetPrepathAndFileName(const QString& fullpath, QString& prepath);

QString Path2Join(const QString& a, const QString& b);
QString Path3Join(const QString& a, const QString& b, const QString& c);
int GetPrepathParts(const QString& absPath, QString& outPrePathLeft, QString& outPrePathRight);

QString join(const QString& prefix, const QString& relative);
QString driver(const QString& fullPath);
QString StrCommonPrefix(const QString& path1, const QString& path2);
bool isLinuxRootOrWinEmpty(const QString& path);  // loose
bool isRootOrEmpty(const QString& path);          // strict

QString longestCommonPrefix(const QStringList& strs);

QStringList GetRels(int prefixLen, const QStringList& lAbsPathList);
std::pair<QString, QStringList> GetLAndRels(const QStringList& lAbsPathList);

QString GetFileExtension(const QString& path);
bool copyDirectoryFiles(const QString& fromDir, const QString& toDir, bool coverFileIfExist = false);

}  // namespace PathTool
#endif  // PATHTOOL_H
