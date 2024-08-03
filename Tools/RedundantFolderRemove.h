#ifndef REDUNDANTFOLDERREMOVE_H
#define REDUNDANTFOLDERREMOVE_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "UndoRedo.h"

class RedundantRmv {
 public:
  RedundantRmv() = default;
  ~RedundantRmv() = default;

  FileOperatorType::BATCH_COMMAND_LIST_TYPE m_cmds;

  virtual auto CleanEmptyFolderCore(const QString& folderPath) -> int = 0;

  auto operator()(const QString& path) -> int {
    QFileInfo fi(path);
    if (!fi.isDir()) {
      qWarning("path[%s] is not a directory", qPrintable(path));
      return 0;
    }
    return CleanEmptyFolderCore(fi.absoluteFilePath());
  }

  auto Exec() -> bool {
    if (m_cmds.isEmpty()) {
      qDebug("nothing to remove");
      return true;
    }
    const bool isAllSucceed = g_undoRedo.Do(m_cmds);
    m_cmds.clear();
    return isAllSucceed;
  }

  operator QString() {
    QString s;
    for (const auto& l : m_cmds) {
      s += (l.join(',') + '|');
    }
    return s;
  }
};

// RedundantFolderRemove:
// A/nothing => recycle folder A
// A/ABCautoGEHI => keep folder A and folder ABCautoGEHI
// A/A.mp4 => move A.mp4 to its up level folder and recycle folder A
class RedundantFolderRemove : public RedundantRmv {
 public:
  /* Here we call a folder with no item or only one item redundant folder
  In that case, we will move the file (if it exists) to its upper level folder. And erase the redundant folder */
  constexpr static int TOLERANCE_LETTER_CNT = 6;
  RedundantFolderRemove() : RedundantRmv() {
    // defence : including subfolder may cause huge problem to the whole file system;
  }
  auto CleanEmptyFolderCore(const QString& folderPath) -> int override;
};

// EmptyFolderRemove:
// A/nothing => recycle folder A
// A/A.mp4 => keep
// A/B/nothing, A/C/nothing => recycle nothingx2
class EmptyFolderRemove : public RedundantRmv {
 public:
  EmptyFolderRemove() : RedundantRmv() {}
  auto CleanEmptyFolderCore(const QString& folderPath) -> int override;
};

class RedundantItemsRemoverByKeyword : public RedundantRmv {
 public:
  explicit RedundantItemsRemoverByKeyword(const QString& keyword) : RedundantRmv(), m_keyword{keyword} {}
  auto CleanEmptyFolderCore(const QString& folderPath) -> int override;

 private:
  const QString m_keyword;
};
#endif  // REDUNDANTFOLDERREMOVE_H
