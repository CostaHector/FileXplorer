#include "PathTool.h"

#include <QDir>
#include <QFileInfo>
#include <QDirIterator>

QString PathTool::StripTrailingSlash(QString path) {
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

QString PathTool::linkPath(const QString& localPath) {
  return "file:///" + localPath;
}
QString PathTool::localPath(const QString& linkPath) {
  return linkPath.mid(8);
}
QString PathTool::sysPath(QString fullPath) {
#ifdef _WIN32
  return fullPath.replace('/', '\\');
#else
  return fullPath.replace('\\', '/');
#endif
}
QString PathTool::normPath(QString fullPath) {
  return fullPath.replace('\\', '/');
}
QString PathTool::absolutePath(const QString& fullPath) {
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
QString PathTool::relativePath(const QString& fullPath, const int rootpathLen) {
  return fullPath.mid(rootpathLen + 1);
}
QString PathTool::forSearchPath(const QString& fullPath) {
  // get QString for database index last three sectors
  int forwardSlashLetterCnt = 0;
  for (int i = fullPath.size() - 1; i > -1; --i) {
    if (fullPath[i] == '/' and ++forwardSlashLetterCnt == 3) {
      return fullPath.mid(i + 1);
    }
  }
  return fullPath;
}
QString PathTool::dirName(const QString& fullPath) {
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
QString PathTool::fileName(const QString& fullPath) {
  int forwardSlashIndex = fullPath.lastIndexOf('/');
  return forwardSlashIndex == -1 ? fullPath : fullPath.mid(forwardSlashIndex + 1);
}

QString PathTool::RelativePath2File(int rootPathLen, const QString& fullPath, int fileNameLen) {
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

std::pair<QString, QString> PathTool::GetBaseNameExt(const QString& fullpath) {
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

QString PathTool::GetBaseName(const QString& fullpath) {
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

QString PathTool::GetFileNameExtRemoved(const QString& fileName) {
  const int lastIndexOfExtDot = fileName.lastIndexOf('.');
  if (lastIndexOfExtDot == -1 || lastIndexOfExtDot + EXTENSION_MAX_LENGTH < fileName.size()) {
    return fileName;
  }
  return fileName.left(lastIndexOfExtDot);
}

QString PathTool::GetFileNameExtRemoved(QString&& fileName) {
  const int lastIndexOfExtDot = fileName.lastIndexOf('.');
  if (lastIndexOfExtDot == -1 || lastIndexOfExtDot + EXTENSION_MAX_LENGTH < fileName.size()) {
    return fileName;
  }
  return fileName.left(lastIndexOfExtDot);
}

QString PathTool::FileExtReplacedWithJson(QString fileName) {
  const int lastIndexOfExtDot = fileName.lastIndexOf('.');
  if (lastIndexOfExtDot == -1 || lastIndexOfExtDot + EXTENSION_MAX_LENGTH < fileName.size()) {
    fileName += ".json";
    return fileName;
  }
  static constexpr int JSON_EXT_SIZE = 4;
  return fileName.replace(lastIndexOfExtDot + 1, JSON_EXT_SIZE, "json");
}

QString PathTool::GetPrepathAndFileName(const QString& fullpath, QString& prepath) {
  const int lastIndexOfSlash = fullpath.lastIndexOf(PATH_SEP_CHAR);
  prepath = fullpath.left(lastIndexOfSlash);
  return fullpath.mid(lastIndexOfSlash + 1);
}

QString PathTool::Path2Join(const QString& a, const QString& b) {
  QString ans;
  ans.reserve(a.size() + 1 + b.size());
  if (!a.isEmpty()) {
    ans += a;
    ans += '/';
  }
  return ans += b;
}

QString PathTool::Path3Join(const QString& a, const QString& b, const QString& c) {
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

int PathTool::GetPrepathParts(const QString& absPath, QString& outPrePathLeft, QString& outPrePathRight) {
  outPrePathLeft.clear();
  outPrePathRight.clear();
  const int lastHashIndex = absPath.lastIndexOf('/');  // 找到最后一个/的位置
  if (lastHashIndex == -1) {                           // C.mp4
    return lastHashIndex;
  }
  const int secondLastHashIndex = absPath.lastIndexOf('/', lastHashIndex - 1);  // 找到倒数第二个/的位置
  if (secondLastHashIndex == -1) {                                              // C:/C.mp4
    outPrePathRight = absPath.left(lastHashIndex);                              // C:
    return lastHashIndex;
  }
  const int thirdLastHashIndex = absPath.lastIndexOf('/', secondLastHashIndex - 1);  // 找到倒数第三个/的位置
  if (thirdLastHashIndex == -1) {                                                    // C:/A/B.mp4
    outPrePathRight = absPath.left(lastHashIndex);                                   // C:/A
    return lastHashIndex;
  }
  // C:/A/B/C.mp4
  outPrePathLeft = absPath.left(thirdLastHashIndex);                                              // C:
  outPrePathRight = absPath.mid(thirdLastHashIndex + 1, lastHashIndex - thirdLastHashIndex - 1);  // A/B
  return lastHashIndex;
}

QString PathTool::join(const QString& prefix, const QString& relative) {
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
QString PathTool::driver(const QString& fullPath) {
#ifdef _WIN32
  int colonIndex = fullPath.indexOf(':');
  return colonIndex != -1 ? fullPath.left(colonIndex) : "";
#else
  return "";
#endif
}
QString PathTool::StrCommonPrefix(const QString& path1, const QString& path2) {
  const int length = std::min(path1.size(), path2.size());
  int index = 0;
  while (index < length && path1[index] == path2[index]) {
    ++index;
  }
  return path1.left(index);
}

bool PathTool::isLinuxRootOrWinEmpty(const QString& path) {
  return path.isEmpty() || path == "/";
}

bool PathTool::isRootOrEmpty(const QString& path) {
  return path.isEmpty() || path == "/" || QDir(path).isRoot();
}

QStringList PathTool::GetRels(int prefixLen, const QStringList& lAbsPathList) {
  // "/home/rel2entry", "/home", prefixLen = 4
  const int rel2EntryN = prefixLen + 1;
  QStringList lRels;
  for (const auto& lAbsPath : lAbsPathList) {
    lRels.append(lAbsPath.mid(rel2EntryN));
  }
  return lRels;
}

std::pair<QString, QStringList> PathTool::GetLAndRels(const QStringList& lAbsPathList) {
  if (lAbsPathList.isEmpty()) {
    return {"", lAbsPathList};
  }
  const QString& prefixPath = longestCommonPrefix(lAbsPathList);
  // "/home/rel2entry" => rel2EntryN = 5+1
  const int prefixLen = prefixPath.size();
  QStringList lRels = GetRels(prefixLen, lAbsPathList);
  return {prefixPath, lRels};
}

QString PathTool::longestCommonPrefix(const QStringList& strs) {
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
QString PathTool::GetFileExtension(const QString& path) {
  const int lastIndexOfDot = path.lastIndexOf('.');
  if (lastIndexOfDot == -1) {
    return {};
  }
  return path.mid(lastIndexOfDot);
}

bool PathTool::copyDirectoryFiles(const QString& fromDir, const QString& toDir, bool coverFileIfExist) {
  QDir sourceDir(fromDir);
  QDir targetDir(toDir);
  if (!targetDir.exists()) { /* if directory don't exists, build it */
    if (!targetDir.mkdir(targetDir.absolutePath()))
      return false;
  }

  QFileInfoList fileInfoList = sourceDir.entryInfoList();
  for (const QFileInfo& fileInfo : fileInfoList) {
    if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
      continue;

    if (fileInfo.isDir()) { /* if it is directory, copy recursively*/
      if (copyDirectoryFiles(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()), coverFileIfExist)) {
        continue;
      }
      return false;
    }
    /* if coverFileIfExist == true, remove old file first */

    if (targetDir.exists(fileInfo.fileName())) {
      if (coverFileIfExist) {
        targetDir.remove(fileInfo.fileName());
        qDebug("%s/%s is covered by file under [%s]", qPrintable(targetDir.absolutePath()), qPrintable(fileInfo.fileName()), qPrintable(fromDir));
      } else {
        qDebug("%s/[%s] was kept", qPrintable(targetDir.absolutePath()), qPrintable(fileInfo.fileName()));
      }
    }
    // files copy
    if (!QFile::copy(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))) {
      return false;
    }
  }
  return true;
}
