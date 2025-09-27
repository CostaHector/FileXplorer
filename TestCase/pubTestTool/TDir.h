#ifndef TDIR_H
#define TDIR_H

#include <QDir>
#include <QDirIterator>
#include <QTemporaryDir>
#include <QSet>

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
  ~TDir() = default;

  static QSet<QString> SnapshotAtPath(const QString& path,
                                      const QDir::Filters filters = DEFAULT_QDIR_FILTER,
                                      QDirIterator::IteratorFlag iterFlag = QDirIterator::IteratorFlag::Subdirectories);
  static bool checkFileContentsAtPath(const QString& absFilePath, const QSet<QString>& containsSet = {}, const QSet<QString>& notContainsSet = {});
  static QStringList FilesContentsSnapshotAtPath(const QStringList& filesAbsPath);

  operator QDir() const { return mDir; }
  bool IsValid() const { return mTempDir.isValid(); }
  bool IsEmpty(QDir::Filters filters = DEFAULT_QDIR_FILTER) const { return mDir.isEmpty(filters); }
  int createEntries(const QList<FsNodeEntry>& entries);
  QList<FsNodeEntry> getEntries(bool bFileContentMatter = false, const QDir::Filters filters = DEFAULT_QDIR_FILTER) const;
  bool touch(const QString& relativePathToFile, const QByteArray& contents = "") const;
  bool mkdir(const QString& folderName) const { return mDir.mkdir(folderName); }
  bool mkpath(const QString& dirPath) const { return mDir.mkpath(dirPath); }
  bool exists(const QString& path2Item) const { return mDir.exists(path2Item); }
  bool fileExists(const QString& file, bool bWinCaseSensitive = false) const;
  bool dirExists(const QString& folder, bool bWinCaseSensitive = false) const;
  QStringList entryList(const QDir::Filters filters = DEFAULT_QDIR_FILTER, const QDir::SortFlags sort = QDir::NoSort) const;
  QString baseName() const {return QFileInfo(path()).baseName(); }
  QString path() const { return mTempPath; }
  QString itemPath(const QString& itemName) const { return mTempPath + '/' + itemName; }
  QSet<QString> Snapshot(const QDir::Filters filters = DEFAULT_QDIR_FILTER,
                         QDirIterator::IteratorFlag iterFlag = QDirIterator::IteratorFlag::Subdirectories) const {
    return SnapshotAtPath(path(), filters, iterFlag);
  }
  QStringList FilesContentsSnapshot(const QStringList& relfilesAbsPath) const {
    QStringList filesAbsPath;
    filesAbsPath.reserve(relfilesAbsPath.size());
    for (const QString& relPath: relfilesAbsPath) {
      filesAbsPath.push_back(itemPath(relPath));
    }
    return FilesContentsSnapshotAtPath(filesAbsPath);
  }
  bool checkFileContents(const QString& relativePath2File, const QSet<QString>& containsSet = {}, const QSet<QString>& notContainsSet = {}) const {
    return checkFileContentsAtPath(itemPath(relativePath2File), containsSet, notContainsSet);
  }

  bool ClearAll();
  void remove() { mTempDir.remove(); }

  static constexpr QDir::Filters DEFAULT_QDIR_FILTER = QDir::Filter::Dirs | QDir::Filter::Files | QDir::Filter::NoDotAndDotDot | QDir::Filter::Hidden;

 private:
  QTemporaryDir mTempDir;
  QString mTempPath;
  QDir mDir;
};

struct AutoRollbackRename final {
 public:
  AutoRollbackRename(QString srcPath, QString dstPath);
  AutoRollbackRename(const QString& prepath, const QString& relSrc1, const QString& relDst2)  //
      : AutoRollbackRename{QDir{prepath}.absoluteFilePath(relSrc1), QDir{prepath}.absoluteFilePath(relDst2)} {}
  ~AutoRollbackRename();
  bool Execute();

 private:
  bool StartToRename(const QString& hintMsg);
  QString mSrcAbsFilePath, mDstAbsFilePath;
  bool mNeedRollback{false};
};

struct AutoRollbackFileContentModify final {
 public:
  AutoRollbackFileContentModify(const QString& absFilePath, const QString& replaceeStr, const QString& replacerStr);
  AutoRollbackFileContentModify(const QString& absFilePath, const QString& newContents);
  ~AutoRollbackFileContentModify();
  bool Execute();

 private:
  enum class Mode { ReplaceMode, FullReplaceMode };
  bool StartToModify(const QString& hintMsg);
  const QString mAbsFilePath;
  const QString mReplaceeStr, mReplacerStr;  // replacee->replacer
  const QString mNewContents;                // full replace
  const Mode mMode;
  QString mOriginContents;
  bool mNeedRollback{false};
};
#endif  // TDIR_H
