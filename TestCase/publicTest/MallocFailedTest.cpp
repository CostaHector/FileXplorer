#include <QCoreApplication>
#include <QtTest>
#include "Logger.h"
#include "PlainTestSuite.h"

#define MOCK_TEST_MALLOC_FAILED // when defined, this test case will failed
#undef MOCK_TEST_MALLOC_FAILED

class MallocFailedTest : public PlainTestSuite {
  Q_OBJECT
public:
  using PlainTestSuite::PlainTestSuite;
#ifdef TEST_MALLOC_FAILED
  static void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
    LOG_C("Mock when new (std::nothrow) testClassName failed");
    return nullptr;
  }
  static void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
    LOG_C("Mock when new (std::nothrow) testClassName[] failed");
    return nullptr;
  }
#endif
private slots:
  void test_placeholder() {
    QCOMPARE(1, 1);
  }
};

#include "MallocFailedTest.moc"
REGISTER_TEST(MallocFailedTest, false)
