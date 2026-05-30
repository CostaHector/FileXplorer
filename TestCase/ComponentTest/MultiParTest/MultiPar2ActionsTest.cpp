#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "MultiPar2Actions.h"
#include "EndToExposePrivateMember.h"
#include "InputDialogHelper.h"
#include "MultiPar2MemoryKey.h"
#include "Configuration.h"
#include <QSignalSpy>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class MultiPar2ActionsTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void EmitCreatePar2Req_ok() {
    auto& inst = MultiPar2Actions::GetInst();

    {
      QCOMPARE(inst.EmitCreatePar2Req(nullptr), false);

      QAction noPropertyAct;
      QCOMPARE(inst.EmitCreatePar2Req(&noPropertyAct), false);

      QAction propertyOutOfRangeAct;
      propertyOutOfRangeAct.setProperty("RateOfRedundancy", -1);
      QCOMPARE(inst.EmitCreatePar2Req(&propertyOutOfRangeAct), false);
    }

    QSignalSpy createPar2ReqSpy{&inst, &MultiPar2Actions::createPar2Req};
    {
      QCOMPARE(inst.EmitCreatePar2Req(inst._CREATE_PAR2_FILES_10), true);
      // aka inst._CREATE_PAR2_FILES_10->triggered();
      QCOMPARE(createPar2ReqSpy.count(), 1);
      QVariantList parmsList = createPar2ReqSpy.takeLast();
      QCOMPARE(parmsList.size(), 1);
      QCOMPARE(parmsList.front().toInt(), 10);
    }
    {
      inst._CREATE_PAR2_FILES_15->triggered();
      QCOMPARE(createPar2ReqSpy.count(), 1);
      QVariantList parmsList = createPar2ReqSpy.takeLast();
      QCOMPARE(parmsList.size(), 1);
      QCOMPARE(parmsList.front().toInt(), 15);
    }
    {
      inst._CREATE_PAR2_FILES_20->triggered();
      QCOMPARE(createPar2ReqSpy.count(), 1);
      QVariantList parmsList = createPar2ReqSpy.takeLast();
      QCOMPARE(parmsList.size(), 1);
      QCOMPARE(parmsList.front().toInt(), 20);
    }
  }

  void EmitCreatePar2ReqCustom_ok() {
    auto& inst = MultiPar2Actions::GetInst();
    setConfig(MultiPar2MemoryKey::CUSTOM_RATE_OF_REDUNDANCY, 10);

    QSignalSpy createPar2ReqSpy{&inst, &MultiPar2Actions::createPar2Req};
    MOCKER(InputDialogHelper::GetIntWithInitial)
        .expects(exactly(3))                          //
        .will(returnValue(std::make_pair(false, 50))) // UserCancel0
        .then(returnValue(std::make_pair(true, -20))) // Yes but out of range
        .then(returnValue(std::make_pair(true, 45))); // Yes and valid

    QCOMPARE(inst.EmitCreatePar2ReqCustom(), false); // UserCancel0
    QCOMPARE(getConfig(MultiPar2MemoryKey::CUSTOM_RATE_OF_REDUNDANCY).toInt(), 10);
    QCOMPARE(createPar2ReqSpy.count(), 0);

    inst._CREATE_PAR2_FILES_CUSTOM->triggered(); // Yes but out of range
    QCOMPARE(getConfig(MultiPar2MemoryKey::CUSTOM_RATE_OF_REDUNDANCY).toInt(), 10);
    QCOMPARE(createPar2ReqSpy.count(), 0);

    QCOMPARE(inst.EmitCreatePar2ReqCustom(), true); // Yes and valid
    QCOMPARE(createPar2ReqSpy.count(), 1);
    QVariantList parmsList = createPar2ReqSpy.takeLast();
    QCOMPARE(parmsList.size(), 1);
    QCOMPARE(parmsList.front().toInt(), 45);

    QCOMPARE(getConfig(MultiPar2MemoryKey::CUSTOM_RATE_OF_REDUNDANCY).toInt(), 45);
  }
};

#include "MultiPar2ActionsTest.moc"
REGISTER_TEST(MultiPar2ActionsTest, false)
