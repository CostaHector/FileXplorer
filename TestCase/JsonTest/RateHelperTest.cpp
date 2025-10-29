#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RateHelper.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "JsonHelper.h"

using namespace JsonHelper;

class RateHelperTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir tDir;
private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());
    QList<FsNodeEntry> nodes{
        {"rate ok.webp", false, ""},
        {"rate ok - 1.png", false, ""},                  // re-rate rate value will be override ok.
        {"rate ok 2.jpg", false, ""},                    // re-rate rate value will be override ok.
        {"rate ok.json", false, R"({"Name":""})"},       // re-rate rate value will be override ok.
        {"rate failed.mp4", false, ""},                  // no json["Name"] correspond in current folder, rate failed
        {"rate failed.webp", false, ""},                 // no json["Name"] correspond in current folder, rate failed
        {"rate failed.json", false, R"({"random":""})"}, // no json["Name"] correspond in current folder, rate failed
        {"rate failed 2.json", false, R"({"Name":""})"},
        {"subfolder/rate failed.json", false, R"({"Name":""})"},
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());
  }

  void test_invalid_scenario() {
    // Missing JSON test
    {
      QVERIFY(!RateHelper::RateMovie(tDir.itemPath("rate failed.mp4"), 5));
      QVERIFY(!RateHelper::RateMovie(tDir.itemPath("rate failed.webp"), 5));
    }

    // Invalid JSON content
    {
      QVERIFY(!RateHelper::RateMovie(tDir.itemPath("rate failed.json"), 5));
    }
  }

  void test_rate_helper_comprehensive() {
    // WEBP rating test passed
    {
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok.webp"), 9));
      QVariantHash data = MovieJsonLoader(tDir.itemPath("rate ok.json"));
      QCOMPARE(data["Rate"].toInt(), 9);
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok.webp"), 9)); // nothing should happened
    }

    // PNG re-rating test passed
    {
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok - 1.png"), 8));
      QVariantHash data = MovieJsonLoader(tDir.itemPath("rate ok.json"));
      QCOMPARE(data["Rate"].toInt(), 8);
    }

    // JPG re-rating test passed
    {
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok 2.jpg"), 7));
      QVariantHash data = MovieJsonLoader(tDir.itemPath("rate ok.json"));
      QCOMPARE(data["Rate"].toInt(), 7);
    }

    // Direct JSON
    {
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok.json"), 6));
      QVariantHash data = MovieJsonLoader(tDir.itemPath("rate ok.json"));
      QCOMPARE(data["Rate"].toInt(), 6);
    }

    // Boundary
    {
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok - 1.png"), 0));  // 最小值
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok - 1.png"), 10)); // 最大值
      QVariantHash data = MovieJsonLoader(tDir.itemPath("rate ok.json"));
      QCOMPARE(data["Rate"].toInt(), 10); // 最后一次评分应该是10
    }

    // rating clamping
    {
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok - 1.png"), 15));
      QVariantHash data = MovieJsonLoader(tDir.itemPath("rate ok.json"));
      QCOMPARE(data["Rate"].toInt(), 10); // 应该被钳制到10

      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok - 1.png"), -15));
      data = MovieJsonLoader(tDir.itemPath("rate ok.json"));
      QCOMPARE(data["Rate"].toInt(), 0); // 应该被钳制到0
    }
  }
};

#include "RateHelperTest.moc"
REGISTER_TEST(RateHelperTest, false)
