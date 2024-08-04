#include "RedundantFolderRemove.h"
#include <QDirIterator>
#include <cmath>

auto RedundantFolderRemove::CleanEmptyFolderCore(const QString& folderPath) -> int {
  QDir dir(folderPath);
  dir.setFilter(QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
  const int dirNameLen = dir.dirName().size();
  const QStringList& lst = dir.entryList();
  if (lst.isEmpty()) {
    m_cmds.append({"moveToTrash", "", folderPath});
    return 1;
  }

  if ((lst.size() == 1) && (std::abs(lst[0].size() - dirNameLen) <= TOLERANCE_LETTER_CNT)) {
    m_cmds.append({"rename", folderPath, lst[0], QFileInfo(folderPath).absolutePath(), lst[0]});
    m_cmds.append({"moveToTrash", "", folderPath});
    return 1;
  }
  return 0;
}

auto EmptyFolderRemove::CleanEmptyFolderCore(const QString& folderPath) -> int {
  QDir dir(folderPath);
  if (dir.isEmpty()) {
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

auto RedundantItemsRemoverByKeyword::CleanEmptyFolderCore(const QString& folderPath) -> int {
  const auto isFolderNeedRecycle = [](const QString& subfolderPath) -> bool {
    QDir dir{subfolderPath, "*", QDir::SortFlag::NoSort, QDir::AllEntries | QDir::NoDotAndDotDot};
    if (!dir.isEmpty(QDir::Dirs | QDir::NoDotAndDotDot)) {
      return false;
    }
    if (dir.count() > 10) {
      return false;
    }
    dir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET);
    if (dir.isEmpty(QDir::Files)) {
      return true;
    }
    return false;
  };

  QDirIterator rIt{
      folderPath, {"*" + m_keyword + "*"}, QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, QDirIterator::IteratorFlag::Subdirectories};
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
    m_cmds.append({"moveToTrash", "", subfolderPath});
  }
  return m_cmds.size();
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
