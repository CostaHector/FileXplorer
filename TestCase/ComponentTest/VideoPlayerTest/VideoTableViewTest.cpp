#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "EndToExposePrivateMember.h"

class VideoTableViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {
  }

  void cleanupTestCase() {  }

};

#include "VideoTableViewTest.moc"
REGISTER_TEST(VideoTableViewTest, false)
