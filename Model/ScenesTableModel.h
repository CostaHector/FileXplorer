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

  QFileInfo fileInfo(const QModelIndex& index) const;
  void setRootPath(const QString& rootPath);

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
};
#endif  // SCENESTABLEMODEL_H
