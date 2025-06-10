#include "TDir.h"
#include "public/PathTool.h"

#include <QTextStream>
bool CreateAFile(const QString& absFilePath, const QByteArray& contents) {
  QFile file{absFilePath};
  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }
  file.write(contents);
  file.close();
  return true;
}

TDir::TDir() : mTempPath{mTempDir.path()}, mDir{mTempPath} {
  if (!IsValid()) {
    // dir.path() returns the unique directory path
    qWarning("mTempDir is invalid");
    return;
  }
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
// for bWinCaseSensitive=true, file cannot be relative
bool TDir::fileExists(const QString& file, bool bWinCaseSensitive) const {
  if (file.isEmpty()) {
    return false;
  }
  const QString absFilePath{mTempPath + '/' + file};
#ifdef _WIN32
  return bWinCaseSensitive ? mDir.entryList(QDir::Filter::Files).contains(file) : QFileInfo{absFilePath}.isFile();
#else  // in linux always case-sensitive
  return QFileInfo{absFilePath}.isFile();
#endif
}
// for bWinCaseSensitive=true, folder cannot be relative
bool TDir::dirExists(const QString& folder, bool bWinCaseSensitive) const {
  if (folder.isEmpty()) {
    return false;
  }
  const QString absFilePath{mTempPath + '/' + folder};
#ifdef _WIN32
  return bWinCaseSensitive ? mDir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot).contains(folder) : QFileInfo{absFilePath}.isDir();
#else  // in linux always case-sensitive
  return QFileInfo{absFilePath}.isDir();
#endif
}

QStringList TDir::entryList(QDir::Filters filters, QDir::SortFlags sort) const {
  return mDir.entryList(filters, sort);
}
