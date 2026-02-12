#ifndef MOCKTESTSUITE_H
#define MOCKTESTSUITE_H

#include "PlainTestSuite.h"
class MockTestSuite : public PlainTestSuite {
public:
  using PlainTestSuite::PlainTestSuite;
protected:
  virtual void LLTUp();
  virtual void LLTDown();
private slots:
  void init();
  void cleanup();
};

#endif // MOCKTESTSUITE_H
