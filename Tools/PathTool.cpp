#include "PathTool.h"

#include <QFileInfo>
#include <QStringList>

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
  const int length = std::min(path1.size(), path2.size());
  int index = 0;
  while (index < length && path1[index] == path2[index]) {
    ++index;
  }
  return path1.left(index);
}

bool PATHTOOL::isRootOrEmpty(const QString& path) {
  return path.isEmpty() or path == "/";
}

QStringList PATHTOOL::GetRels(int prefixLen, const QStringList& lAbsPathList) {
  // "/home/rel2entry", "/home", prefixLen = 4
  const int rel2EntryN = prefixLen + 1;
  QStringList lRels;
  for (const auto& lAbsPath : lAbsPathList) {
    lRels.append(lAbsPath.mid(rel2EntryN));
  }
  return lRels;
}

std::pair<QString, QStringList> PATHTOOL::GetLAndRels(const QStringList& lAbsPathList) {
  if (lAbsPathList.isEmpty()) {
    return {"", lAbsPathList};
  }
  const auto& prefixPath = longestCommonPrefix(lAbsPathList);
  // "/home/rel2entry" => rel2EntryN = 5+1
  const int prefixLen = prefixPath.size();
  QStringList lRels = GetRels(prefixLen, lAbsPathList);
  return {prefixPath, lRels};
}

QString PATHTOOL::longestCommonPrefix(const QStringList& strs) {
  if (strs.isEmpty()) {
    return "";
  }
  if (strs.size() == 1) {
    return QFileInfo(strs[0]).absolutePath();
  }

  QString prefix = strs[0];
  int count = strs.size();
  for (int i = 1; i < count; ++i) {
    prefix = commonPrefix(prefix, strs[i]);
    if (!prefix.size()) {
      break;
    }
  }

  // source         => plain prefix   => path prefix
  // /home/costa    => /home/costa    => /home
  // /home/costa/H  => /home/costa

  // return path without trailing '/'

  const int slashIndex = prefix.lastIndexOf('/');
  return slashIndex == -1 ? prefix : prefix.left(slashIndex);
  ;
}
