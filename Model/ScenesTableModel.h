#ifndef SCENESTABLEMODEL_H
#define SCENESTABLEMODEL_H

#include "Tools/QAbstractTableModelPub.h"
#include "Tools/SceneInfoManager.h"
#include <utility>
#include <QSet>
#include <QFileInfo>

class ScenesTableModel : public QAbstractTableModelPub {
 public:
  ScenesTableModel(QObject* object = nullptr);
  inline bool IsScnsEmpty() const { return mCurBegin == nullptr || mCurEnd == nullptr || mCurBegin == mCurEnd;}
  int rowCount(const QModelIndex& /*parent*/ = {}) const override {
    int begin{0}, end{0};
    std::tie(begin, end) = GetEntryIndexBE(GetEntryListLen());
    int scenesCnt = end - begin;
    return scenesCnt / mSCENES_CNT_COLUMN + int(scenesCnt % mSCENES_CNT_COLUMN != 0);
  }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return mSCENES_CNT_COLUMN; }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  inline bool isIndexValid(const QModelIndex& index, int* pLinearInd = nullptr) const {
    if (!index.isValid()) {
      qWarning("Invalid index");
      return false;
    }
    const int linearInd = index.row() * mSCENES_CNT_COLUMN + index.column();
    if (mCurBegin + linearInd >= mCurEnd) {
      qWarning("Invalid index(%d, %d) user input", index.row(), index.column());
      return false;
    }
    if (pLinearInd != nullptr) {
      *pLinearInd = linearInd;
    }
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

  bool ChangeRowsCnt(int newRowCnt, int newPageIndex);
  bool ChangeColumnsCnt(int newColumnCnt = 4, int newPageIndex = -1);
  bool ShowAllScenesInOnePage();

  void SortOrder(SceneInfoManager::SceneSortOption sortOption, bool reverse = false);
  bool SetPageIndex(int newPageIndex);
  std::pair<int, int> GetEntryIndexBE(int totalLen) const;

  inline int GetPageCnt() const {
    if (mSCENES_CNT_ROW == -1) {
      return 1;
    }
    int N = GetEntryListLen();
    return N / (mSCENES_CNT_ROW * mSCENES_CNT_COLUMN) + int(N % (mSCENES_CNT_ROW * mSCENES_CNT_COLUMN) != 0);
  }

  inline const SCENES_TYPE& GetEntryList() const { return mFilterEnable ? mEntryListFiltered : mEntryList; }
  inline int GetEntryListLen() const { return GetEntryList().size(); }
  void setFilterRegularExpression(const QString& pattern);

 private:
  inline int toLinearIndex(const QModelIndex& index) const {
    return index.row() * mSCENES_CNT_COLUMN + index.column();
  }
  int mPageIndex{-1};
  int mSCENES_CNT_COLUMN{4};
  int mSCENES_CNT_ROW{-1};
  bool mFilterEnable{false};
  QString mPattern;
  QString mRootPath;
  SCENES_TYPE mEntryList;
  SCENES_TYPE mEntryListFiltered;
  SCENES_TYPE::const_iterator mCurBegin{nullptr}, mCurEnd{nullptr};
};
#endif  // SCENESTABLEMODEL_H
