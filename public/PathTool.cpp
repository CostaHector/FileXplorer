#include "PathTool.h"
#include "Logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QSet>
namespace PathTool{

QString GetPathByApplicationDirPath(const QString& relativePath) {
  static const QDir appDir{QCoreApplication::applicationDirPath()};
  // QFileInfo{"."}.absoluteFilePath() may be same as appDir.
  // "." can be modified by QDir::setCurrent("newpwd"),
  // while appDir is not influenced by setCurrent. it reamins exe file path.
  return QDir::cleanPath(appDir.absoluteFilePath(relativePath));
}

QString linkPath(const QString& localPath) {
#ifdef _WIN32
  return "file:///" + localPath;  // file:///C:/to/path
#else
  return "file://" + localPath;  // file:///home/to/path
#endif
}

QString localPath(const QString& linkPath) {
#ifdef _WIN32
  return linkPath.mid(8);
#else
  return linkPath.mid(7);
#endif
}

QString sysPath(QString fullPath) {
#ifdef _WIN32
  return fullPath.replace('/', '\\');
#else
  return fullPath.replace('\\', '/');
#endif
}
QString normPath(QString fullPath) {
  return fullPath.replace('\\', '/');
}
QString absolutePath(const QString& fullPath) {
  // "C:/A/" => "C:/" => ""
  // same as "C:/A"
  // "/home/to/" => "/home" => "/" => ""
  // same as "/home/to"
  if (fullPath.isEmpty()) {
    return fullPath;
  }
  QString noSingleTrailingSlash = fullPath;
  if (fullPath.size() > 1 && fullPath.back() == '/') {
    noSingleTrailingSlash.chop(1);
  }
#ifdef _WIN32
  int end = noSingleTrailingSlash.lastIndexOf('/');
  return end == -1 ? "" : noSingleTrailingSlash.left(end);
#else
  int end = noSingleTrailingSlash.lastIndexOf('/');
  return end == 0 or end == -1 ? "/" : noSingleTrailingSlash.left(end);

#endif
}
QString relativePath(const QString& fullPath, const int rootpathLen) {
  return fullPath.mid(rootpathLen + 1);
}
QString forSearchPath(const QString& fullPath) {
  // get QString for database index last three sectors
  int forwardSlashLetterCnt = 0;
  for (int i = fullPath.size() - 1; i > -1; --i) {
    if (fullPath[i] == '/' && ++forwardSlashLetterCnt == 3) {
      return fullPath.mid(i + 1);
    }
  }
  return fullPath;
}
QString dirName(const QString& fullPath) {
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
QString fileName(const QString& fullPath) {
  int forwardSlashIndex = fullPath.lastIndexOf('/');
  return forwardSlashIndex == -1 ? fullPath : fullPath.mid(forwardSlashIndex + 1);
}

QString RelativePath2File(int rootPathLen, const QString& fullPath, int fileNameLen) {
  if (fileNameLen > 0) {
    return fullPath.mid(rootPathLen, fullPath.size() - fileNameLen - rootPathLen);
  }

  int lastSlashIndex = fullPath.lastIndexOf('/');
  if (lastSlashIndex == -1) {
    return {};
  }
  if (lastSlashIndex - rootPathLen + 1 < 1) {
    return {};
  }
  return fullPath.mid(rootPathLen, lastSlashIndex - rootPathLen + 1);
}

bool HasLetter(const QString& fullpath, int startIdx) {
  for (int i = startIdx; i < fullpath.size(); ++i) {
    if (fullpath[i].isLetter()) {
      return true;
    }
  }
  return false;
}

BASE_NAME_2_DOT_EXT GetBaseNameExt(const QString& fullpath) {
  const int lastIndexOfSlash = fullpath.lastIndexOf(PATH_SEP_CHAR);
  const int lastIndexOfExtDot = fullpath.lastIndexOf('.');
  if (lastIndexOfExtDot <= lastIndexOfSlash                          // Kris./nice shoes
      || lastIndexOfExtDot == -1                                     // hello world
      || lastIndexOfExtDot + EXTENSION_MAX_LENGTH < fullpath.size()  // ok. say when
      || !HasLetter(fullpath, lastIndexOfExtDot + 1))                // sc. 01
  {
    return std::make_pair(fullpath.mid(lastIndexOfSlash + 1),  // base
                          "");                                 // extension
  }
  return std::make_pair(fullpath.mid(lastIndexOfSlash + 1, lastIndexOfExtDot - lastIndexOfSlash - 1),  // base
                        fullpath.mid(lastIndexOfExtDot));                                              // extension
}

QString GetBaseName(const QString& fullpath) {
  const int lastIndexOfSlash = fullpath.lastIndexOf(PATH_SEP_CHAR);
  const int lastIndexOfExtDot = fullpath.lastIndexOf('.');
  if (lastIndexOfExtDot <= lastIndexOfSlash                          // Kris./nice shoes
      || lastIndexOfExtDot == -1                                     // hello world
      || lastIndexOfExtDot + EXTENSION_MAX_LENGTH < fullpath.size()  // ok. say when
      || !HasLetter(fullpath, lastIndexOfExtDot + 1))                // sc. 01
  {
    return fullpath.mid(lastIndexOfSlash + 1);
  }

  return fullpath.mid(lastIndexOfSlash + 1, lastIndexOfExtDot - lastIndexOfSlash - 1);
}

QString GetFileNameExtRemoved(const QString& fileName) {
  const int lastIndexOfExtDot = fileName.lastIndexOf('.');
  if (lastIndexOfExtDot == -1 || lastIndexOfExtDot + EXTENSION_MAX_LENGTH < fileName.size()) {
    return fileName;
  }
  return fileName.left(lastIndexOfExtDot);
}

QString GetFileNameExtRemoved(QString&& fileName) {
  const int lastIndexOfExtDot = fileName.lastIndexOf('.');
  if (lastIndexOfExtDot == -1 || lastIndexOfExtDot + EXTENSION_MAX_LENGTH < fileName.size()) {
    return fileName;
  }
  return fileName.left(lastIndexOfExtDot);
}

QString FileExtReplacedWithJson(QString fileName) {
  const int lastIndexOfExtDot = fileName.lastIndexOf('.');
  if (lastIndexOfExtDot == -1 || lastIndexOfExtDot + EXTENSION_MAX_LENGTH < fileName.size()) {
    fileName += ".json";
    return fileName;
  }
  static constexpr int JSON_EXT_SIZE = 4;
  return fileName.replace(lastIndexOfExtDot + 1, JSON_EXT_SIZE, "json");
}

QString GetPrepathAndFileName(const QString& fullpath, QString& prepath) {
  const int lastIndexOfSlash = fullpath.lastIndexOf(PATH_SEP_CHAR);
  prepath = fullpath.left(lastIndexOfSlash);
  return fullpath.mid(lastIndexOfSlash + 1);
}

QString Path2Join(const QString& a, const QString& b) {
  QString ans;
  ans.reserve(a.size() + 1 + b.size());
  if (!a.isEmpty()) {
    ans += a;
    ans += '/';
  }
  return ans += b;
}

QString Path3Join(const QString& a, const QString& b, const QString& c) {
  QString ans;
  ans.reserve(a.size() + 1 + b.size() + 1 + c.size());
  if (!a.isEmpty()) {
    ans += a;
    ans += '/';
  }
  if (!b.isEmpty()) {
    ans += b;
    ans += '/';
  }
  return ans += c;
}

int GetPrepathParts(const QString& absPath, QString& outPrePathLeft, QString& outPrePathRight) {
  outPrePathLeft.clear();
  outPrePathRight.clear();
  const int lastSlashIndex = absPath.lastIndexOf('/');  // 找到最后一个/的位置
  if (lastSlashIndex == -1) {                           // C.mp4
    return lastSlashIndex;
  }
  const int secondlastSlashIndex = absPath.lastIndexOf('/', lastSlashIndex - 1);  // 找到倒数第二个/的位置
  if (secondlastSlashIndex == -1) {                                               // C:/C.mp4
    outPrePathRight = absPath.left(lastSlashIndex);                               // C:
    return lastSlashIndex;
  }
  const int thirdlastSlashIndex = absPath.lastIndexOf('/', secondlastSlashIndex - 1);  // 找到倒数第三个/的位置
  if (thirdlastSlashIndex == -1) {                                                     // C:/A/B.mp4
    outPrePathRight = absPath.left(lastSlashIndex);                                    // C:/A
    return lastSlashIndex;
  }
  // C:/A/B/C.mp4
  outPrePathLeft = absPath.left(thirdlastSlashIndex);                                                // C:
  outPrePathRight = absPath.mid(thirdlastSlashIndex + 1, lastSlashIndex - thirdlastSlashIndex - 1);  // A/B
  return lastSlashIndex;
}

// get last 3 part of a file abs path
QString GetEffectiveName(const QString& itemPath) {
  // input string => output string, for example:
  // C:/A/B/C.ext => B/C.ext
  // C:/A/Videos/C.ext => A/Videos/C.ext
  // C:/A/Video/C.ext => A/Video/C.ext
  // C:/A/Vid/C.ext => A/Vid/C.ext
  // C:/A/VIDEO_TS/C.ext => A/VIDEO_TS/C.ext
  static const QSet<QString> specialSegments{"videos", "video", "vid", "video_ts"};
  const QString segment = itemPath.section('/', -2, -2).toLower();
  if (specialSegments.contains(segment)) {
    return itemPath.section('/', -3, -1);
  }
  return itemPath.section('/', -2, -1);
}

QString join(const QString& prefix, const QString& relative) {
  if (prefix.isEmpty()) {
    return relative;
  }
  if (relative.isEmpty()) {
    return prefix;
  }
  if (!prefix.isEmpty() && prefix.back() == '/') {
    // C:/ a
    return prefix + relative;
  }
  return prefix + '/' + relative;
}
QString driver(const QString& fullPath) {
#ifdef _WIN32
  int colonIndex = fullPath.indexOf(':');
  return colonIndex != -1 ? fullPath.left(colonIndex) : "";
#else
  return "";
#endif
}
QString StrCommonPrefix(const QString& path1, const QString& path2) {
  const int length = std::min(path1.size(), path2.size());
  int index = 0;
  while (index < length && path1[index] == path2[index]) {
    ++index;
  }
  return path1.left(index);
}

bool isRootOrEmpty(const QString& path) {
  return path.isEmpty() || path == "/" || QDir(path).isRoot();
}

QStringList GetRels(int prefixLen, const QStringList& lAbsPathList) {
  // "/home/rel2entry", "/home", prefixLen = 4
  const int rel2EntryN = prefixLen + 1;
  QStringList lRels;
  for (const auto& lAbsPath : lAbsPathList) {
    lRels.append(lAbsPath.mid(rel2EntryN));
  }
  return lRels;
}

std::pair<QString, QStringList> GetLAndRels(const QStringList& lAbsPathList) {
  if (lAbsPathList.isEmpty()) {
    return {"", lAbsPathList};
  }
  const QString& prefixPath = longestCommonPrefix(lAbsPathList);
  // "/home/rel2entry" => rel2EntryN = 5+1
  const int prefixLen = prefixPath.size();
  QStringList lRels = GetRels(prefixLen, lAbsPathList);
  return {prefixPath, lRels};
}

QString longestCommonPrefix(const QStringList& strs) {
  if (strs.isEmpty()) {
    return "";
  }
  if (strs.size() == 1) {
    return absolutePath(strs[0]);
  }

  QString prefix = strs[0];
  int count = strs.size();
  for (int i = 1; i < count; ++i) {
    prefix = StrCommonPrefix(prefix, strs[i]);
    if (!prefix.size()) {
      break;
    }
  }

  // source         => string common prefix   => path common prefix
  // /home/costa    => /home/costa            => /home
  // /home/costa/H  => /home/costa

  // return path should without trailing '/'
  const int slashIndex = prefix.lastIndexOf('/');
  return slashIndex == -1 ? prefix : prefix.left(slashIndex);
}

// contains dot itself
QString GetFileExtension(const QString& path) {
  const int lastIndexOfDot = path.lastIndexOf('.');
  if (lastIndexOfDot == -1) {
    return {};
  }
  return path.mid(lastIndexOfDot);
}

}
