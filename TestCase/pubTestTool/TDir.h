#ifndef TDIR_H
#define TDIR_H

#include <QDir>
#include <QTemporaryDir>

struct FsNodeEntry {
  QString relativePathToNode;
  bool isDir;
  QByteArray contents;
  bool operator==(const FsNodeEntry& rhs) const;
  bool operator<(const FsNodeEntry& rhs) const;
};

bool CreateAFile(const QString& absFilePath, const QByteArray& contents = "");

class TDir {
 public:
  TDir();

  operator QDir() const { return mDir; }
  bool IsValid() const { return mTempDir.isValid(); }

  int createEntries(const QList<FsNodeEntry>& entries);
  QList<FsNodeEntry> getEntries(bool bFileContentMatter = false, const QDir::Filters filters = QDir::Filter::Dirs | QDir::Filter::Files | QDir::Filter::NoDotAndDotDot | QDir::Filter::Hidden) const;
  bool touch(const QString& relativePathToFile, const QByteArray& contents = "") const;
  bool mkdir(const QString& folderName) const { return mDir.mkdir(folderName); }
  bool mkpath(const QString& dirPath) const { return mDir.mkpath(dirPath); }
  bool exists(const QString& path2Item) const { return mDir.exists(path2Item); }
  bool fileExists(const QString& file, bool bWinCaseSensitive = false) const;
  bool dirExists(const QString& folder, bool bWinCaseSensitive = false) const;
  QStringList entryList(const QDir::Filters filters = QDir::Filter::Dirs | QDir::Filter::Files | QDir::Filter::NoDotAndDotDot | QDir::Filter::Hidden, const QDir::SortFlags sort = QDir::NoSort) const;
  QString path() const { return mTempPath; }
  QString itemPath(const QString& itemName) const { return mTempPath + '/' + itemName; }

 private:
  QTemporaryDir mTempDir;
  QString mTempPath;
  QDir mDir;
};

#endif  // TDIR_H
