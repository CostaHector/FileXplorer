#include "TDir.h"
#include "public/PathTool.h"
#include <QTextStream>
#include <QDirIterator>

bool FsNodeEntry::operator==(const FsNodeEntry& rhs) const {
  return relativePathToNode == rhs.relativePathToNode && isDir == rhs.isDir && (isDir || contents == rhs.contents);
}

bool FsNodeEntry::operator<(const FsNodeEntry& rhs) const {
  return relativePathToNode < rhs.relativePathToNode;
}

bool CreateAFile(const QString& absFilePath, const QByteArray& contents) {
  QFile file{absFilePath};
  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }
  file.write(contents);
  file.close();
  return true;
}

QByteArray ReadAFile(const QString& absFilePath) {
  QFile file{absFilePath};
  if (!file.open(QIODevice::ReadOnly)) {
    return {};
  }
  return file.readAll();
}

#ifdef _WIN32
#include <shlwapi.h>  // PathFindFileNameW needed
bool EntryExistsWindowsCaseSensitive(const QString& path, QDir::Filters filter = QDir::NoFilter) {
  const std::wstring wPath = path.toStdWString();
  LPCWSTR inputPath = wPath.c_str();
  WIN32_FIND_DATAW findData;
  HANDLE hFind = FindFirstFileW(inputPath, &findData);
  if (hFind == INVALID_HANDLE_VALUE) {  // entry not exist
    return false;
  }
  FindClose(hFind);
  if (filter == QDir::Filter::NoFilter) {  // files or dirs
    return true;
  }
  if (bool(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  //
      != (filter == QDir::Filter::Dirs)) {                        //
    return false;                                                 // type not match, need dir but here is file
  }
  const std::wstring& actualName = PathFindFileNameW(findData.cFileName);
  const std::wstring& inputName = PathFindFileNameW(inputPath);
  return inputName == actualName;
}
#endif

TDir::TDir() : mTempPath{mTempDir.path()}, mDir{mTempPath} {
  if (!IsValid()) {
    // dir.path() returns the unique directory path
    qWarning("mTempDir is invalid");
    return;
  }
}

int TDir::createEntries(const QList<FsNodeEntry>& entries) {
  if (entries.isEmpty()) {
    return 0;
  }
  int filesCreatedSucceedCnt{0}, filesEntryCnt{0};
  int folderCreatedSucceedCnt{0}, foldersEntryCnt{0};
  for (const auto& entry : entries) {
    if (entry.isDir) {
      ++foldersEntryCnt;
      folderCreatedSucceedCnt += mkpath(entry.relativePathToNode);
    } else {
      ++filesEntryCnt;
      filesCreatedSucceedCnt += touch(entry.relativePathToNode, entry.contents);
    }
  }
  qDebug("Files:%d/%d, Folders:%d/%d created succeed", filesEntryCnt, filesCreatedSucceedCnt, foldersEntryCnt, folderCreatedSucceedCnt);
  return filesCreatedSucceedCnt + folderCreatedSucceedCnt;
}

QList<FsNodeEntry> TDir::getEntries(bool bFileContentMatter, const QDir::Filters filters) const {
  if (!IsValid()) {
    return {};
  }
  QList<FsNodeEntry> ans;
  QDirIterator it{mTempPath, {}, filters, QDirIterator::IteratorFlag::Subdirectories};
  int n = mTempPath.size() + 1;
  while (it.hasNext()) {
    const QString absPath = it.next();
    bool isDir = it.fileInfo().isDir();
    if (isDir || !bFileContentMatter) {
      ans.append({absPath.mid(n), isDir, {}});
      continue;
    }
    ans.append({absPath.mid(n), false, ReadAFile(absPath)});
  }
  std::sort(ans.begin(), ans.end());
  return ans;
}

bool TDir::touch(const QString& relativePathToFile, const QByteArray& contents) const {
  if (relativePathToFile.isEmpty()) {
    qWarning("Relative Path to File cannot be empty");
    return false;
  }
  if (!IsValid()) {
    qWarning("mTempDir is invalid, cannot touch file[%s]", qPrintable(relativePathToFile));
    return false;
  }
  const QString fileAbsPath = mTempDir.filePath(relativePathToFile);
  const QString prepath = PathTool::absolutePath(fileAbsPath);
  if (QFile::exists(prepath)) {
    if (!QFileInfo{prepath}.isDir()) {
      qWarning("path[%s] should be a directory but it is occupied by a file", qPrintable(prepath));
      return false;
    }
  } else {
    if (!QDir{}.mkpath(prepath)) {
      qWarning("mkpath[%s] failed cannot write", qPrintable(prepath));
      return false;
    }
  }
  return CreateAFile(fileAbsPath, contents);
}

bool TDir::fileExists(const QString& file, bool bWinCaseSensitive) const {
  if (file.isEmpty()) {
    return false;
  }
  const QString absFilePath{mTempPath + '/' + file};
#ifdef _WIN32
  if (!bWinCaseSensitive) {
    return QFileInfo{absFilePath}.isFile();
  }
  return EntryExistsWindowsCaseSensitive(QDir::toNativeSeparators(absFilePath), QDir::Filter::Files);
#else  // in linux always case-sensitive
  return QFileInfo{absFilePath}.isFile();
#endif
}

bool TDir::dirExists(const QString& folder, bool bWinCaseSensitive) const {
  if (folder.isEmpty()) {
    return false;
  }
  const QString absFilePath{mTempPath + '/' + folder};
#ifdef _WIN32
  if (!bWinCaseSensitive) {
    return QFileInfo{absFilePath}.isDir();
  }
  return EntryExistsWindowsCaseSensitive(QDir::toNativeSeparators(absFilePath), QDir::Filter::Dirs);
#else  // in linux always case-sensitive
  return QFileInfo{absFilePath}.isDir();
#endif
}

QStringList TDir::entryList(const QDir::Filters filters, const QDir::SortFlags sort) const {
  return mDir.entryList(filters, sort);
}
