#include "FileSystemHelper.h"
#include <QDirIterator>
#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QTextStream>
#include <QDataStream>

bool FileSystemNode::operator==(const FileSystemNode& rhs) const {
  if (fileName != rhs.fileName) {
    return false;
  }
  if (isDir != rhs.isDir) {
    return false;
  }
  if (isDir) {
    return true;
  }
  return fileContents == rhs.fileContents;
}

FileSystemHelper FileSystemHelper::GetSubHelper(const QString& subName) const {
  return FileSystemHelper{m_path + '/' + subName};
}

bool FileSystemHelper::EraseFileSystemTree(bool bRootInclude) const {
  if (m_path.isEmpty() || m_path == "/" || (m_path.size() >= 2 && (m_path.back() == ':' || m_path[m_path.size() - 2] == ':'))) {
    // "", "/", "C:", "C:/"
    qWarning("Remove root path[%s] is never allowed", qPrintable(m_path));
    return false;
  }
  if (!QFileInfo(m_path).isDir()) {
    qDebug("path[%s] (included) were already removed before or not directry.", qPrintable(m_path));
    return true;  // Already empty or non-existent
  }
  int directFoldersCnt{0}, directFilesCnt{0};
  QDir pathDir(m_path, "", QDir::SortFlag::Name | QDir::SortFlag::DirsFirst, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
  for (const QString& nm : pathDir.entryList()) {
    if (QFileInfo(m_path, nm).isDir()) {
      if (!QDir(m_path + '/' + nm).removeRecursively()) {
        qWarning("QDir(%s).removeRecursively failed", qPrintable(m_path));
        return false;
      }
      ++directFoldersCnt;
      continue;
    }
    if (!pathDir.remove(nm)) {
      qWarning("QDir(%s).remove(%s) failed", qPrintable(m_path), qPrintable(nm));
      return false;
    }
    ++directFilesCnt;
  }

  if (bRootInclude) {  // recover root path
    if (!pathDir.removeRecursively()) {
      qWarning("QDir(%s)removeRecursively() failed", qPrintable(m_path));
      return false;
    }
    ++directFoldersCnt;
  }
  qDebug("Direct removed %d folder(s) and %d file(s)", directFoldersCnt, directFilesCnt);
  return true;
}

FileSystemNode FileSystemHelper::GetNode(const QString& fileName, bool bIsText) const {
  bool isDir = QFileInfo(m_path, fileName).isDir();
  return FileSystemNode{fileName, isDir, (isDir ? QString("") : GetFileContent(fileName, bIsText))};
}

QString FileSystemHelper::GetFileContent(const QString& fileName, bool bIsText) const {
  QFile fileA(m_path + '/' + fileName);
  const auto readMode = bIsText ? QIODevice::ReadOnly | QIODevice::Text : QIODevice::ReadOnly;
  if (!fileA.open(readMode)) {
    qWarning("File open failed[%s]", qPrintable(fileName));
    return "";
  }
  if (bIsText) {
    QTextStream streamA(&fileA);
    QString ans = streamA.readAll();
    fileA.close();
    return ans;
  } else {
    QDataStream streamA(&fileA);
    char binData[1024] = "\0";
    streamA.readRawData(binData, sizeof(binData));
    fileA.close();
    return binData;
  }
}

bool FileSystemHelper::CreateATextFile(const QString& fileName, const QString& contents) const {
  QFile file(m_path + '/' + fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }
  QTextStream out(&file);
  out << contents;
  file.close();
  return true;
}

const FileSystemHelper& FileSystemHelper::operator<<(const FileSystemNode& node) const {
  static constexpr int n = 10;
  static constexpr int m = 10;
  static const auto STRIP_FIRST_AND_LAST = [](QString fileContents) -> QString {
    if (fileContents.size() <= n + m) {
      return fileContents;
    }
    fileContents.replace(n, fileContents.size() - n - m, "...");
    return fileContents;
  };
  if (node.isDir) {
    bool mkDirRet = QDir(m_path).mkdir(node.fileName);
    qDebug("\t-%d %64s\n", mkDirRet, qPrintable(node.fileName));
  } else {
    bool touchRet = CreateATextFile(node.fileName, node.fileContents);
    const QString dispContent = STRIP_FIRST_AND_LAST(node.fileContents);
    qDebug("\t %d %64s\t[%d char(s):%s]\n", touchRet, qPrintable(node.fileName), node.fileContents.size(), qPrintable(dispContent));
  }
  return *this;
}

bool IsTwoFileContentEqual(const QString& lhs, const QString& rhs, bool bIsText) {
  QFile fileA(lhs);
  QFile fileB(rhs);
  const auto readMode = QIODevice::ReadOnly | (bIsText ? QIODevice::Text : QIODevice::NotOpen);
  if (!fileA.open(readMode) || !fileB.open(readMode)) {
    qWarning("File open failed[%s] or [%s]", qPrintable(lhs), qPrintable(rhs));
    return false;
  }
  bool atEndSameTime{false};
  if (bIsText) {
    QTextStream streamA(&fileA), streamB(&fileB);
    QString lineA, lineB;
    while (!streamA.atEnd() && !streamB.atEnd()) {
      lineA = streamA.readLine();
      lineB = streamB.readLine();
      if (lineA != lineB) {
        return false;
      }
    }
    atEndSameTime = streamA.atEnd() && streamB.atEnd();
  } else {
    QDataStream streamA(&fileA), streamB(&fileB);
    quint8 byteA, byteB;
    while (!streamA.atEnd() && !streamB.atEnd()) {
      streamA >> byteA;
      streamB >> byteB;
      if (byteA != byteB) {
        return false;
      }
    }
    atEndSameTime = streamA.atEnd() && streamB.atEnd();
  }
  fileA.close();
  fileB.close();
  return atEndSameTime;
}

bool IsFileTreeEqual(const QString& lhs, const QString& rhs, const bool bCompareContent) {
  QFileInfo lFi{lhs}, rFi{rhs};
  if (lFi.fileName() != rFi.fileName()) {
    return false;
  }
  if (lFi.isFile() && rFi.isFile()) {
    return !bCompareContent || IsTwoFileContentEqual(lhs, rhs);
  } else if (lFi.isDir() && rFi.isDir()) {
    QDir lhsDir{lhs, "", QDir::SortFlag::Name, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot};
    QDir rhsDir{rhs, "", QDir::SortFlag::Name, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot};
    const auto& lhsLst = lhsDir.entryList();
    const auto& rhsLst = rhsDir.entryList();
    if (lhsLst.size() != rhsLst.size()) {
      return false;
    }
    for (int i = 0; i < lhsLst.size(); ++i) {
      if (!IsFileTreeEqual(lhs + '/' + lhsLst[i], rhs + '/' + rhsLst[i], bCompareContent)) {
        return false;
      }
    }
    return true;
  }
  qDebug("One File One Dir, lhs:%s, rhs:%s", qPrintable(lhs), qPrintable(rhs));
  return false;
}
