#ifndef PAGINATEDLIST_H
#define PAGINATEDLIST_H

#include <QList>
#include <QModelIndex>

template <typename SceneElementType>
struct PaginatedListRangeEraseGuard;

template <typename SceneElementType>
class PaginatedList {
 public:
  using TBeforeDataResetCallable = std::function<void()>;
  using TAfterDataResetCallable = std::function<void()>;
  using TEmitPageCntChangedCallable = std::function<void(int)>;
  using TRangeEraserCallback = std::function<void(int, int)>;
  friend class PaginatedListRangeEraseGuard<SceneElementType>;
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
  TRangeEraserCallback GetRangeEraser() {
    return [this](int beg, int end) {  //
      mDataList.erase(mDataList.begin() + mCurPageStart + beg, mDataList.begin() + mCurPageStart + end);
    };
  }
  bool setSorter(SceneElementSorter sorter);
  bool setSortResultReverse(bool bResultReverse);

  bool sort();
  typename SceneElementTypeList::const_iterator constBeginCurPage() const { return mDataList.cbegin() + mCurPageStart; } // used in SceneSortProxyModel

 private:
  typename SceneElementTypeList::const_iterator constEndCurPage() const { return mDataList.cbegin() + mCurPageEnd; }

  int GetGlbEleCnt() const { return mDataList.size(); }
  static std::pair<int, int> GetEntryIndexBE(const int perPage, const int pageIndex, const int elesCnt) {
    if (perPage < 0) {
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

template <typename SceneElementType>
struct PaginatedListRangeEraseGuard {
 public:
  explicit PaginatedListRangeEraseGuard(PaginatedList<SceneElementType>* spyList);
  ~PaginatedListRangeEraseGuard();

 private:
  PaginatedList<SceneElementType>* mSpyList{nullptr};
  const int mBeforePageCnt{0};
};

#endif  // PAGINATEDLIST_H
