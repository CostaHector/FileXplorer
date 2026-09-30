#ifndef FOLDERNXTANDLASTITERATOR_H
#define FOLDERNXTANDLASTITERATOR_H

#include <QString>
#include <QDir>

class FolderNxtAndLastIterator {
 public:
  enum class NaviDirection {
    PREV,
    NEXT,
  };

  explicit FolderNxtAndLastIterator(const QStringList& nameFilters = {},
                                    QDir::Filters dirFilters = QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot,
                                    bool bIncludingSubDir = false);
  static FolderNxtAndLastIterator GetInstsNaviFolders();
  static FolderNxtAndLastIterator GetInstsNaviImages(bool bIncludingSubDir);

  bool operator()(const QString& parentPath, bool bForce = false);

  QString next(const QString& parentPath, const QString& curItemName) { return lastNextCore(parentPath, curItemName, NaviDirection::NEXT); }
  QString last(const QString& parentPath, const QString& curItemName) { return lastNextCore(parentPath, curItemName, NaviDirection::PREV); }

  void setIncludingSubDirectory(bool bInclude) { mIncludingSubDirectory = bInclude; }
  bool IsIncludingSubDirectory() const {return mIncludingSubDirectory;}

 private:
  QString lastNextCore(const QString& parentPath, const QString& curItemName, NaviDirection direction = NaviDirection::NEXT);
  QString GetDestinationItem(const QString& parentPath, const QString& curItemName, NaviDirection direction) const;
  QString m_lastTimeParentPath;

  bool operator()(const QString& parentPath, QStringList& itemsListDisposable); // test only
  void initSameLevelPaths(QStringList& itemsListDisposable) {
    mSameLevelPaths.swap(itemsListDisposable);
    mSameLevelPathsLowercase.clear();
    mSameLevelPathsLowercase.reserve(mSameLevelPaths.size());
    for (const QString& s: mSameLevelPaths) {
      mSameLevelPathsLowercase.push_back(s.toLower());
    }
  }
  QStringList mSameLevelPaths, mSameLevelPathsLowercase;

  const QStringList mNameFilters;
  const QDir::Filters mDirFilters;
  bool mIncludingSubDirectory;
};

#endif  // FOLDERNXTANDLASTITERATOR_H
