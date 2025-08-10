#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pubTestTool/MyTestSuite.h"
#include "public/DisplayEnhancement.h"

using namespace FILE_PROPERTY_DSP;
class DisplayEnhancementTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_fileSizeGMKB_ok() {
    QCOMPARE(sizeToHumanReadFriendly(1 * 1024 * 1024 * 1024  //
                                     + 1 * 1024 * 1024       //
                                     + 1 * 1024              //
                                     + 1),                   //
             "1'1'1'1");                                     // 1G 1M 1K 1B
    QCOMPARE(sizeToHumanReadFriendly(0 * 1024 * 1024 * 1024  //
                                     + 99 * 1024 * 1024      //
                                     + 1023 * 1024           //
                                     + 1),                   //
             "0'99'1023'1");                                 // 99M 1023K 1B
  }
  void test_sizeToFileSizeDetail_ok() {
    QCOMPARE(sizeToFileSizeDetail(0 * 1024 * 1024 * 1024  //
                                  + 1 * 1024 * 1024       //
                                  + 0 * 1024              //
                                  + 1),                   //
             "0'1'0'1 (1048577 Bytes)");                  // 1M 1B
  }
  void test_durationToHumanReadFriendly() {
    QCOMPARE(durationToHumanReadFriendly((1 * 60 * 60   //
                                          + 1 * 60      //
                                          + 1)          //
                                         * 1000),       //
             "01:01:01.000");                           // 01h01m01s
    QCOMPARE(durationToHumanReadFriendly((23 * 60 * 60  //
                                          + 0 * 60      //
                                          + 59)         //
                                         * 1000),       //
             "23:00:59.000");                           // 23h00m59s
    QCOMPARE(durationToHumanReadFriendly((25 * 60 * 60  //
                                          + 0 * 60      //
                                          + 0)          //
                                         * 1000),       //
             "");                                       // 25h > max 24h
  }
};

#include "DisplayEnhancementTest.moc"
DisplayEnhancementTest g_DisplayEnhancementTest;
