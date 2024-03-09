#ifndef PATHTOOL_H
#define PATHTOOL_H

#include <QString>

namespace PATHTOOL {
QString StripTrailingSlash(QString path);
QString linkPath(const QString& localPath);
QString localPath(const QString& linkPath);
QString sysPath(QString fullPath);
QString normPath(QString fullPath);
QString absolutePath(const QString& fullPath);
QString relativePath(const QString& fullPath, const int rootpathLen = 0);
QString forSearchPath(const QString& fullPath);  // get QString for database index last three sectors
QString dirName(const QString& fullPath);
QString fileName(const QString& fullPath);
QString join(const QString& prefix, const QString& relative);
QString driver(const QString& fullPath);
QString commonPrefix(const QString& path1, const QString& path2);
bool isRootOrEmpty(const QString& path);

QString longestCommonPrefix(const QStringList& strs);

QStringList GetRels(int prefixLen, const QStringList& lAbsPathList);
std::pair<QString, QStringList> GetLAndRels(const QStringList& lAbsPathList);

constexpr char PATH_SEP_CHAR = '/';
}  // namespace PATHTOOL

#endif  // PATHTOOL_H
