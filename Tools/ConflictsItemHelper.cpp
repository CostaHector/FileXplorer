#include "ConflictsItemHelper.h"

QStringList Finder::FindAllItems(const QString& l, const QStringList& lRels) const {
  if (lRels.isEmpty()) {
    return {};
  }
  const int n = l.size();
  QStringList itemList;
  for (const QString& lName : lRels) {
    if (QFileInfo(l, lName).isFile()) {
      itemList.append(lName);
    } else {
      QStringList lst;
      lst.append(lName);
      QDirIterator it(l + '/' + lName, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot, QDirIterator::IteratorFlag::Subdirectories);
      while (it.hasNext()) {
        it.next();
        lst.append(it.filePath().mid(n + 1));
      }
      itemList += QStringList(lst.crbegin(), lst.crend());
    }
  }
  return itemList;
}

QStringList Finder::FindLLRelRCommon(const QString& l, const QStringList& lRels, const QString& r) const {
  if (lRels.isEmpty()) {
    return {};
  }
  QStringList lFileLst;
  QStringList lFolderLst;
  for (const QString& lName : lRels) {
    if (QFileInfo(l, lName).isFile()) {
      lFileLst.append(isLink() ? lName + ".lnk" : lName);
    } else {
      lFolderLst.append(isLink() ? lName + ".lnk" : lName);
    }
  }

  auto rFolderLst = QDir(r, "", QDir::SortFlag::NoSort, QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot).entryList();
  auto commonFoldersSet = QSet<QString>(lFolderLst.cbegin(), lFolderLst.cend()).intersect(QSet<QString>(rFolderLst.cbegin(), rFolderLst.cend()));
  QStringList commonFolders(commonFoldersSet.cbegin(), commonFoldersSet.cend());
  commonFolders.sort(Qt::CaseSensitivity::CaseInsensitive);

  QStringList commonList;
  for (const QString& folder : commonFolders) {
    const QString& newl = l + '/' + folder;
    const QString& newr = r + '/' + folder;
    const QStringList& newlRels = QDir(newl, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryList();
    const QStringList& tempDup = Finder::FindLLRelRCommon(newl, newlRels, newr);
    for (const QString& d : tempDup) {
      commonList.append(folder + '/' + d);
    }
    commonList.append(folder);
  }
  const QStringList& rFileLst = QDir(r, "", QDir::SortFlag::NoSort, QDir::Filter::Files).entryList();

  QSet<QString> commonFilesSet(lFileLst.cbegin(), lFileLst.cend());
  commonFilesSet.intersect(QSet<QString>(rFileLst.cbegin(), rFileLst.cend()));
  QStringList commonFiles(commonFilesSet.cbegin(), commonFilesSet.cend());
  commonFiles.sort(Qt::CaseSensitivity::CaseInsensitive);
  commonList += commonFiles;
  return commonList;
}
