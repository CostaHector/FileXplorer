#ifndef SCENESTABLEMODEL_H
#define SCENESTABLEMODEL_H

#include "Tools/QAbstractTableModelPub.h"
#include <utility>
#include <QSet>
#include <QFileInfo>

struct SCENE_INFO {
  QString filename;
  qint64 size;
  qint64 like;
};

class ImgCorrespondVid {
 public:
  void append(const QString& vidName, const QString& vidFullPath) {
    const int lastDot = vidName.lastIndexOf('.');
    const QString coreName{lastDot == -1 ? vidName : vidName.left(lastDot)};
    mCoreName2VidMap[coreName] = vidFullPath;
  }

  bool contains(const QString& imgPath, QString* pVidPath = nullptr) const {
    const int lastDot = imgPath.lastIndexOf('.');
    const QString coreName{lastDot == -1 ? imgPath : imgPath.left(lastDot)};
    auto it = mCoreName2VidMap.find(coreName);
    if (it == mCoreName2VidMap.cend()) {
      return false;
    }
    if (pVidPath != nullptr) {
      *pVidPath = it.value();
    }
    return true;
  }

  inline void clear() {
    decltype(mCoreName2VidMap) tmp;
    mCoreName2VidMap.swap(tmp);
  }

  inline bool size() const { return mCoreName2VidMap.size(); }

 private:
  QHash<QString, QString> mCoreName2VidMap;
};

class ScenesTableModel : public QAbstractTableModelPub {
 public:
  typedef QList<SCENE_INFO> SCENE_INFO_LIST;
  ScenesTableModel(QObject* object = nullptr);

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
  QString absolutePath(const QModelIndex& index) const;
  bool setRootPath(const QString& rootPath);

  bool ChangeRowsCnt(int newRowCnt, int newPageIndex);
  bool ChangeColumnsCnt(int newColumnCnt = 4, int newPageIndex = -1);
  void SortOrder(bool reverse = false);
  bool SetPageIndex(int newPageIndex);
  std::pair<int, int> GetEntryIndexBE(int totalLen) const;

  inline int GetPageCnt() const {
    if (mSCENES_CNT_ROW == -1) {
      return 1;
    }
    int N = GetEntryListLen();
    return N / (mSCENES_CNT_ROW * mSCENES_CNT_COLUMN) + int(N % (mSCENES_CNT_ROW * mSCENES_CNT_COLUMN) != 0);
  }

  inline const SCENE_INFO_LIST& GetEntryList() const { return mFilterEnable ? mEntryListFiltered : mEntryList; }
  inline int GetEntryListLen() const { return GetEntryList().size(); }
  void setFilterRegExp(const QString& pattern);

 private:
  int mPageIndex{-1};
  int mSCENES_CNT_COLUMN{4};
  int mSCENES_CNT_ROW{-1};
  bool mFilterEnable{false};
  QString mPattern;
  QString mRootPath;
  SCENE_INFO_LIST mEntryList;
  SCENE_INFO_LIST mEntryListFiltered;
  SCENE_INFO_LIST::const_iterator mCurBegin{nullptr}, mCurEnd{nullptr};
  ImgCorrespondVid mImg2Vid;
};
#endif  // SCENESTABLEMODEL_H
