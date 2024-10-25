#ifndef FILESYSTEMHELPER_H
#define FILESYSTEMHELPER_H

#include <QString>

struct FileSystemNode {
  FileSystemNode(QString&& _nm, bool _isDir = true, QString&& _cont = "") : fileName{_nm}, isDir{_isDir}, fileContents{_cont} {}
  FileSystemNode(const QString& _nm, bool _isDir = true, const QString& _cont = "") : fileName{_nm}, isDir{_isDir}, fileContents{_cont} {}
  QString fileName;
  bool isDir;
  QString fileContents;
  bool operator==(const FileSystemNode& rhs) const;
};

class FileSystemHelper {
 public:
  FileSystemHelper(const QString& initPth = "") : m_path{initPth} {}
  const FileSystemHelper& ChangePath(const QString& newPath) {
    m_path = newPath;
    qDebug("Into path %s:\n", qPrintable(m_path));
    return *this;
  }
  FileSystemHelper GetSubHelper(const QString& subName) const;

  bool EraseFileSystemTree(bool bRootInclude = false) const;

  FileSystemNode GetNode(const QString& fileName, bool bIsText = true) const;

  QString GetFileContent(const QString& fileName, bool bIsText = true) const;

  bool CreateATextFile(const QString& fileName, const QString& contents) const;

  const FileSystemHelper& operator<<(const FileSystemNode& node) const;

 private:
  QString m_path;
};

bool IsTwoFileContentEqual(const QString& lhs, const QString& rhs, bool bIsText = true);
bool IsFileTreeEqual(const QString& lhs, const QString& rhs, const bool bCompareContent = false);

#endif  // FILESYSTEMHELPER_H
