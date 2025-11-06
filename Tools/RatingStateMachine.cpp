#include "RatingStateMachine.h"
#include "SceneStyleDelegate.h"
#include "ScenesListModel.h"
#include "SceneInfo.h"
#include "Logger.h"

RatingStateMachine::RatingStateMachine() {
  mTransitionTable[(int)RatingState::IDLE][(int)RatingAction::CLICK_SAME_ONE_RATE_BAR_AGAIN] = &RatingStateMachine::PlaceHolderTransit;
  mTransitionTable[(int)RatingState::IDLE][(int)RatingAction::CLICK_SAME_ONE_NOT_RATE_BAR_AGAIN] = &RatingStateMachine::PlaceHolderTransit;
  mTransitionTable[(int)RatingState::IDLE][(int)RatingAction::CLICK_OTHER_RATE_BAR] = &RatingStateMachine::ClickOtherRateBarWhenIdle;
  mTransitionTable[(int)RatingState::IDLE][(int)RatingAction::CLICK_OTHER_NOT_RATE_BAR] = &RatingStateMachine::ClickOtherNotRateBarWhenIdle;

  mTransitionTable[(int)RatingState::SELECTED_SHOW][(int)RatingAction::CLICK_SAME_ONE_RATE_BAR_AGAIN] =
      &RatingStateMachine::ClickSameOneRateBarWhenShow;
  mTransitionTable[(int)RatingState::SELECTED_SHOW][(int)RatingAction::CLICK_SAME_ONE_NOT_RATE_BAR_AGAIN] =
      &RatingStateMachine::ClickSameOneNotRateBarWhenShow;
  mTransitionTable[(int)RatingState::SELECTED_SHOW][(int)RatingAction::CLICK_OTHER_RATE_BAR] = &RatingStateMachine::ClickOtherOneRateBarWhenShow;
  mTransitionTable[(int)RatingState::SELECTED_SHOW][(int)RatingAction::CLICK_OTHER_NOT_RATE_BAR] =
      &RatingStateMachine::ClickOtherOneNotRateBarWhenShow;

  mTransitionTable[(int)RatingState::SELECTED_HIDE][(int)RatingAction::CLICK_SAME_ONE_RATE_BAR_AGAIN] =
      &RatingStateMachine::ClickSameOneRateBarWhenHide;
  mTransitionTable[(int)RatingState::SELECTED_HIDE][(int)RatingAction::CLICK_SAME_ONE_NOT_RATE_BAR_AGAIN] =
      &RatingStateMachine::ClickSameOneNotRateBarWhenHide;
  mTransitionTable[(int)RatingState::SELECTED_HIDE][(int)RatingAction::CLICK_OTHER_RATE_BAR] = &RatingStateMachine::ClickOtherOneRateBarWhenHide;
  mTransitionTable[(int)RatingState::SELECTED_HIDE][(int)RatingAction::CLICK_OTHER_NOT_RATE_BAR] =
      &RatingStateMachine::ClickOtherOneNotRateBarWhenHide;
}

RatingAction RatingStateMachine::DetermineAction(const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt) const {
  const bool isInsideRatingBar = RateHelper::isClickPointInsideRatingBar(clickedPnt, visualRect);
  if (status() == RatingState::IDLE) {
    return isInsideRatingBar ? RatingAction::CLICK_OTHER_RATE_BAR : RatingAction::CLICK_OTHER_NOT_RATE_BAR;
  }

  if (nowInd == curIndex()) {
    return isInsideRatingBar ? RatingAction::CLICK_SAME_ONE_RATE_BAR_AGAIN : RatingAction::CLICK_SAME_ONE_NOT_RATE_BAR_AGAIN;
  } else {
    return isInsideRatingBar ? RatingAction::CLICK_OTHER_RATE_BAR : RatingAction::CLICK_OTHER_NOT_RATE_BAR;
  }
}

void RatingStateMachine::DoStateTransition(SceneStyleDelegate& delegate,
                                           const QModelIndex& nowInd,
                                           const QRect& visualRect,
                                           const QPoint& clickedPnt) {
  RatingAction rateAct = DetermineAction(nowInd, visualRect, clickedPnt);
  auto transiFunc = mTransitionTable[(int)status()][(int)rateAct];
  CHECK_NULLPTR_RETURN_VOID(transiFunc);
  (this->*transiFunc)(delegate, nowInd, visualRect, clickedPnt);
}

void RatingStateMachine::PlaceHolderTransit(SceneStyleDelegate& delegate,
                                            const QModelIndex& nowInd,
                                            const QRect& visualRect,
                                            const QPoint& clickedPnt) {
  LOG_W("Should not run into here");
  return;
}

void RatingStateMachine::ClickOtherRateBarWhenIdle(SceneStyleDelegate& delegate,
                                                   const QModelIndex& nowInd,
                                                   const QRect& visualRect,
                                                   const QPoint& clickedPnt) {
  const int oldRateValue = nowInd.data(ScenesListModel::RatingRole).toInt();

  mRateData.curIndex = nowInd;
  mRateData.mState = RatingState::SELECTED_SHOW;
  mRateData.oldRateValue = oldRateValue;
  mRateData.newRateValue = oldRateValue;

  emit delegate.cellVisualUpdateRequested(nowInd);
}

void RatingStateMachine::ClickOtherNotRateBarWhenIdle(SceneStyleDelegate& delegate,
                                                      const QModelIndex& nowInd,
                                                      const QRect& visualRect,
                                                      const QPoint& clickedPnt) {
  const int oldRateValue = nowInd.data(ScenesListModel::RatingRole).toInt();

  mRateData.curIndex = nowInd;
  mRateData.mState = RatingState::SELECTED_HIDE;
  mRateData.oldRateValue = oldRateValue;
  mRateData.newRateValue = oldRateValue;
}

void RatingStateMachine::ClickSameOneRateBarWhenShow(SceneStyleDelegate& delegate,
                                                     const QModelIndex& nowInd,
                                                     const QRect& visualRect,
                                                     const QPoint& clickedPnt) {
  mRateData.newRateValue = RateHelper::ratingAtPosition(clickedPnt, visualRect);
  if (mRateData.newRateValue != mRateData.oldRateValue) {  // 评分真实改变了
    // 去修改json/scn 二进制
    if (QAbstractItemModel* model = const_cast<QAbstractItemModel*>(nowInd.model())) {
      bool bResult = model->setData(nowInd, mRateData.newRateValue, ScenesListModel::RatingRole);
    }
  }
  emit delegate.cellVisualUpdateRequested(nowInd);
}
void RatingStateMachine::ClickSameOneNotRateBarWhenShow(SceneStyleDelegate& delegate,
                                                        const QModelIndex& nowInd,
                                                        const QRect& visualRect,
                                                        const QPoint& clickedPnt) {
  const int oldRateValue = nowInd.data(ScenesListModel::RatingRole).toInt();
  mRateData.oldRateValue = oldRateValue;
  mRateData.newRateValue = oldRateValue;

  mRateData.mState = RatingState::SELECTED_HIDE;
  emit delegate.cellVisualUpdateRequested(nowInd);
}

void RatingStateMachine::ClickOtherOneRateBarWhenShow(SceneStyleDelegate& delegate,
                                                      const QModelIndex& nowInd,
                                                      const QRect& visualRect,
                                                      const QPoint& clickedPnt) {
  const int oldRateValue = nowInd.data(ScenesListModel::RatingRole).toInt();

  mRateData.oldRateValue = oldRateValue;
  mRateData.newRateValue = oldRateValue;
  mRateData.mState = RatingState::SELECTED_HIDE;
  emit delegate.cellVisualUpdateRequested(mRateData.curIndex);

  mRateData.curIndex = nowInd;
  mRateData.mState = RatingState::SELECTED_SHOW;
  emit delegate.cellVisualUpdateRequested(nowInd);
}

void RatingStateMachine::ClickOtherOneNotRateBarWhenShow(SceneStyleDelegate& delegate,
                                                         const QModelIndex& nowInd,
                                                         const QRect& visualRect,
                                                         const QPoint& clickedPnt) {
  const int oldRateValue = nowInd.data(ScenesListModel::RatingRole).toInt();

  mRateData.oldRateValue = oldRateValue;
  mRateData.newRateValue = oldRateValue;
  mRateData.mState = RatingState::SELECTED_HIDE;
  emit delegate.cellVisualUpdateRequested(mRateData.curIndex);

  mRateData.curIndex = nowInd;
}

void RatingStateMachine::ClickSameOneRateBarWhenHide(SceneStyleDelegate& delegate,
                                                     const QModelIndex& nowInd,
                                                     const QRect& visualRect,
                                                     const QPoint& clickedPnt) {
  mRateData.mState = RatingState::SELECTED_SHOW;
  emit delegate.cellVisualUpdateRequested(mRateData.curIndex);
}

void RatingStateMachine::ClickSameOneNotRateBarWhenHide(SceneStyleDelegate& delegate,
                                                        const QModelIndex& nowInd,
                                                        const QRect& visualRect,
                                                        const QPoint& clickedPnt) {
  // do nothing here
}

void RatingStateMachine::ClickOtherOneRateBarWhenHide(SceneStyleDelegate& delegate,
                                                      const QModelIndex& nowInd,
                                                      const QRect& visualRect,
                                                      const QPoint& clickedPnt) {
  const int oldRateValue = nowInd.data(ScenesListModel::RatingRole).toInt();

  mRateData.oldRateValue = oldRateValue;
  mRateData.newRateValue = oldRateValue;
  mRateData.curIndex = nowInd;
  mRateData.mState = RatingState::SELECTED_SHOW;
  emit delegate.cellVisualUpdateRequested(nowInd);
}

void RatingStateMachine::ClickOtherOneNotRateBarWhenHide(SceneStyleDelegate& delegate,
                                                         const QModelIndex& nowInd,
                                                         const QRect& visualRect,
                                                         const QPoint& clickedPnt) {
  const int oldRateValue = nowInd.data(ScenesListModel::RatingRole).toInt();
  mRateData.oldRateValue = oldRateValue;
  mRateData.newRateValue = oldRateValue;
  mRateData.curIndex = nowInd;
}
