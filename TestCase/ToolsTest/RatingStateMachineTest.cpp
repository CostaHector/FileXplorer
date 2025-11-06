#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "SceneStyleDelegate.h"
#include "BeginToExposePrivateMember.h"
#include "RatingStateMachine.h"
#include "EndToExposePrivateMember.h"

#include <QModelIndex>

class RatingStateMachineTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:

  void rate_data_default_status_ok() {
    RateData rateData;
    QVERIFY(rateData.isInit());
  }

  void default_status_ok() {
    RatingStateMachine mRateMachine;
    QCOMPARE(mRateMachine.status(), RatingState::IDLE);
    QCOMPARE(mRateMachine.curIndex(), (QModelIndex{}));
    QCOMPARE(mRateMachine.oldRate(), 0);
    QCOMPARE(mRateMachine.newRate(), 0);

    constexpr int DIM1 = std::extent<decltype(mRateMachine.mTransitionTable), 0>::value;
    constexpr int DIM2 = std::extent<decltype(mRateMachine.mTransitionTable), 1>::value;
    constexpr int ELE_CNT = DIM1 * DIM2;
    QCOMPARE(DIM1, (int)RatingState::BUTT);
    QCOMPARE(DIM2, (int)RatingAction::BUTT);

    RatingStateMachine::F_STATE_TRANSIT* pBegin = mRateMachine.mTransitionTable[0];
    RatingStateMachine::F_STATE_TRANSIT* pEnd = pBegin + ELE_CNT;
    RatingStateMachine::F_STATE_TRANSIT* itOfNullptr = std::find(pBegin, pEnd, nullptr);
    QCOMPARE(itOfNullptr, pEnd);

    SceneStyleDelegate noUse;
    mRateMachine.PlaceHolderTransit(noUse, {}, {}, {});
  }
};

#include "RatingStateMachineTest.moc"
REGISTER_TEST(RatingStateMachineTest, false)
