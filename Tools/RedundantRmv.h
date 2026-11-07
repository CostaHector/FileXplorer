#ifndef REDUNDANTRMV_H
#define REDUNDANTRMV_H

#include "FileOperatorPub.h"
#include <QStringList>

class RedundantRmv {
public:
  RedundantRmv() = default;
  virtual ~RedundantRmv() = default;
  int Find(const QString& path);
  bool Exec();
protected:
  QString m_workPath;
  QStringList m_relNames;
  FileOperatorType::BATCH_COMMAND_LIST_TYPE m_mixedCmds; // 前置工作
private:
  virtual int CleanEmptyFolderCore(const QString& folderPath) = 0;
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
  using RedundantRmv::RedundantRmv;
  // defence : including subfolder may cause huge problem to the whole file system;
private:
  int CleanEmptyFolderCore(const QString& folderPath) override;
};

// EmptyFolderRmv:
// A/{} => recycle folder A
// A/{A.mp4} => do nothing
// A/B/{}, A/C/{}=> recycle B and C
class EmptyFolderRmv : public RedundantRmv {
public:
  using RedundantRmv::RedundantRmv;
private:
  int CleanEmptyFolderCore(const QString& folderPath) override;
  QStringList FindOutEmptyFolderOrAllSubFolderEmpty(const QString& folderPath) const;
};

class FolderNameContainKeyRmv : public RedundantRmv {
public:
  explicit FolderNameContainKeyRmv(const QString& keyword) : RedundantRmv(), m_keyword{keyword} {}
private:
  int CleanEmptyFolderCore(const QString& folderPath) override;
  const QString m_keyword;
};
#endif  // REDUNDANTRMV_H
