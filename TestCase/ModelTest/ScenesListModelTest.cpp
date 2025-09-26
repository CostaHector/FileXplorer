#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "ScenesListModel.h"
#include "EndToExposePrivateMember.h"

#include <QDir>
#include <QDirIterator>

Q_DECLARE_METATYPE(QDir::Filters)
Q_DECLARE_METATYPE(QDirIterator::IteratorFlag)

class ScenesListModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {
    qRegisterMetaType<QDir::Filters>("QDir::Filters");
    qRegisterMetaType<QDirIterator::IteratorFlag>("QDirIterator::IteratorFlag");
    Configuration().clear();
  }

  void cleanupTestCase() { Configuration().clear(); }

  void test_1() {
// todo:
  }
};

#include "ScenesListModelTest.moc"
REGISTER_TEST(ScenesListModelTest, false)
