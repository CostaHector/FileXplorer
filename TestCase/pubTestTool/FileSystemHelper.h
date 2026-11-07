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

#endif  // FILESYSTEMHELPER_H
