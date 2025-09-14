#include "FileOsWalker.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include "PathTool.h"

FileOsWalker::FileOsWalker(const QString& pre, bool sufInside)
    : mPrepathWithSlash{PathTool::normPath(pre) + '/'},  // "rel/"
      N{mPrepathWithSlash.size()},                       // N = len("rel/")
      mSufInside{sufInside}                              // a.txt
{                                                        //
}

struct KeyInfo {
  QString absFilePath;
  bool isFiles;
  QString relToNames;
  QString completeNames;
  QString suffixs;
  bool operator<(const KeyInfo& rhs) const { return absFilePath < rhs.absFilePath; }
};

KeyInfo FillByFileInfo(const QFileInfo& fi, const int relStartIndexN, const bool sufInsideFileName) {
  if (sufInsideFileName) {
    return {fi.absoluteFilePath(), fi.isFile(), fi.absolutePath().mid(relStartIndexN), fi.fileName(), ""};
  } else {
    QString completeNm, dotSuf;
    std::tie(completeNm, dotSuf) = PathTool::GetBaseNameExt(fi.fileName());
    return {fi.absoluteFilePath(), fi.isFile(), fi.absolutePath().mid(relStartIndexN), completeNm, dotSuf};
  }
}

void FileOsWalker::operator()(const QStringList& rels, const bool includingSub) {
  // Reverse the return value, One can get bottom To Top result like os.walk
  QList<KeyInfo> itemsLst;
  for (const QString& rel : rels) {
    const QFileInfo fi{mPrepathWithSlash + rel};
    itemsLst.push_back(FillByFileInfo(fi, N, mSufInside));
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
      tempItemsLst.push_back(FillByFileInfo(it.fileInfo(), N, mSufInside));
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
  }
}
