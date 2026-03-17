#ifndef FILEOSWALKER_H
#define FILEOSWALKER_H

#include <QStringList>
class QFileInfo;

class FileOsWalker {
 public:
  FileOsWalker(const QString& pre, bool sufInside);
  void operator()(const QStringList& rels, const bool includingSub);
  inline int size() const { return completeNames.size(); }

  QStringList relToNames;
  QStringList completeNames;
  QStringList suffixs;
  QList<bool> isFiles;
  QStringList filesFullPath;

  const QString mPrepathWithSlash;
  const int N;

 private:
  const bool mSufInside;
};
#endif // FILEOSWALKER_H
