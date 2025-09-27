#ifndef SCENESLISTMODEL_H
#define SCENESLISTMODEL_H

#include "QAbstractListModelPub.h"
#include "SceneInfoManager.h"
#include <utility>
#include <QSet>
#include <QFileInfo>
#include <QPixmapCache>

class ScenesListModel : public QAbstractListModelPub {
public:
  explicit ScenesListModel(QObject* object = nullptr);

 bool setRootPath(const QString& rootPath, const bool bForce = false);
 inline QString rootPath() const { return mRootPath; }

  inline bool IsScnsEmpty() const { return mCurBegin == nullptr || mCurEnd == nullptr; }
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return IsScnsEmpty() ? 0 : mCurEnd - mCurBegin; }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  bool isIndexValid(const QModelIndex& index, int& linearInd) const;
  QFileInfo fileInfo(const QModelIndex& index) const;
  QString filePath(const QModelIndex& index) const;
  QString fileName(const QModelIndex& index) const;
  QString baseName(const QModelIndex& index) const;
  QString absolutePath(const QModelIndex& index) const;
  QStringList GetImgs(const QModelIndex& index) const;
  QStringList GetVids(const QModelIndex& index) const;

  bool ChangeItemsCntIn1Page(int scCnt1Page);

  bool SetPageIndex(int newPageIndex);
  std::pair<int, int> GetEntryIndexBE(int totalLen) const;

  inline int GetPageCnt() const {
    if (IsScnsEmpty()) {return 0;}
    int N = GetEntryListLen();
    return N / SCENES_CNT_1_PAGE + int(N % SCENES_CNT_1_PAGE != 0);
  }

  inline const SCENE_INFO_LIST& GetEntryList() const { return mEntryList; }
  inline int GetEntryListLen() const { return GetEntryList().size(); }

public slots:
  void onIconSizeChange(const QSize& newSize);

private:
  int mPageIndex{0};
  int SCENES_CNT_1_PAGE{12};  // 4-by-3
  QString mPattern;
  QString mRootPath;
  SCENE_INFO_LIST mEntryList;
  SCENE_INFO_LIST::const_iterator mCurBegin{nullptr}, mCurEnd{nullptr};

  QPixmapCache mPixCache;
  int mWidth = 404, mHeight = 250;
};
#endif  // SCENESLISTMODEL_H
