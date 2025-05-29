#ifndef REDUNDANTFOLDERREMOVE_H
#define REDUNDANTFOLDERREMOVE_H
#include "FileOperation/FileOperatorPub.h"
class RedundantRmv {
 public:
  RedundantRmv() = default;
  ~RedundantRmv() = default;

  FileOperatorType::BATCH_COMMAND_LIST_TYPE m_cmds;

  virtual auto CleanEmptyFolderCore(const QString& folderPath) -> int = 0;
  int operator()(const QString& path);
  bool Exec();

  operator QString() const {
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

class ZeroOrOneItemFolderProc : public RedundantRmv {
 public:
  constexpr static int TOLERANCE_LETTER_CNT = 6;
  ZeroOrOneItemFolderProc() : RedundantRmv() {
    // defence : including subfolder may cause huge problem to the whole file system;
  }
  auto CleanEmptyFolderCore(const QString& folderPath) -> int override;
};

// EmptyFolderRmv:
// A/{} => recycle folder A
// A/{A.mp4} => do nothing
// A/B/{}, A/C/{}=> recycle B and C
class EmptyFolderRmv : public RedundantRmv {
 public:
  EmptyFolderRmv() : RedundantRmv() {}
  auto CleanEmptyFolderCore(const QString& folderPath) -> int override;
};

class FolderNameContainKeyRmv : public RedundantRmv {
 public:
  explicit FolderNameContainKeyRmv(const QString& keyword) : RedundantRmv(), m_keyword{keyword} {}
  auto CleanEmptyFolderCore(const QString& folderPath) -> int override;

 private:
  const QString m_keyword;
};
#endif  // REDUNDANTFOLDERREMOVE_H
