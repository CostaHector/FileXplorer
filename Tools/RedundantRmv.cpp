#include "RedundantRmv.h"
#include "PublicVariable.h"
#include "FileBatchOps.h"
// #include "NotificatorMacro.h"

#include "UndoRedo.h"
#include "NotificatorMacro.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>

int RedundantRmv::Find(const QString& path) {
  if (!QFileInfo{path}.isDir()) {
    LOG_W("path[%s] is not a directory", qPrintable(path));
    return 0;
  }
  m_workPath = path;
  return CleanEmptyFolderCore(path);
}

bool RedundantRmv::Exec() {
  if (!m_mixedCmds.isEmpty()) {
    const bool mixedCmdsSucceed = UndoRedo::GetInst().Do(m_mixedCmds);
    LOG_OE_P(mixedCmdsSucceed, "Mixed Commands", "Execute %d commands", m_mixedCmds.size());
    return mixedCmdsSucceed;
  } else if (!m_relNames.isEmpty()) {
    FileBatchOps::OpStatus recycleStatus = FileBatchOps::MoveToTrashSelected(m_workPath, m_relNames);
    return recycleStatus != FileBatchOps::OpStatus::PARTIAL_FAILED;
  } else {
    LOG_OK_NP("Skip", "Nothing to remove");
    return true;
  }
}

// --------- ZeroOrOneItemFolderProc
int ZeroOrOneItemFolderProc::CleanEmptyFolderCore(const QString& folderPath) {
  /*
Path/Movie                            folderPath                   dir
  -- Chris Evans                      subFolderName                subDir
  -- Michael Fassbender               subFolderName                subDir
    -- Michael Fassbender.mp4         subsubItemName 往上移一层
    -- Michael Fassbender 0.jpeg      subsubItemName
*/
  QDir dir{folderPath, "", QDir::SortFlag::NoSort, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};
  const QStringList folders{dir.entryList()};
  for (const QString& subFolderName : folders) {
    const QString subFullpath = dir.absoluteFilePath(subFolderName);
    const QString subRelative2WorkPath = subFullpath.mid(m_workPath.size() + 1);
    const QDir subDir{subFullpath, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot};
    const QStringList subsubItems = subDir.entryList();
    using namespace FileOperatorType;
    if (subsubItems.size() == 0) {
      // empty folder => recycle
      m_mixedCmds.append(ACMD::GetInstMOVETOTRASH(folderPath, subFolderName));
    } else if (subsubItems.size() == 1) {
      // folder contains only 1 item
      // item 往上移一层, 再删除folder自身
      const QString subsubItemName = subsubItems.front();
      if (!subsubItemName.startsWith(subFolderName)) {
        continue;
      }
      m_mixedCmds.append(ACMD::GetInstMV(subFullpath, subsubItemName, folderPath));
      m_mixedCmds.append(ACMD::GetInstMOVETOTRASH(folderPath, subFolderName));
    }
  }
  return m_mixedCmds.size() + m_relNames.size();
}

// --------- EmptyFolderRmv
QStringList EmptyFolderRmv::FindOutEmptyFolderOrAllSubFolderEmpty(const QString& folderPath) const {
  // find out folderPath which meet:
  // contain no files and whose subfolders are all empty
  QDir dir{folderPath, "", QDir::SortFlag::Name | QDir::SortFlag::DirsFirst};
  dir.setFilter(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot | QDir::Filter::Hidden);
  bool bAllSubFolderEmpty{true};
  QStringList fullPathLst;
  for (const QString& folderName: dir.entryList()) {
    QStringList lst{FindOutEmptyFolderOrAllSubFolderEmpty(folderPath + '/' + folderName)};
    if (lst.isEmpty()) {
      bAllSubFolderEmpty = false;
    } else {
      fullPathLst += lst;
    }
  }
  const bool bContainsNoFiles{dir.isEmpty(QDir::Filter::Files | QDir::Filter::Hidden)};
  if (bContainsNoFiles && bAllSubFolderEmpty) {
    fullPathLst.push_back(folderPath);
  }
  return fullPathLst;
}

int EmptyFolderRmv::CleanEmptyFolderCore(const QString& folderPath) {
  QStringList fullPathLst {FindOutEmptyFolderOrAllSubFolderEmpty(folderPath)};
  for (const QString& folderFullPath : fullPathLst) {
    m_relNames.push_back(folderFullPath.mid(m_workPath.size() + 1));
  }
  return fullPathLst.size();
}

// --------- FolderNameContainKeyRmv
int FolderNameContainKeyRmv::CleanEmptyFolderCore(const QString& folderPath) {
  const auto isFolderNeedRecycle = [](const QString& subfolderPath) -> bool {
    // contains directory =>no recycle
    // contains 10 files  =>no recycle
    // contains video     =>no recycle
    QDir dir{subfolderPath, "*", QDir::SortFlag::NoSort, QDir::AllEntries | QDir::NoDotAndDotDot};
    if (!dir.isEmpty(QDir::Dirs | QDir::NoDotAndDotDot)) {
      return false;
    }
    if (dir.count() > 10) {
      return false;
    }
    dir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET);
    if (!dir.isEmpty(QDir::Files)) {
      return false;
    }
    return true;
  };

  QDirIterator rIt{folderPath, {"*" + m_keyword + "*"}, QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDirIterator::IteratorFlag::Subdirectories};
  while (rIt.hasNext()) {
    rIt.next();
    if (!rIt.fileName().contains(m_keyword)) {
      continue;
    }
    const QString& subfolderPath = rIt.filePath();
    if (!isFolderNeedRecycle(subfolderPath)) {
      continue;
    }
    // recycle this folder
    m_relNames.push_back(subfolderPath.mid(m_workPath.size() + 1));
  }
  return m_relNames.size();
}
