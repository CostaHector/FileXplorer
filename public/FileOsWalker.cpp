#include "FileOsWalker.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>
#include "PathTool.h"

FileOsWalker::FileOsWalker(const QString& pre, bool sufInside)
    : mPrepathWithSlash{PathTool::normPath(pre) + '/'},  // "rel/"
      N{mPrepathWithSlash.size()},                       // N = len("rel/")
      mSufInside{sufInside}                              // a.txt
{                                                        //
}

void FileOsWalker::operator()(const QStringList& rels, const bool includingSub) {
  // Reverse the return value, One can get bottom To Top result like os.walk
  for (const QString& rel : rels) {
    const QFileInfo fi{mPrepathWithSlash + rel};
    FillByFileInfo(fi);
    if (!includingSub) {
      continue;
    }
    if (!fi.isDir()) {
      continue;
    }
    // folders
    QDirIterator it(fi.absoluteFilePath(), {}, QDir::Filter::NoDotAndDotDot | QDir::Filter::AllEntries, QDirIterator::IteratorFlag::Subdirectories);
    while (it.hasNext()) {
      it.next();
      const QFileInfo subFi{it.fileInfo()};
      FillByFileInfo(subFi);
    }
  }
}

void FileOsWalker::FillByFileInfo(const QFileInfo& fi) {
  QString completeNm, dotSuf;
  isFiles.append(fi.isFile());
  relToNames.append(fi.absolutePath().mid(N));
  if (mSufInside) {
    completeNames.append(fi.fileName());
    suffixs.append("");
  } else {
    std::tie(completeNm, dotSuf) = PathTool::GetBaseNameExt(fi.fileName());
    completeNames.append(completeNm);
    suffixs.append(dotSuf);
  }
}
