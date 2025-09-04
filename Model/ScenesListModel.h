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
  ScenesListModel(QObject* object = nullptr);
  inline bool IsScnsEmpty() const { return mCurBegin == nullptr || mCurEnd == nullptr || mCurBegin == mCurEnd; }
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return mCurEnd - mCurBegin; }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  inline bool isIndexValid(const QModelIndex& index, int& linearInd) const {
    if (!index.isValid()) {
      return false;
    }
    if (mCurBegin + index.row() >= mCurEnd) {
      qWarning("Invalid index(%d) user input", index.row());
      return false;
    }
    linearInd = index.row();
    return true;
  }
  QFileInfo fileInfo(const QModelIndex& index) const;
  QString filePath(const QModelIndex& index) const;
  QString fileName(const QModelIndex& index) const;
  QString baseName(const QModelIndex& index) const;
  QString absolutePath(const QModelIndex& index) const;
  bool setRootPath(const QString& rootPath, const bool bForce = false);
  inline QString rootPath() const { return mRootPath; }
  QStringList GetImgs(const QModelIndex& index) const;
  QStringList GetVids(const QModelIndex& index) const;

  bool ChangeItemsCntIn1Page(int scCnt1Page);
  bool ShowAllScenesInOnePage();

  void SortOrder(SceneInfoManager::SceneSortOption sortOption, bool reverse = false);
  bool SetPageIndex(int newPageIndex);
  std::pair<int, int> GetEntryIndexBE(int totalLen) const;

  inline int GetPageCnt() const {
    int N = GetEntryListLen();
    return N / SCENES_CNT_1_PAGE + int(N % SCENES_CNT_1_PAGE != 0);
  }

  inline const SCENES_TYPE& GetEntryList() const { return mFilterEnable ? mEntryListFiltered : mEntryList; }
  inline int GetEntryListLen() const { return GetEntryList().size(); }
  void setFilterRegularExpression(const QString& pattern);

public slots:
  void onIconSizeChange(const QSize& newSize);

private:
  int mPageIndex{0};
  int SCENES_CNT_1_PAGE{12};  // 4-by-3
  bool mFilterEnable{false};
  QString mPattern;
  QString mRootPath;
  SCENES_TYPE mEntryList;
  SCENES_TYPE mEntryListFiltered;
  SCENES_TYPE::const_iterator mCurBegin{nullptr}, mCurEnd{nullptr};

  QPixmapCache mPixCache;
  int mWidth = 404, mHeight = 250;
};
#endif  // SCENESLISTMODEL_H
