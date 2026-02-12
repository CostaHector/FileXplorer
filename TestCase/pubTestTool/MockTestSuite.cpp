#include "MockTestSuite.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

void MockTestSuite::LLTUp() {}
void MockTestSuite::LLTDown() {}

void MockTestSuite::init() {
  LLTUp();
}

void MockTestSuite::cleanup() {
  LLTDown();
}
