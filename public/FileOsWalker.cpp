#include "FileOsWalker.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include "PathTool.h"

FileOsWalker::FileOsWalker(const QString& pre, bool sufInside)
    : mPrepathWithoutSlash{PathTool::normPath(pre)},  // "rel" must not endswith "/"
      N{mPrepathWithoutSlash.size()},                 // N = len("rel/")
      mSufInside{sufInside}                           // a.txt
{                                                     //
}

struct KeyInfo {
  QString absFilePath;
  bool isFiles;
  QString relToNames;
  QString completeNames;
  QString suffixs;
  bool operator<(const KeyInfo& rhs) const { return absFilePath < rhs.absFilePath; }
};

// fullPath=C:/A/B/C.mp4
// rootPath=C:
// rel2File="A/B"
// fileName="C.mp4"
KeyInfo FillByFileInfo(const QString& fullPath, const int relStartIndexN, const bool sufInsideFileName) {
  QString fileName = PathTool::fileName(fullPath);
  QString rel2File = PathTool::GetRelPathFromRootRelName(relStartIndexN, fullPath, fileName.size());
  if (sufInsideFileName) {
    return {fullPath, QFileInfo{fullPath}.isFile(), rel2File, fileName, ""};
  } else {
    QString completeNm, dotSuf;
    std::tie(completeNm, dotSuf) = PathTool::GetBaseNameExt(fileName);
    return {fullPath, QFileInfo{fullPath}.isFile(), rel2File, completeNm, dotSuf};
  }
}

// rels not start with "/"
void FileOsWalker::operator()(const QStringList& rels, const bool includingSub) {
  // Reverse the return value, One can get bottom To Top result like os.walk
  QList<KeyInfo> itemsLst;
  for (const QString& rel : rels) {
    const QString fullPath = mPrepathWithoutSlash + "/" + rel;
    const QFileInfo fi{fullPath};
    itemsLst.push_back(FillByFileInfo(fullPath, N, mSufInside));
    if (!includingSub) {
      continue;
    }
    if (!fi.isDir()) {
      continue;
    }
    // folders
    decltype(itemsLst) tempItemsLst;
    QDirIterator it(fi.absoluteFilePath(), {}, QDir::Filter::NoDotAndDotDot | QDir::Filter::AllEntries, QDirIterator::IteratorFlag::Subdirectories);
    while (it.hasNext()) {
      it.next();
      tempItemsLst.push_back(FillByFileInfo(it.filePath(), N, mSufInside));
    }    
    std::sort(tempItemsLst.begin(), tempItemsLst.end());
    itemsLst += std::move(tempItemsLst);
  }

  relToNames.reserve(itemsLst.size());
  completeNames.reserve(itemsLst.size());
  suffixs.reserve(itemsLst.size());
  isFiles.reserve(itemsLst.size());
  for (auto& item : itemsLst) {
    relToNames.push_back(item.relToNames);
    completeNames.push_back(item.completeNames);
    suffixs.push_back(item.suffixs);
    isFiles.push_back(item.isFiles);
    filesFullPath.push_back(item.absFilePath);
  }
}
