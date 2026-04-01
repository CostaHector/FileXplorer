#include "PaginatedList.h"
#include "PublicMacro.h"
#include "Logger.h"

template <typename SceneElementType>
void PaginatedList<SceneElementType>::setData(SceneElementTypeList newEntryList) {
  const int beforePageCnt{GetPageCnt()};
  if (mBeforeDataResetFunc) {
    mBeforeDataResetFunc();
  }
  mDataList.swap(newEntryList);
  sort();
  UpdatePageStartAndEndIndex();
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
  if (m_sortResultReverse) {
    auto localSorter = m_sorter;
    std::sort(mDataList.begin(), mDataList.end(),
              [localSorter](const SceneElementType& lhs, const SceneElementType& rhs) -> bool { return localSorter(rhs, lhs); });
  } else {
    std::sort(mDataList.begin(), mDataList.end(), m_sorter);
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
  if (newPerPage == 0) {
    LOG_W("none in one page");
  } else if (newPerPage < 0) {
    LOG_D("all items in one page");
    newPerPage = GetGlbEleCnt();
  } else {
    LOG_D("%d items in one page", newPerPage);
  }

  const int beforePageCnt{GetPageCnt()};
  if (mBeforeDataResetFunc) {
    mBeforeDataResetFunc();
  }
  mPerPageEleCnt = newPerPage;
  UpdatePageStartAndEndIndex();
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

  // Don't use RowsCountBeginChange, RowsCountEndChange(); here. QPixmap Image varies here
  if (mBeforeDataResetFunc) {
    mBeforeDataResetFunc();
  }
  mCurPageIndex = newPageIndex;
  UpdatePageStartAndEndIndex();
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
bool PaginatedList<SceneElementType>::setSortResultReverse(bool bResultReverse) {
  if (m_sortResultReverse == bResultReverse) {
    return false;
  }
  m_sortResultReverse = bResultReverse;
  if (mBeforeDataResetFunc) {
    mBeforeDataResetFunc();
  }
  std::reverse(mDataList.begin(), mDataList.end());
  if (mAfterDataResetFunc) {
    mAfterDataResetFunc();
  }
  return true;
}

template <typename SceneElementType>
typename PaginatedList<SceneElementType>::TRangeEraserCallback PaginatedList<SceneElementType>::GetRangeEraser() {
  return std::bind(&PaginatedList<SceneElementType>::EraseRange, this, std::placeholders::_1, std::placeholders::_2);
}

template <typename SceneElementType>
typename PaginatedList<SceneElementType>::TRangeListEraserCallback PaginatedList<SceneElementType>::GetRangeListEraser() {
  return std::bind(&PaginatedList<SceneElementType>::EraseRangeList, this, std::placeholders::_1);
}

template <typename SceneElementType>
void PaginatedList<SceneElementType>::EraseRange(int beg, int end) {
  if (beg >= end) {
    return;
  }
  int frontRow = beg;
  int backRow = end - 1;
  QList<std::pair<int, int>> rangeLst{{frontRow, backRow}};
  EraseRangeList(rangeLst);
}

template <typename SceneElementType>
void PaginatedList<SceneElementType>::EraseRangeList(const QList<std::pair<int, int>>& rangeLst) {
  // rangeLst: {{front0, back0}, {front1, back1}}, 升序
  // 删除元素后, 需要刷新当前页的索引端点, 同时页数也可能变化, 页码保持不变
  if (rangeLst.isEmpty()) {
    return;
  }
  const int mBeforePageCnt{GetPageCnt()};
  for (auto it = rangeLst.rbegin(); it != rangeLst.rend(); ++it) {
    int beginRow = it->first;
    int endRow = it->second + 1;
    if (beginRow < endRow) {
      mDataList.erase(mDataList.begin() + mCurPageStart + beginRow, mDataList.begin() + mCurPageStart + endRow);
    }
  }
  UpdatePageStartAndEndIndex();
  const int afterPageCnt{GetPageCnt()};
  if (afterPageCnt != mBeforePageCnt) {
    if (mEmitPageCntChangedFunc) {
      mEmitPageCntChangedFunc(afterPageCnt);
    }
  }
}

#include "SceneInfo.h"
template class PaginatedList<SceneInfo>;
template class PaginatedList<int>;
