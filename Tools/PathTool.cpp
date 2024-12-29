#include "PathTool.h"

#include <QDir>
#include <QFileInfo>
#include <QDirIterator>

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

QString PATHTOOL::RelativePath2File(int rootPathLen, const QString& fullPath, int fileNameLen) {
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

std::pair<QString, QString> PATHTOOL::GetBaseNameExt(const QString& fullpath) {
  const int lastIndexOfSlash = fullpath.lastIndexOf(PATH_SEP_CHAR);
  const int lastIndexOfExtDot = fullpath.lastIndexOf('.');
  // like "C:/.a/b"
  // like "C:/a/any.no_extension"
  // like "C:/a/any file"
  // index(dot) + 5 <= size()
  if (lastIndexOfExtDot <= lastIndexOfSlash || lastIndexOfExtDot == -1 || lastIndexOfExtDot + EXTENSION_MAX_LENGTH < fullpath.size()) {
    return std::make_pair(fullpath.mid(lastIndexOfSlash + 1), "");
  }
  return std::make_pair(fullpath.mid(lastIndexOfSlash + 1, lastIndexOfExtDot - lastIndexOfSlash - 1), fullpath.mid(lastIndexOfExtDot));
}

QString PATHTOOL::GetBaseName(const QString& fullpath) {
  const int lastIndexOfSlash = fullpath.lastIndexOf(PATH_SEP_CHAR);
  const int lastIndexOfExtDot = fullpath.lastIndexOf('.');
  if (lastIndexOfExtDot <= lastIndexOfSlash || lastIndexOfExtDot == -1 || lastIndexOfExtDot + EXTENSION_MAX_LENGTH < fullpath.size()) {
    return fullpath.mid(lastIndexOfSlash + 1);
  }
  return fullpath.mid(lastIndexOfSlash + 1, lastIndexOfExtDot - lastIndexOfSlash - 1);
}

QString PATHTOOL::GetFileNameExtRemoved(const QString& fileName) {
  const int lastIndexOfExtDot = fileName.lastIndexOf('.');
  if (lastIndexOfExtDot == -1 || lastIndexOfExtDot + EXTENSION_MAX_LENGTH < fileName.size()) {
    return fileName;
  }
  return fileName.left(lastIndexOfExtDot);
}

QString PATHTOOL::GetFileNameExtRemoved(QString&& fileName) {
  const int lastIndexOfExtDot = fileName.lastIndexOf('.');
  if (lastIndexOfExtDot == -1 || lastIndexOfExtDot + EXTENSION_MAX_LENGTH < fileName.size()) {
    return fileName;
  }
  return fileName.left(lastIndexOfExtDot);
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

bool PATHTOOL::isLinuxRootOrWinEmpty(const QString& path) {
  return path.isEmpty() or path == "/";
}

bool PATHTOOL::isRootOrEmpty(const QString& path) {
  return path.isEmpty() or path == "/" or QDir(path).isRoot();
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
}

int PATHTOOL::getFileExtensionDotIndex(const QString& path) {
  const int lastDot = path.lastIndexOf('.');
  const int N = path.size();
  if (lastDot == -1) {
    return N;
  }
  if (N >= 2 and lastDot == N - 2 and path[N - 1].isDigit()) {
    return N;
  }
  if (N >= 3 and lastDot == N - 3 and path[N - 1].isDigit() and path[N - 2].isDigit()) {
    return N;
  }
  return N - (lastDot + 1) > EXTENSION_MAX_LENGTH ? N : lastDot;
}

QString PATHTOOL::GetFileExtension(const QString& path) {
  return path.mid(getFileExtensionDotIndex(path));
}

OSWalker_RETURN PATHTOOL::OSWalker(const QString& pre, const QStringList& rels, const bool includingSub, const bool includingSuffix) {
  // Reverse the return value, One can get bottom To Top result like os.walk
  const auto dirIterFlag {includingSub ? QDirIterator::IteratorFlag::Subdirectories : QDirIterator::IteratorFlag::NoIteratorFlags};
  const int n1 = pre.size() + 1;
  QDir preDir(pre);

  QStringList relToNames;
  QStringList completeNames;
  QStringList suffixs;
  QList<bool> isFiles;

  for (const QString& rel : rels) {
    QFileInfo fileInfo(preDir.absoluteFilePath(rel));
    isFiles.append(fileInfo.isFile());
    relToNames.append(fileInfo.absolutePath().mid(n1));
    if (includingSuffix) {
      completeNames.append(fileInfo.fileName());
      suffixs.append("");
    } else {
      const QString& nm = fileInfo.fileName();
      const int dotIndex = getFileExtensionDotIndex(nm);
      completeNames.append(nm.left(dotIndex));
      suffixs.append(nm.mid(dotIndex));
    }

    if (includingSub and fileInfo.isDir()) {  // folders
      QDirIterator it(fileInfo.absoluteFilePath(), {}, QDir::Filter::NoDotAndDotDot | QDir::Filter::AllEntries, dirIterFlag);
      while (it.hasNext()) {
        it.next();
        auto fi = it.fileInfo();
        isFiles.append(fi.isFile());
        relToNames.append(fi.absolutePath().mid(n1));
        if (includingSuffix) {
          completeNames.append(fi.fileName());
          suffixs.append("");
        } else {
          const QString& nm = fi.fileName();
          const int dotIndex = getFileExtensionDotIndex(nm);
          completeNames.append(nm.left(dotIndex));
          suffixs.append(nm.mid(dotIndex));
        }
      }
    }
  }
  return {relToNames, completeNames, suffixs, isFiles};
}

bool PATHTOOL::copyDirectoryFiles(const QString& fromDir, const QString& toDir, bool coverFileIfExist) {
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
