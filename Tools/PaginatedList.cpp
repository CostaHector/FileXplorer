#include "PaginatedList.h"
#include "PublicMacro.h"
#include "Logger.h"

template <typename SceneElementType>
void PaginatedList<SceneElementType>::setData(SceneElementTypeList newEntryList) {
  int startIndex{-1}, endIndex{-1};
  std::tie(startIndex, endIndex) = GetEntryIndexBE(mPerPageEleCnt, GetCurPageIndex(), newEntryList.size());

  const int beforePageCnt{GetPageCnt()};
  if (mBeforeDataResetFunc) {
    mBeforeDataResetFunc();
  }
  mDataList.swap(newEntryList);
  sort();
  mCurPageStart = startIndex;
  mCurPageEnd = endIndex;
  if (mAfterDataResetFunc) {
    mAfterDataResetFunc();
  }
  const int afterPageCnt{GetPageCnt()};
  if (afterPageCnt != beforePageCnt) {
    if (mEmitPageCntChangedFunc) {
      mEmitPageCntChangedFunc(afterPageCnt);
    }
  }
}

template <typename SceneElementType>
bool PaginatedList<SceneElementType>::sort() {
  if (m_sorter == nullptr) {
    return false;
  }
  auto beg{mDataList.begin()};
  auto end{mDataList.end()};
  if (m_sortDescending) {
    auto localSorter = m_sorter;
    std::sort(beg, end, [localSorter](const SceneElementType& lhs, const SceneElementType& rhs) -> bool { return localSorter(rhs, lhs); });
  } else {
    std::sort(beg, end, m_sorter);
  }
  return true;
}

// -1 means all elements in one page, > 0 means count
template <typename SceneElementType>
bool PaginatedList<SceneElementType>::setPerPageEleCnt(int newPerPage) {
  const int befPerPage{GetPerPageEleCnt()};
  if (newPerPage == befPerPage) {
    LOG_D("scenes count per page remains: %d", befPerPage);
    return false;
  }
  int startIndex{-1}, endIndex{-1};
  if (newPerPage == 0) {
    LOG_W("none in one page");
    startIndex = 0;
    endIndex = 0;
  } else if (newPerPage < 0) {
    LOG_D("all items in one page");
    startIndex = 0;
    endIndex = GetGlbEleCnt();
    newPerPage = GetGlbEleCnt();
  } else {
    LOG_D("%d items in one page", newPerPage);
    std::tie(startIndex, endIndex) = GetEntryIndexBE(newPerPage, GetCurPageIndex(), GetGlbEleCnt());
  }

  const int beforePageCnt{GetPageCnt()};
  if (mBeforeDataResetFunc) {
    mBeforeDataResetFunc();
  }
  mPerPageEleCnt = newPerPage;
  mCurPageStart = startIndex;
  mCurPageEnd = endIndex;
  if (mAfterDataResetFunc) {
    mAfterDataResetFunc();
  }
  const int afterPageCnt{GetPageCnt()};
  if (afterPageCnt != beforePageCnt) {
    if (mEmitPageCntChangedFunc) {
      mEmitPageCntChangedFunc(afterPageCnt);
    }
  }
  return true;
}

template <typename SceneElementType>
bool PaginatedList<SceneElementType>::setCurPageIndex(int newPageIndex) {
  if (newPageIndex < 0) {
    LOG_D("invalid page index[%d]", newPageIndex);
    return false;
  }
  if (newPageIndex == GetCurPageIndex()) {
    LOG_D("page index remains[%d]", newPageIndex);
    return false;
  }

  int startIndex{-1}, endIndex{-1};
  std::tie(startIndex, endIndex) = GetEntryIndexBE(GetPerPageEleCnt(), newPageIndex, GetGlbEleCnt());

  // Don't use RowsCountBeginChange, RowsCountEndChange(); here. QPixmap Image varies here
  if (mBeforeDataResetFunc) {
    mBeforeDataResetFunc();
  }
  mCurPageIndex = newPageIndex;
  mCurPageStart = startIndex;
  mCurPageEnd = endIndex;
  if (mAfterDataResetFunc) {
    mAfterDataResetFunc();
  }
  return true;
}

template <typename SceneElementType>
bool PaginatedList<SceneElementType>::setSorter(SceneElementSorter sorter) {
  if (sorter == nullptr) {
    return false;
  }
  if (m_sorter == sorter) {
    return false;  // no need
  }
  m_sorter = sorter;
  if (mBeforeDataResetFunc) {
    mBeforeDataResetFunc();
  }
  sort();
  if (mAfterDataResetFunc) {
    mAfterDataResetFunc();
  }
  return true;
}

template <typename SceneElementType>
bool PaginatedList<SceneElementType>::setSortOrder(bool bDescendingReverse) {
  if (m_sortDescending == bDescendingReverse) {
    return false;
  }
  m_sortDescending = bDescendingReverse;
  if (mBeforeDataResetFunc) {
    mBeforeDataResetFunc();
  }
  std::reverse(mDataList.begin(), mDataList.end());
  if (mAfterDataResetFunc) {
    mAfterDataResetFunc();
  }
  return true;
}

// ----------

template <typename SceneElementType>
PaginatedListRangeEraseGuard<SceneElementType>::PaginatedListRangeEraseGuard(PaginatedList<SceneElementType>* spyList)  //
    : mSpyList{spyList}, mBeforePageCnt{spyList == nullptr ? 0 : spyList->GetPageCnt()} {
  CHECK_NULLPTR_RETURN_VOID(spyList);
}

template <typename SceneElementType>
PaginatedListRangeEraseGuard<SceneElementType>::~PaginatedListRangeEraseGuard() {
  CHECK_NULLPTR_RETURN_VOID(mSpyList);
  const int afterPageCnt{mSpyList->GetPageCnt()};
  if (afterPageCnt != mBeforePageCnt) {
    if (mSpyList->mEmitPageCntChangedFunc) {
      mSpyList->mEmitPageCntChangedFunc(afterPageCnt);
    }
  }
  mSpyList->UpdatePageStartAndEndIndex();
  mSpyList = nullptr;
}

#include "SceneInfo.h"
template class PaginatedList<SceneInfo>;
template class PaginatedList<int>;
template class PaginatedListRangeEraseGuard<SceneInfo>;
template class PaginatedListRangeEraseGuard<int>;
