#include "RedundantFolderRemove.h"
#include "public/PublicVariable.h"
#include "public/UndoRedo.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>
#include <cmath>

int RedundantRmv::operator()(const QString& path) {
  QFileInfo fi(path);
  if (!fi.isDir()) {
    qWarning("path[%s] is not a directory", qPrintable(path));
    return 0;
  }
  return CleanEmptyFolderCore(fi.absoluteFilePath());
}

bool RedundantRmv::Exec() {
  if (m_cmds.isEmpty()) {
    qDebug("nothing to remove");
    return true;
  }
  const bool isAllSucceed = g_undoRedo.Do(m_cmds);
  m_cmds.clear();
  return isAllSucceed;
}

using namespace FileOperatorType;
constexpr int ZeroOrOneItemFolderProc::TOLERANCE_LETTER_CNT;

auto ZeroOrOneItemFolderProc::CleanEmptyFolderCore(const QString& folderPath) -> int {
  m_cmds.clear();
  if (!QFileInfo(folderPath).isDir()) {
    qDebug("Path[%s] is not a folder", qPrintable(folderPath));
    return -1;
  }
  QDir dir{folderPath, "", QDir::SortFlag::NoSort, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};
  const int dirNameLen = dir.dirName().size();
  for (const QString& sub : dir.entryList()) {
    QDir subDir{dir.absoluteFilePath(sub), "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot};
    switch (subDir.count()) {
      case 0: {
        // empty folder => recycle
        m_cmds.append(ACMD::GetInstMOVETOTRASH(folderPath, sub));
        break;
      }
      case 1: {
        // 1 file folder and delta(length)>TOLERANCE_LETTER_CNT => no recycle
        const QString& itemName = dir.entryList().front();
        if (std::abs(itemName.size() - dirNameLen) > TOLERANCE_LETTER_CNT) {
          qDebug("ignore parent folder name len:%d, item name len:%d", itemName.size(), dirNameLen);
          break;
        }
        m_cmds.append(ACMD::GetInstMV(subDir.absolutePath(), itemName, folderPath));
        m_cmds.append(ACMD::GetInstMOVETOTRASH(folderPath, sub));
        break;
      }
      default:
        break;
    }
  }
  return m_cmds.size();
}

auto EmptyFolderRmv::CleanEmptyFolderCore(const QString& folderPath) -> int {
  // as recursive calling, m_cmds will not clean automatically
  QDir dir(folderPath);
  if (dir.isEmpty()) {
    m_cmds.append(ACMD::GetInstMOVETOTRASH("", folderPath));
    return 1;
  }
  dir.setFilter(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot);
  int totalCount = 0;
  for (const QString& path : dir.entryList()) {
    totalCount += CleanEmptyFolderCore(folderPath + '/' + path);
  }
  return totalCount;
}

auto FolderNameContainKeyRmv::CleanEmptyFolderCore(const QString& folderPath) -> int {
  m_cmds.clear();
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
    m_cmds.append(ACMD::GetInstMOVETOTRASH("", subfolderPath));
  }
  return m_cmds.size();
}
