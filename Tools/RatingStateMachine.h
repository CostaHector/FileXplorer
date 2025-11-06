#ifndef RATINGSTATEMACHINE_H
#define RATINGSTATEMACHINE_H

#include "RateHelper.h"
#include <QModelIndex>
#include <QPersistentModelIndex>

enum class RatingState {
  IDLE,           // 初始状态：无选中项
  SELECTED_SHOW,  // 选中且显示评分条
  SELECTED_HIDE,  // 选中但隐藏评分条
  BUTT,
};

enum class RatingAction {
  CLICK_SAME_ONE_RATE_BAR_AGAIN,      // 再次点击同一项的评分条区域
  CLICK_SAME_ONE_NOT_RATE_BAR_AGAIN,  // 再次点击同一项的评分条区域
  CLICK_OTHER_RATE_BAR,               // 点击其他项的评分条区域
  CLICK_OTHER_NOT_RATE_BAR,           // 点击其他项的非评分条区域
  BUTT,
};

struct RateData {
  explicit RateData(QModelIndex _curIndex = {},
           RatingState _state = RatingState::IDLE,
           int _oldRateValue = RateHelper::MOVIE_RATE_VALUE::MIN_V,
           int _newRateValue = RateHelper::MOVIE_RATE_VALUE::MIN_V)
      : curIndex{_curIndex}, mState{_state}, oldRateValue{_oldRateValue}, newRateValue{_newRateValue} {}

  operator bool() const { return curIndex.isValid(); }
  void invalidate() {
    curIndex = QPersistentModelIndex{};
    mState = RatingState::IDLE;
    oldRateValue = newRateValue = RateHelper::MOVIE_RATE_VALUE::MIN_V;
  }
  bool isInit() const {
    return !curIndex.isValid() && mState == RatingState::IDLE && oldRateValue == RateHelper::MOVIE_RATE_VALUE::MIN_V &&
           newRateValue == RateHelper::MOVIE_RATE_VALUE::MIN_V;
  }
  bool operator==(const RateData& rhs) const {
    return curIndex == rhs.curIndex && mState == rhs.mState && oldRateValue == rhs.oldRateValue && newRateValue == rhs.newRateValue;
  }
  QPersistentModelIndex curIndex;
  RatingState mState;
  int oldRateValue;
  int newRateValue;
};

class SceneStyleDelegate;

class RatingStateMachine {
 public:
  explicit RatingStateMachine();

  RatingState status() const { return mRateData.mState; }
  QModelIndex curIndex() const { return mRateData.curIndex; }
  int oldRate() const { return mRateData.oldRateValue; }
  int newRate() const { return mRateData.newRateValue; }

  void DoStateTransition(SceneStyleDelegate& delegate, const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt);

  using F_STATE_TRANSIT = void (RatingStateMachine::*)(SceneStyleDelegate& delegate,
                                                       const QModelIndex& nowInd,
                                                       const QRect& visualRect,
                                                       const QPoint& clickedPnt);
  F_STATE_TRANSIT mTransitionTable[(int)RatingState::BUTT][(int)RatingAction::BUTT]{nullptr};
  void PlaceHolderTransit(SceneStyleDelegate& delegate, const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt);

 private:
  RatingAction DetermineAction(const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt) const;

  // IDLE->
  void ClickOtherRateBarWhenIdle(SceneStyleDelegate& delegate, const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt);
  void ClickOtherNotRateBarWhenIdle(SceneStyleDelegate& delegate, const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt);

  // SELECTED_SHOW->
  void ClickSameOneRateBarWhenShow(SceneStyleDelegate& delegate, const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt);
  void ClickSameOneNotRateBarWhenShow(SceneStyleDelegate& delegate, const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt);
  void ClickOtherOneRateBarWhenShow(SceneStyleDelegate& delegate, const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt);
  void ClickOtherOneNotRateBarWhenShow(SceneStyleDelegate& delegate, const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt);

  // SELECT_SHOW->
  void ClickSameOneRateBarWhenHide(SceneStyleDelegate& delegate, const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt);
  void ClickSameOneNotRateBarWhenHide(SceneStyleDelegate& delegate, const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt);
  void ClickOtherOneRateBarWhenHide(SceneStyleDelegate& delegate, const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt);
  void ClickOtherOneNotRateBarWhenHide(SceneStyleDelegate& delegate, const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt);

  RateData mRateData;
};

#endif  // RATINGSTATEMACHINE_H
