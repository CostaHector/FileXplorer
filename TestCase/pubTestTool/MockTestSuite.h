#ifndef MOCKTESTSUITE_H
#define MOCKTESTSUITE_H

#include "PlainTestSuite.h"
#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class MockTestSuite : public PlainTestSuite {
public:
  using PlainTestSuite::PlainTestSuite;

protected:
  virtual void LLTUp();
  virtual void LLTDown();
protected slots:
  void init() {
    GlobalMockObject::reset();
    LLTUp();
    qDebug("init now");
  }
  void cleanup() {
    LLTDown();
    GlobalMockObject::verify();
    qDebug("cleanup now");
  }
};

#endif // MOCKTESTSUITE_H
