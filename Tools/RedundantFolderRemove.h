#ifndef REDUNDANTFOLDERREMOVE_H
#define REDUNDANTFOLDERREMOVE_H

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
      s += (l.toStr() + '|');
    }
    return s;
  }
};

/* Here we call a folder with no item or only one item redundant folder
In that case, we will move the file (if it exists) to its upper level folder.
And erase the redundant folder */
/* usage example:
A/{B, C} => do nothing
A/{ABCautoGEHI} => do nothing, because len(sub)-len(parent) > TOLERANCE_LETTER_CNT
A/{} => recycle parent A
A/{AB} => upgrade AB level, then recycle parent A
A/{A.mp4} => upgrade A.mp4 level, then recycle folder A
*/

class RedunParentFolderRem : public RedundantRmv {
 public:
  constexpr static int TOLERANCE_LETTER_CNT = 6;
  RedunParentFolderRem() : RedundantRmv() {
    // defence : including subfolder may cause huge problem to the whole file system;
  }
  auto CleanEmptyFolderCore(const QString& folderPath) -> int override;
};

// EmptyFolderRemove:
// A/{} => recycle folder A
// A/{A.mp4} => do nothing
// A/B/{}, A/C/{}=> recycle B and C
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
