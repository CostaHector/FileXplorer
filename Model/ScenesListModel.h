#ifndef SCENESLISTMODEL_H
#define SCENESLISTMODEL_H

#include "QAbstractListModelPub.h"
#include "SceneInfoManager.h"
#include <utility>
#include <QSet>
#include <QFileInfo>
#include <QPixmapCache>

class ScenesListModel : public QAbstractListModelPub {
  Q_OBJECT
 public:
  explicit ScenesListModel(QObject* object = nullptr);

  bool setRootPath(const QString& rootPath, const bool bForce = false);
  inline QString rootPath() const { return mRootPath; }

  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return mCurEnd - mCurBegin; }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  bool isIndexValid(const QModelIndex& index, int& linearInd) const;
  QFileInfo fileInfo(const QModelIndex& index) const;
  QString filePath(const QModelIndex& index) const;
  QString fileName(const QModelIndex& index) const;
  QString baseName(const QModelIndex& index) const;
  QString absolutePath(const QModelIndex& index) const;
  QStringList GetImgs(const QModelIndex& index) const;
  QStringList GetVids(const QModelIndex& index) const;
  QString GetJson(const QModelIndex& index) const;

  std::pair<int, int> GetEntryIndexBE(const int scenesCountPerPage, const int maxLen) const;

  inline int GetPageCnt() const {
    int N = GetEntryListLen();
    if (mScenesCountPerPage <= 0) return 0;
    return (N + mScenesCountPerPage - 1) / mScenesCountPerPage;
  }

  inline const SceneInfoList& GetEntryList() const { return mEntryList; }
  inline int GetEntryListLen() const { return GetEntryList().size(); }
  inline SceneInfoList::const_iterator GetFirstIterator() const { return mCurBegin; }

 signals:
  void pagesCountChanged(int newPagesCount);

 public slots:
  void onIconSizeChange(const QSize& newSize);
  bool onScenesCountsPerPageChanged(int scenesCntInAPage);
  bool onPageIndexChanged(int newPageIndex);

 private:
  int mPageIndex{0};
  int mScenesCountPerPage{12};  // 4-by-3
  QString mPattern;
  QString mRootPath;
  SceneInfoList mEntryList;
  SceneInfoList::const_iterator mCurBegin{mEntryList.cbegin()}, mCurEnd{mEntryList.cend()};

  QPixmapCache mPixCache;
  int mWidth = 404, mHeight = 250;
};
#endif  // SCENESLISTMODEL_H
