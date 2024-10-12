#ifndef PATHTOOL_H
#define PATHTOOL_H

#include <QString>

namespace PATHTOOL {
inline QString GetWinStdPath(const QString& path) {
#ifdef _WIN32
  if (not path.isEmpty() and path.back() == ':') {
    return path + '/';
  }
#endif
return path;
}
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

QString join(const QString& prefix, const QString& relative);
QString driver(const QString& fullPath);
QString commonPrefix(const QString& path1, const QString& path2);
bool isLinuxRootOrWinEmpty(const QString& path);  // loose
bool isRootOrEmpty(const QString& path);          // strict

QString longestCommonPrefix(const QStringList& strs);

QStringList GetRels(int prefixLen, const QStringList& lAbsPathList);
std::pair<QString, QStringList> GetLAndRels(const QStringList& lAbsPathList);

constexpr char PATH_SEP_CHAR = '/';
}  // namespace PATHTOOL

#endif  // PATHTOOL_H
