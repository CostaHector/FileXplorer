#include "PathTool.h"

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
  int end = fullPath.lastIndexOf('/');
  return end != -1 ? fullPath.left(end) : fullPath;
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
