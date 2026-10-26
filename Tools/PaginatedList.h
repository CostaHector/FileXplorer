#ifndef PAGINATEDLIST_H
#define PAGINATEDLIST_H

#include <QList>
#include <QModelIndex>

template <typename SceneElementType>
class PaginatedList {
 public:
  using TBeforeDataResetCallable = std::function<void()>;
  using TAfterDataResetCallable = std::function<void()>;
  using TEmitPageCntChangedCallable = std::function<void(int)>;
  using TRangeEraserCallback = std::function<void(int, int)>;
  using TRangeListEraserCallback = std::function<void(const QList<std::pair<int, int>>&)>;
  using SceneElementTypeList = QList<SceneElementType>;
  using SceneElementSorter = bool(*)(const SceneElementType&, const SceneElementType&);

  void initPerPageCnt(int perPageCnt) { mPerPageEleCnt = perPageCnt; }
  void initSortSetting(SceneElementSorter sorter, bool bDescending) {
    m_sorter = sorter;
    m_sortResultReverse = bDescending;
  }
  void registerCallback(TBeforeDataResetCallable befFunc, TAfterDataResetCallable aftFunc, TEmitPageCntChangedCallable emitFunc) {
    mBeforeDataResetFunc = befFunc;
    mAfterDataResetFunc = aftFunc;
    mEmitPageCntChangedFunc = emitFunc;
  }

  void setData(SceneElementTypeList newEntryList);
  bool setPerPageEleCnt(int newPerPage);
  bool setCurPageIndex(int newPageIndex);
  const SceneElementType& operator[](int localIndex) const { return constBeginCurPage()[localIndex]; }
  SceneElementType& operator[](int localIndex) { return mDataList[toGlobalIndex(localIndex)]; }
  bool isLocalIndexValid(const QModelIndex& localIndex, int& localInd) const {
    if (!localIndex.isValid()) {
      return false;
    }
    localInd = localIndex.row();
    if (localInd >= mCurPageEnd - mCurPageStart) {
      return false;
    }
    return true;
  }
  int toGlobalIndex(int localInd) const { return mCurPageStart + localInd; }
  int GetLocalEleCnt() const { return mCurPageEnd - mCurPageStart; }
  void UpdatePageStartAndEndIndex() { std::tie(mCurPageStart, mCurPageEnd) = GetEntryIndexBE(GetPerPageEleCnt(), GetCurPageIndex(), GetGlbEleCnt()); }
  bool setSorter(SceneElementSorter sorter);
  bool setSortResultReverse(bool bResultReverse);

  bool sort();
  typename SceneElementTypeList::const_iterator constBeginCurPage() const { return mDataList.cbegin() + mCurPageStart; } // used in SceneSortProxyModel

  TRangeEraserCallback GetRangeEraser();
  TRangeListEraserCallback GetRangeListEraser();

 private:
  typename SceneElementTypeList::const_iterator constEndCurPage() const { return mDataList.cbegin() + mCurPageEnd; }

  int GetGlbEleCnt() const { return mDataList.size(); }
  static std::pair<int, int> GetEntryIndexBE(const int perPage, const int pageIndex, const int elesCnt) {
    if (perPage <= 0) {
      return std::make_pair(0, elesCnt);
    }
    const int begin = perPage * pageIndex;
    const int end = perPage * (pageIndex + 1);
    return std::make_pair(std::min(begin, elesCnt), std::min(end, elesCnt));
  }

  // pageCnt = floor(globalEleCnt / perPageEleCnt) = trunc result((globalEleCnt + perPageEleCnt - 1) / perPageEleCnt)
  int GetPageCnt() const { return mPerPageEleCnt <= 0 ? 0 : (GetGlbEleCnt() + mPerPageEleCnt - 1) / mPerPageEleCnt; }
  int GetPerPageEleCnt() const { return mPerPageEleCnt; }
  int GetCurPageIndex() const { return mCurPageIndex; }
  int GetCurPageStart() const { return mCurPageStart; }
  int GetCurPageEnd() const { return mCurPageEnd; }
  void EraseRange(int beg, int end); // {begin, end}
  void EraseRangeList(const QList<std::pair<int, int>>& rangeLst); // {front, back}

  SceneElementTypeList mDataList;
  int mPerPageEleCnt{40};
  int mCurPageIndex{0};
  int mCurPageStart{0}, mCurPageEnd{0};
  TBeforeDataResetCallable mBeforeDataResetFunc;
  TAfterDataResetCallable mAfterDataResetFunc;
  TEmitPageCntChangedCallable mEmitPageCntChangedFunc;

  SceneElementSorter m_sorter{nullptr};
  bool m_sortResultReverse{false}; // aka reverse order
};

#endif  // PAGINATEDLIST_H
