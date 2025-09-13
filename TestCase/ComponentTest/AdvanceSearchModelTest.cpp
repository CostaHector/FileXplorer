#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "AdvanceSearchModel.h"
#include "SearchProxyModel.h"
#include "EndToExposePrivateMember.h"

class AdvanceSearchModelTest : public PlainTestSuite {
  Q_OBJECT
public:
  AdvanceSearchModelTest() : PlainTestSuite{} {
    LOG_D("AdvanceSearchModelTest object created\n");
  }


private slots:
  void initTestCase() {
  }

  void cleanupTestCase() {
  }


};

#include "AdvanceSearchModelTest.moc"
REGISTER_TEST(AdvanceSearchModelTest, false)
