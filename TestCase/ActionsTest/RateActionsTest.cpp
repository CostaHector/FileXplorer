#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RateActions.h"
#include "EndToExposePrivateMember.h"
#include "InputDialogHelper.H"
#include "MemoryKey.h"
#include "RateHelper.h"

#include <QSignalSpy>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class RateActionsTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void basic_ok() {
    auto& inst0 = RateActions::GetInst(RateActions::RateRequestFrom::FILE_XPLORER);
    auto& inst1 = RateActions::GetInst(RateActions::RateRequestFrom::VIDEO_TABLE_VIEW);
    auto& inst2 = RateActions::GetInst(RateActions::RateRequestFrom::FROM_BUTT); // fall back to FILE_XPLORER
    QCOMPARE(&inst0, &inst2);
    QVERIFY(&inst0 != &inst1);
  }

  void signals_ok() {
    auto& inst = RateActions::GetInst(RateActions::RateRequestFrom::VIDEO_TABLE_VIEW);
    {
      QSignalSpy rateMovieReqSpy{&inst, &RateActions::RateMovieReq};

      QVERIFY(!inst.onRateActionTriggered(nullptr));
      emit inst.RATE_AGS->triggered(nullptr);
      QCOMPARE(rateMovieReqSpy.count(), 0);

      QVERIFY(inst.RATE_AGS != nullptr);
      QList<QAction*> actList{inst.RATE_AGS->actions()};
      QVERIFY(actList.size() > 0);
      QAction* firstAct = actList.front();
      QVERIFY(firstAct != nullptr);

      bool bContainData{false};
      const int expectNewRate = firstAct->data().toInt(&bContainData);
      QVERIFY(bContainData);

      QVERIFY(inst.onRateActionTriggered(firstAct));
      QCOMPARE(rateMovieReqSpy.count(), 1);
      QCOMPARE(rateMovieReqSpy.takeLast(), (QVariantList{expectNewRate}));

      bool bNotContainData{false};
      QAction emptyAct;
      emptyAct.data().toInt(&bNotContainData);
      QVERIFY(!bNotContainData);
      QCOMPARE(rateMovieReqSpy.count(), 0);
    }

    {
      QSignalSpy rateMovieRecursivelyReqSpy{&inst, &RateActions::RateMovieRecursivelyReq};
      QSignalSpy adjustRateMovieReqSpy{&inst, &RateActions::AdjustRateMovieReq};
      QSignalSpy adjustRateMovieRecursivelyReqSpy{&inst, &RateActions::AdjustRateMovieRecursivelyReq};

      inst._RATE_RECURSIVELY->trigger();
      QCOMPARE(rateMovieRecursivelyReqSpy.count(), 1);
      QCOMPARE(rateMovieRecursivelyReqSpy.takeLast(), (QVariantList{false}));

      inst._RATE_RECURSIVELY_OVERRIDE->trigger();
      QCOMPARE(rateMovieRecursivelyReqSpy.count(), 1);
      QCOMPARE(rateMovieRecursivelyReqSpy.takeLast(), (QVariantList{true}));

      inst._INCREASING_RATING->trigger();
      QCOMPARE(adjustRateMovieReqSpy.count(), 1);
      QCOMPARE(adjustRateMovieReqSpy.takeLast(), (QVariantList{1}));
      inst._DECREASING_RATING->trigger();
      QCOMPARE(adjustRateMovieReqSpy.count(), 1);
      QCOMPARE(adjustRateMovieReqSpy.takeLast(), (QVariantList{-1}));

      inst._INCREASING_RATETING_RECURSIVELY->trigger();
      QCOMPARE(adjustRateMovieRecursivelyReqSpy.count(), 1);
      QCOMPARE(adjustRateMovieRecursivelyReqSpy.takeLast(), (QVariantList{1}));
      inst._DECREASING_RATETING_RECURSIVELY->trigger();
      QCOMPARE(adjustRateMovieRecursivelyReqSpy.count(), 1);
      QCOMPARE(adjustRateMovieRecursivelyReqSpy.takeLast(), (QVariantList{-1}));
    }
  }

  void onRateMoviesRecursively_ok() {
    constexpr int initRate = 6;
    Configuration().setValue(VideoPlayerKey::RATE_MOVIE_DEFAULT_VALUE.name, initRate);
    QCOMPARE(Configuration().value(VideoPlayerKey::RATE_MOVIE_DEFAULT_VALUE.name, VideoPlayerKey::RATE_MOVIE_DEFAULT_VALUE.v).toInt(), initRate);

    constexpr int userInputNewValue = 9;
    const std::pair<bool, int> cancel0{false, 7};
    const std::pair<bool, int> accept1{true, userInputNewValue};

    MOCKER(InputDialogHelper::GetIntWithInitial)                                                            //
        .expects(exactly(2))                                                                                //
        // .with(eq(nullptr), any(), any(), eq(initRate), eq(RateHelper::MIN_V), eq(RateHelper::MAX_V), eq(1)) //
        .will(returnValue(cancel0))                                                                         // user cancelled
        .then(returnValue(accept1));                                                                        // path not exist

    auto& inst = RateActions::GetInst(RateActions::RateRequestFrom::VIDEO_TABLE_VIEW);
    QCOMPARE(inst.onRateMoviesRecursively("path/2/inexists path", true, nullptr), 0); // user cancelled
    QCOMPARE(Configuration().value(VideoPlayerKey::RATE_MOVIE_DEFAULT_VALUE.name, VideoPlayerKey::RATE_MOVIE_DEFAULT_VALUE.v).toInt(), initRate);

    QCOMPARE(inst.onRateMoviesRecursively("path/2/inexists path", true, nullptr), 0); // path not exist
    QCOMPARE(Configuration().value(VideoPlayerKey::RATE_MOVIE_DEFAULT_VALUE.name, VideoPlayerKey::RATE_MOVIE_DEFAULT_VALUE.v).toInt(), userInputNewValue);
  }
};

#include "RateActionsTest.moc"
REGISTER_TEST(RateActionsTest, false)
