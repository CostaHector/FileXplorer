#include "RedundantFolderRemove.h"

auto EmptyFolderRemove::CleanEmptyFolderCore(const QString& folderPath) -> int {
  QDir dir(folderPath);
  if (dir.isEmpty()) {
    m_cmds.append({"moveToTrash", "", folderPath});
    return 1;
  }
  if (not m_includingSubFolder) {
    if (not dir.isEmpty()) {
      return 0;
    }
    m_cmds.append({"moveToTrash", "", folderPath});
    return 1;
  }
  dir.setFilter(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot);
  int totalCount = 0;
  for (const QString& path : dir.entryList()) {
    totalCount += CleanEmptyFolderCore(folderPath + '/' + path);
  }
  return totalCount;
}

auto RedundantFolderRemove::CleanEmptyFolderCore(const QString& folderPath) -> int {
  QDir dir(folderPath);
  dir.setFilter(QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
  const int dirNameLen = dir.dirName().size();
  const QStringList& lst = dir.entryList();
  if (lst.isEmpty()) {
    m_cmds.append({"moveToTrash", "", folderPath});
    return 1;
  }
  if (lst.size() == 1 and (-TOLERANCE_LETTER_CNT <= lst[0].size() - dirNameLen and lst[0].size() - dirNameLen <= TOLERANCE_LETTER_CNT)) {
    m_cmds.append({"rename", folderPath, lst[0], QFileInfo(folderPath).absolutePath(), lst[0]});
    m_cmds.append({"moveToTrash", "", folderPath});
    return 1;
  }
  // A/ABCautoGEHI => keep;
  // A/A.mp4 => move A.mp4 to its up level folder;
  return 0;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main() {
  EmptyFolderRemove efr(true);
  int cnt = efr("../");
  qDebug("EmptyFolderRemove %d", cnt);
  qDebug() << efr.m_removeList;

  RedundantFolderRemove rfr;
  cnt = rfr("../");
  qDebug("RedundantFolderRemove %d", cnt);
  qDebug() << rfr.m_moveList;
  return 0;
}
#endif
