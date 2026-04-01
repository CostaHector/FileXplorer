#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "PaginatedList.h"
#include "EndToExposePrivateMember.h"

extern template class PaginatedList<int>;
using PaginatedIntList = PaginatedList<int>;
// using TBeforeDataResetCallable = std::function<void()>;
// using TAfterDataResetCallable = std::function<void()>;
// using TEmitPageCntChangedCallable = std::function<void(int)>;

namespace {
int m_TBeforeDataResetCallableCallTime{0};
int m_TAfterDataResetCallableCallTime{0};
int m_TEmitPageCntChangedCallableCallTime{0};
void initCallTime() {
  m_TBeforeDataResetCallableCallTime = 0;
  m_TAfterDataResetCallableCallTime = 0;
  m_TEmitPageCntChangedCallableCallTime = 0;
}
void BeforeDataResetCallable() {
  ++m_TBeforeDataResetCallableCallTime;
}
void AfterDataResetCallable() {
  ++m_TAfterDataResetCallableCallTime;
}
void EmitPageCntChangedCallable(int newPageCnt) {
  ++m_TEmitPageCntChangedCallableCallTime;
}

bool lessInt(const int& lhs, const int& rhs) {
  return lhs < rhs;
}
bool greaterInt(const int& lhs, const int& rhs) {
  return lhs > rhs;
}
}  // namespace

class PaginatedListTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private:
 private slots:
  void init() { initCallTime(); }

  void default_behavior_ok() {
    PaginatedIntList lst;
    QVERIFY(!lst.mBeforeDataResetFunc);
    QVERIFY(!lst.mAfterDataResetFunc);
    QVERIFY(!lst.mEmitPageCntChangedFunc);
    int localInd{-1};
    QCOMPARE(lst.isLocalIndexValid({}, localInd), false);

    // default ok
    lst.initPerPageCnt(1000);
    QCOMPARE(lst.GetGlbEleCnt(), 0);
    QCOMPARE(lst.GetPageCnt(), 0);
    QCOMPARE(lst.GetPerPageEleCnt(), 1000);
    QCOMPARE(lst.GetCurPageIndex(), 0);
    QCOMPARE(lst.GetCurPageStart(), 0);
    QCOMPARE(lst.GetCurPageEnd(), 0);
    QCOMPARE(lst.GetEntryIndexBE(1000, 0, 0), std::make_pair(0, 0));
    QCOMPARE(lst.constBeginCurPage(), lst.mDataList.cbegin() + 0);
    QCOMPARE(lst.constEndCurPage(), lst.mDataList.cbegin() + 0);

    QCOMPARE(lst.setCurPageIndex(-1), false);
    QVERIFY(lst.m_sorter == nullptr);
    QCOMPARE(lst.m_sortResultReverse, false);
    QCOMPARE(lst.setSorter(nullptr), false);
    QCOMPARE(lst.setSortResultReverse(false), false);  // unchange
  }

  void setData_no_register_ok() {
    PaginatedIntList lst;
    lst.initPerPageCnt(3);
    lst.setData({});
    lst.setData({0, 1, 2, 3, 4, 5, 6, 7, 8});
    QCOMPARE(lst.GetPageCnt(), 3);  // ceil(9/3)

    lst.setPerPageEleCnt(1);
    QCOMPARE(lst.GetPageCnt(), 9);  // ceil(9/1)

    {
      lst.setPerPageEleCnt(5);
      QCOMPARE(lst.GetPageCnt(), 2);  // ceil(9/5)
      if (lst.mBeforeDataResetFunc) {
        lst.mBeforeDataResetFunc();
      }
      const auto eraser = lst.GetRangeEraser();
      eraser(0, 5);  // removed:[0,1,2,3,4], left:[5,6,7,8]
      if (lst.mAfterDataResetFunc) {
        lst.mAfterDataResetFunc();
      }
    }
    QCOMPARE(lst.GetGlbEleCnt(), 9 - 5);  // 4
    QCOMPARE(lst.GetPageCnt(), 1);        // ceil((9-5)/5)
    QCOMPARE(lst.GetCurPageStart(), 0);
    QCOMPARE(lst.GetCurPageEnd(), 4);
    QCOMPARE(lst.constBeginCurPage(), lst.mDataList.cbegin() + 0);
    QCOMPARE(lst.constEndCurPage(), lst.mDataList.cbegin() + 4);

    QCOMPARE(m_TBeforeDataResetCallableCallTime, 0);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 0);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);
  }

  void setData_register_ok() {
    PaginatedIntList lst;
    lst.initPerPageCnt(3);
    lst.registerCallback(&BeforeDataResetCallable, &AfterDataResetCallable, &EmitPageCntChangedCallable);

    lst.setData({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
    QCOMPARE(lst.GetGlbEleCnt(), 10);
    QCOMPARE(lst.GetPageCnt(), 4);  // ceil(10/3)
    QCOMPARE(lst.GetPerPageEleCnt(), 3);
    QCOMPARE(lst.GetCurPageIndex(), 0);
    QCOMPARE(lst.GetCurPageStart(), 0);
    QCOMPARE(lst.GetCurPageEnd(), 3);
    QCOMPARE(lst.GetEntryIndexBE(3, 0, 10), std::make_pair(0, 3));
    QCOMPARE(lst.GetEntryIndexBE(-1, 0, 10), std::make_pair(0, 10));  // perPage < 0
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);  // 0->4
    initCallTime();

    lst.setData({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14});
    QCOMPARE(lst.GetPageCnt(), 5);  // ceil(15/3)
    QCOMPARE(lst.GetCurPageStart(), 0);
    QCOMPARE(lst.GetCurPageEnd(), 3);
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);  // 4->6
    initCallTime();

    lst.setData({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13});
    QCOMPARE(lst.GetPageCnt(), 5);  // ceil(14/3)
    QCOMPARE(lst.GetCurPageStart(), 0);
    QCOMPARE(lst.GetCurPageEnd(), 3);
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);  // 5->5
    initCallTime();

    lst.setData({0, 1});
    QCOMPARE(lst.GetPageCnt(), 1);  // ceil(2/3)
    QCOMPARE(lst.GetCurPageStart(), 0);
    QCOMPARE(lst.GetCurPageEnd(), 2);
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);  // 5->1
    initCallTime();

    lst.setData({});
    QCOMPARE(lst.GetPageCnt(), 0);
    QCOMPARE(lst.GetCurPageStart(), 0);
    QCOMPARE(lst.GetCurPageEnd(), 0);
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);  // 1->0
    initCallTime();
  }

  void setPageIndex_ok() {
    PaginatedIntList lst;
    lst.initPerPageCnt(6);
    lst.registerCallback(&BeforeDataResetCallable, &AfterDataResetCallable, &EmitPageCntChangedCallable);

    lst.setData({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
    QCOMPARE(lst.GetPageCnt(), 2);  // ceil(10/6)
    QCOMPARE(lst.GetCurPageIndex(), 0);
    QCOMPARE(lst.GetCurPageStart(), 0);
    QCOMPARE(lst.GetCurPageEnd(), 6);
    QCOMPARE(lst.GetLocalEleCnt(), 6);
    initCallTime();

    QCOMPARE(lst.setCurPageIndex(1), true);
    QCOMPARE(lst.GetCurPageStart(), 6);
    QCOMPARE(lst.GetCurPageEnd(), 10);
    QCOMPARE(lst.GetLocalEleCnt(), 4);
    QCOMPARE(lst.GetCurPageIndex(), 1);
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);
    initCallTime();

    QCOMPARE(lst.setCurPageIndex(1), false);  // page index unchange
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 0);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 0);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);
    initCallTime();

    QCOMPARE(lst.setCurPageIndex(-1), false);  // page index invalid
    QCOMPARE(lst.GetCurPageIndex(), 1);        // still former page index
    QCOMPARE(lst.GetLocalEleCnt(), 4);
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 0);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 0);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);
    initCallTime();

    QCOMPARE(lst.setCurPageIndex(999), true);  // page index out of range, let it show nothing(after end)
    QCOMPARE(lst.GetCurPageStart(), 10);
    QCOMPARE(lst.GetCurPageEnd(), 10);
    QCOMPARE(lst.GetLocalEleCnt(), 0);
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);
    initCallTime();
  }

  void setPerPageEleCnt_ok() {
    PaginatedIntList lst;
    lst.initPerPageCnt(3);
    lst.registerCallback(&BeforeDataResetCallable, &AfterDataResetCallable, &EmitPageCntChangedCallable);

    lst.setData({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});  // ceil(10/3)
    QCOMPARE(lst.GetPageCnt(), 4);
    initCallTime();

    QCOMPARE(lst.setPerPageEleCnt(5), true);
    QCOMPARE(lst.GetPageCnt(), 2);  // ceil(10/5)
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);  // 4 -> 2
    initCallTime();

    QCOMPARE(lst.setPerPageEleCnt(5), false);  // again
    QCOMPARE(lst.GetPageCnt(), 2);
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 0);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 0);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);  // 2 -> 2
    initCallTime();

    QCOMPARE(lst.setPerPageEleCnt(999), true);
    QCOMPARE(lst.GetPageCnt(), 1);  // ceil(10/999)
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);  // 2 -> 1
    initCallTime();

    QCOMPARE(lst.setPerPageEleCnt(-1), true);
    QCOMPARE(lst.GetPageCnt(), 1);  // ceil(10/10)
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);  // 1 -> 1
    initCallTime();

    QCOMPARE(lst.setPerPageEleCnt(0), true);
    QCOMPARE(lst.GetPageCnt(), 0);
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);  // 1 -> 0
    initCallTime();
  }

  void rangeEraser_onFirstPage_ok() {
    PaginatedIntList lst;
    lst.initPerPageCnt(6);
    lst.registerCallback(&BeforeDataResetCallable, &AfterDataResetCallable, &EmitPageCntChangedCallable);

    lst.setData({0, 1, 2, 3, 4, 5, 6, 7});
    QCOMPARE(lst.GetPageCnt(), 2);  // ceil(8/6)
    QCOMPARE(lst.GetCurPageIndex(), 0);
    QCOMPARE(lst.GetLocalEleCnt(), 6);
    QCOMPARE(lst[0], 0);
    QCOMPARE(lst[1], 1);
    QCOMPARE(lst[2], 2);
    QCOMPARE(lst[3], 3);
    QCOMPARE(lst[4], 4);
    QCOMPARE(lst[5], 5);

    initCallTime();
    {
      const auto eraser = lst.GetRangeEraser();
      // mock call erase manually
      lst.mBeforeDataResetFunc();
      // removed:[1,2,4], left:[0,3,5,6,7]
      eraser(4, 5);
      eraser(2, 3);
      eraser(1, 2);
      lst.mAfterDataResetFunc();
    }
    QCOMPARE(lst.GetGlbEleCnt(), 5);  // 5
    QCOMPARE(lst.GetPageCnt(), 1);    // ceil(5/6)
    QCOMPARE(lst.GetCurPageStart(), 0);
    QCOMPARE(lst.GetCurPageEnd(), 5);
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);  // 2 -> 1
    initCallTime();

    // data should still correct
    QCOMPARE(lst[0], 0);
    QCOMPARE(lst[1], 3);
    QCOMPARE(lst[2], 5);
    QCOMPARE(lst[3], 6);
    QCOMPARE(lst[4], 7);
  }

  void rangeEraser_onSecondPage_ok() {
    PaginatedIntList lst;
    lst.initPerPageCnt(6);
    lst.registerCallback(&BeforeDataResetCallable, &AfterDataResetCallable, &EmitPageCntChangedCallable);

    lst.setData({0, 1, 2, 3, 4, 5, 6, 7});
    QCOMPARE(lst.GetGlbEleCnt(), 8);
    QCOMPARE(lst.GetPageCnt(), 2);  // ceil(8/6)
    QCOMPARE(lst.GetCurPageIndex(), 0);
    QCOMPARE(lst.GetLocalEleCnt(), 6);

    QCOMPARE(lst.setCurPageIndex(1), true);
    QCOMPARE(lst.GetLocalEleCnt(), 2);
    QCOMPARE(lst.GetCurPageIndex(), 1);
    QCOMPARE(lst.GetLocalEleCnt(), 2);
    QCOMPARE(lst.constBeginCurPage(), lst.mDataList.cbegin() + 6);
    QCOMPARE(lst.constEndCurPage(), lst.mDataList.cbegin() + 8);

    // 2 operator[] should correct
    const PaginatedIntList& constRefLst = lst;
    QCOMPARE(lst[0], 6);
    QCOMPARE(lst[1], 7);
    QCOMPARE(constRefLst[0], 6);
    QCOMPARE(constRefLst[1], 7);
    initCallTime();

    {
      const auto eraser = lst.GetRangeEraser();
      // mock call erase manually
      lst.mBeforeDataResetFunc();
      eraser(0, 1);  // removed:[6], left:[0,1,2,3,4,5,7]
      lst.mAfterDataResetFunc();
    }
    QCOMPARE(lst.GetGlbEleCnt(), 7);  // 8-1
    QCOMPARE(lst.GetLocalEleCnt(), 1);
    QCOMPARE(lst.GetPageCnt(), 2);  // ceil(7/6)
    QCOMPARE(lst.GetCurPageIndex(), 1);
    QCOMPARE(lst.GetCurPageStart(), 6);
    QCOMPARE(lst.GetCurPageEnd(), 7);
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);  // 2 -> 2
    initCallTime();

    // page2: data should still correct
    QCOMPARE(lst[0], 7);

    // remove last element in page 2
    {
      const auto eraser = lst.GetRangeEraser();
      // mock call erase manually
      lst.mBeforeDataResetFunc();
      eraser(0, 1);  // removed:[7], left:[0,1,2,3,4,5]
      lst.mAfterDataResetFunc();
    }
    QCOMPARE(lst.GetGlbEleCnt(), 6);  // 7-1
    QCOMPARE(lst.GetLocalEleCnt(), 0);
    QCOMPARE(lst.GetPageCnt(), 1);       // ceil(6/6)
    QCOMPARE(lst.GetCurPageIndex(), 1);  // 不会修改当前页码
    QCOMPARE(lst.GetCurPageStart(), 6);
    QCOMPARE(lst.GetCurPageEnd(), 6);
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);  // 2 -> 1
    initCallTime();

    // page1: data should still correct
    QCOMPARE(lst.setCurPageIndex(0), true);
    QCOMPARE(lst.GetLocalEleCnt(), 6);
    QCOMPARE(lst[0], 0);
    QCOMPARE(lst[1], 1);
    QCOMPARE(lst[2], 2);
    QCOMPARE(lst[3], 3);
    QCOMPARE(lst[4], 4);
    QCOMPARE(lst[5], 5);
  }

  void rangeListErase_ok() {
    PaginatedIntList lst;
    lst.initPerPageCnt(6);
    lst.registerCallback(&BeforeDataResetCallable, &AfterDataResetCallable, &EmitPageCntChangedCallable);

    lst.setData({0, 1, 2, 3, 4, 5, 6, 7});  // ceil(8/6) = 2
    QCOMPARE(lst.GetPageCnt(), 2);
    QCOMPARE(lst.mCurPageStart, 0);
    QCOMPARE(lst.mCurPageEnd, 6);
    initCallTime();

    auto rngLstEraser = lst.GetRangeListEraser();
    QList<std::pair<int, int>> frontBackList{{0, 2}, {3, 5}};
    // 删除 0, 1, 2, 3, 4, 5, page:0 剩下6, 7
    lst.mBeforeDataResetFunc();
    rngLstEraser(frontBackList);
    lst.mAfterDataResetFunc();

    QCOMPARE(lst.GetPageCnt(), 1);
    QCOMPARE(lst.GetLocalEleCnt(), 2);
    QCOMPARE(lst.mCurPageStart, 0);
    QCOMPARE(lst.mCurPageEnd, 2);

    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);
    initCallTime();
  }

  void initSortSetting_ok() {
    {  // no sort pointer, globally, registered
      PaginatedIntList lst;
      lst.registerCallback(&BeforeDataResetCallable, &AfterDataResetCallable, &EmitPageCntChangedCallable);
      QVERIFY(lst.m_sorter == nullptr);
      QCOMPARE(lst.m_sortResultReverse, false);
      lst.initPerPageCnt(1000);
      lst.setData({2, 1, 3, 0, 4});
      QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
      QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
      QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);
      initCallTime();

      QCOMPARE(lst.mDataList, (QList<int>{2, 1, 3, 0, 4}));
      QCOMPARE(lst.sort(), false);
      QCOMPARE(lst.mDataList, (QList<int>{2, 1, 3, 0, 4}));

      QCOMPARE(m_TBeforeDataResetCallableCallTime, 0);
      QCOMPARE(m_TAfterDataResetCallableCallTime, 0);
      QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);
      initCallTime();
    }

    {  // sort ascending/descending only 1 page. globally, registered
      PaginatedIntList lst;
      lst.initPerPageCnt(1000);
      lst.initSortSetting(&::lessInt, false);  // less, no reverse, global
      lst.registerCallback(&BeforeDataResetCallable, &AfterDataResetCallable, &EmitPageCntChangedCallable);
      initCallTime();
      lst.setData({2, 1, 3, 0, 4});
      QCOMPARE(lst.GetPageCnt(), 1);
      QCOMPARE(lst.mDataList, (QList<int>{0, 1, 2, 3, 4}));
      QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
      QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
      QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);  // 0->1
      initCallTime();

      QCOMPARE(lst.setSortResultReverse(true), true);
      QCOMPARE(lst.mDataList, (QList<int>{4, 3, 2, 1, 0}));
      QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
      QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
      QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);  // will never call this
      initCallTime();
      QCOMPARE(lst.setSortResultReverse(true), false);  // unchange
    }
  }

  void setSorter_ok() {
    PaginatedIntList lst;
    lst.initPerPageCnt(1000);
    lst.initSortSetting(&::lessInt, true);  // less, reverse, global
    lst.registerCallback(&BeforeDataResetCallable, &AfterDataResetCallable, &EmitPageCntChangedCallable);
    lst.setData({2, 1, 3, 0, 4});
    QCOMPARE(lst.mDataList, (QList<int>{4, 3, 2, 1, 0}));
    QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
    QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
    QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 1);
    initCallTime();

    QCOMPARE(lst.setSorter(nullptr), false);  // nullptr
    QCOMPARE(lst.setSorter(lessInt), false);  // unchange

    QCOMPARE(lst.setSorter(greaterInt), true);  // greater, reverse
    QCOMPARE(lst.mDataList, (QList<int>{0, 1, 2, 3, 4}));
  }

  void setSortResultReverse_ok() {
    // only one page
    {
      PaginatedIntList lst;
      lst.initPerPageCnt(1000);
      lst.initSortSetting(nullptr, false);  // no sort, no reverse, global
      lst.registerCallback(&BeforeDataResetCallable, &AfterDataResetCallable, &EmitPageCntChangedCallable);
      lst.setData({2, 1, 3, 0, 4});
      QCOMPARE(lst.mDataList, (QList<int>{2, 1, 3, 0, 4}));
      initCallTime();

      QCOMPARE(lst.setSortResultReverse(false), false);  // unchange
      QCOMPARE(lst.mDataList, (QList<int>{2, 1, 3, 0, 4}));
      QCOMPARE(m_TBeforeDataResetCallableCallTime, 0);
      QCOMPARE(m_TAfterDataResetCallableCallTime, 0);
      QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);
      initCallTime();

      QCOMPARE(lst.setSortResultReverse(true), true);  // simply reverse result
      QCOMPARE(lst.mDataList, (QList<int>{4, 0, 3, 1, 2}));
      QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
      QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
      QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);
      initCallTime();

      QCOMPARE(lst.setSortResultReverse(false), true);  // simply reverse result again
      QCOMPARE(lst.mDataList, (QList<int>{2, 1, 3, 0, 4}));
      QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
      QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
      QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);
      initCallTime();
    }

    // 2 page global
    {
      PaginatedIntList lst;
      lst.initPerPageCnt(3);
      lst.initSortSetting(nullptr, false);  // no sort, no reverse, global
      lst.registerCallback(&BeforeDataResetCallable, &AfterDataResetCallable, &EmitPageCntChangedCallable);
      lst.setData({2, 1, 3, 0, 4});
      QCOMPARE(lst.mDataList, (QList<int>{2, 1, 3, 0, 4}));
      initCallTime();

      QCOMPARE(lst.setSortResultReverse(false), false);  // unchange
      QCOMPARE(lst.mDataList, (QList<int>{2, 1, 3, 0, 4}));
      QCOMPARE(m_TBeforeDataResetCallableCallTime, 0);
      QCOMPARE(m_TAfterDataResetCallableCallTime, 0);
      QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);
      initCallTime();

      QCOMPARE(lst.setSortResultReverse(true), true);  // simply reverse result
      QCOMPARE(lst.mDataList, (QList<int>{4, 0, 3, 1, 2}));
      QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
      QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
      QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);
      initCallTime();

      QCOMPARE(lst.setSortResultReverse(false), true);  // simply reverse result again
      QCOMPARE(lst.mDataList, (QList<int>{2, 1, 3, 0, 4}));
      QCOMPARE(m_TBeforeDataResetCallableCallTime, 1);
      QCOMPARE(m_TAfterDataResetCallableCallTime, 1);
      QCOMPARE(m_TEmitPageCntChangedCallableCallTime, 0);
      initCallTime();
    }
  }
};

#include "PaginatedListTest.moc"
REGISTER_TEST(PaginatedListTest, false)
