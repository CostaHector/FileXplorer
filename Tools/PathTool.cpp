#include "PathTool.h"

QString PATHTOOL::StripTrailingSlash(QString path) {
  // drive letter will be kept while trailing path seperator will be trunc
  // i.e.,
  // "XX:/A/"  -> "XX:/A" and "XX:/" -> same
  // "/home/user/" ->"/home/user" and "/" -> same
#ifdef WIN32
  return (path.size() > 2 and path[path.size() - 2] != ':' and path.back() == PATH_SEP_CHAR) ? path.chopped(1) : path;
#else
  return (path.size() > 1 and path.back() == PATH_SEP_CHAR) ? path.chopped(1) : path;
#endif
}

QString PATHTOOL::linkPath(const QString& localPath) {
  return "file:///" + localPath;
}
QString PATHTOOL::localPath(const QString& linkPath) {
  return linkPath.mid(8);
}
QString PATHTOOL::sysPath(QString fullPath) {
#ifdef _WIN32
  return fullPath.replace('/', '\\');
#else
  return fullPath.replace('\\', '/');
#endif
}
QString PATHTOOL::normPath(QString fullPath) {
  return fullPath.replace('\\', '/');
}
QString PATHTOOL::absolutePath(const QString& fullPath) {
  // "C:/A/" => "C:/" => ""
  // same as "C:/A"
  // "/home/to/" => "/home" => "/" => ""
  // same as "/home/to"
  if (fullPath.isEmpty()) {
    return fullPath;
  }
  QString noSingleTrailingSlash = fullPath;
  if (fullPath.size() > 1 and fullPath.back() == '/') {
    noSingleTrailingSlash.chop(1);
  }
#ifdef WIN32
  int end = noSingleTrailingSlash.lastIndexOf('/');
  return end == -1 ? "" : noSingleTrailingSlash.left(end);
#else
  int end = noSingleTrailingSlash.lastIndexOf('/');
  return end == 0 or end == -1 ? "/" : noSingleTrailingSlash.left(end);

#endif
}
QString PATHTOOL::relativePath(const QString& fullPath, const int rootpathLen) {
  return fullPath.mid(rootpathLen + 1);
}
QString PATHTOOL::forSearchPath(const QString& fullPath) {
  // get QString for database index last three sectors
  int forwardSlashLetterCnt = 0;
  for (int i = fullPath.size() - 1; i > -1; --i) {
    if (fullPath[i] == '/' and ++forwardSlashLetterCnt == 3) {
      return fullPath.mid(i + 1);
    }
  }
  return fullPath;
}
QString PATHTOOL::dirName(const QString& fullPath) {
  int first = 0, end = 0;
  for (int i = fullPath.size() - 1; i > -1; --i) {
    if (fullPath[i] == '/') {
      if (end == 0) {
        end = i;
      } else if (first == 0) {
        first = i;
        break;
      }
    }
  }
  return fullPath.mid(first + 1, end - (first + 1));
}
QString PATHTOOL::fileName(const QString& fullPath) {
  int forwardSlashIndex = fullPath.lastIndexOf('/');
  return forwardSlashIndex == -1 ? fullPath : fullPath.mid(forwardSlashIndex + 1);
}
QString PATHTOOL::join(const QString& prefix, const QString& relative) {
  if (prefix.isEmpty()) {
    return relative;
  }
  if (relative.isEmpty()) {
    return prefix;
  }
  if (not prefix.isEmpty() and prefix.back() == '/') {
    // C:/ a
    return prefix + relative;
  }
  return prefix + '/' + relative;
}
QString PATHTOOL::driver(const QString& fullPath) {
#ifdef _WIN32
  int colonIndex = fullPath.indexOf(':');
  return colonIndex != -1 ? fullPath.left(colonIndex) : "";
#else
  return "";
#endif
}
QString PATHTOOL::commonPrefix(const QString& path1, const QString& path2) {
  return "";
}
QString PATHTOOL::commonPath(const QString& path1, const QString& path2) {
  return "";
}

bool PATHTOOL::isRootOrEmpty(const QString& path) {
  return path.isEmpty() or path == "/";
}
