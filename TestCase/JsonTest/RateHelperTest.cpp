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
        {"rate ok - 1.png", false, ""},                   // re-rate rate value will be override ok.
        {"rate ok 2.jpg", false, ""},                     // re-rate rate value will be override ok.
        {"rate ok.json", false, R"({"Name":""})"},        // re-rate rate value will be override ok.
        {"rate failed.mp4", false, ""},                   // no json["Name"] correspond in current folder, rate failed
        {"rate failed.webp", false, ""},                  // no json["Name"] correspond in current folder, rate failed
        {"rate failed.json", false, R"({"random":""})"},  // no json["Name"] correspond in current folder, rate failed
        {"rate failed 2.json", false, R"({"Name":""})"},
        {"subfolder/rate failed.json", false, R"({"Name":""})"},
        {"subfolder/subfolder.json", false, R"({"Name":""})"},
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());
  }

  void ratingCalculation_ok() {
    constexpr int DEV_X = 99;
    constexpr int DEV_Y = 999;
    QRect visualRect(DEV_X, DEV_Y, 100, 200);
    QCOMPARE(visualRect.width(), 100);
    QCOMPARE(visualRect.height(), 200);

    QCOMPARE(RateHelper::ratingAtPosition(QPoint{DEV_X + 0, 500}, visualRect), 1);
    QCOMPARE(RateHelper::ratingAtPosition(QPoint{DEV_X + 10, 500}, visualRect), 2);
    QCOMPARE(RateHelper::ratingAtPosition(QPoint{DEV_X + 19, 500}, visualRect), 2);  // truncate simply
    QCOMPARE(RateHelper::ratingAtPosition(QPoint{DEV_X + 20, 500}, visualRect), 3);
    QCOMPARE(RateHelper::ratingAtPosition(QPoint{DEV_X + 30, 500}, visualRect), 4);
    QCOMPARE(RateHelper::ratingAtPosition(QPoint{DEV_X + 40, 500}, visualRect), 5);
    QCOMPARE(RateHelper::ratingAtPosition(QPoint{DEV_X + 50, 500}, visualRect), 6);
    QCOMPARE(RateHelper::ratingAtPosition(QPoint{DEV_X + 60, 500}, visualRect), 7);
    QCOMPARE(RateHelper::ratingAtPosition(QPoint{DEV_X + 70, 500}, visualRect), 8);
    QCOMPARE(RateHelper::ratingAtPosition(QPoint{DEV_X + 80, 500}, visualRect), 9);
    QCOMPARE(RateHelper::ratingAtPosition(QPoint{DEV_X + 90, 500}, visualRect), 10);
    QCOMPARE(RateHelper::ratingAtPosition(QPoint{DEV_X + 100, 500}, visualRect), 10);
  }

  void getRatingRect_ok() {  //
    QCOMPARE(RateHelper::RATING_BAR_HEIGHT, 50);
    // y0 = 200 - 50, 100 - 50, 80 - 50, 50 - 50
    QCOMPARE(RateHelper::getRatingRect(QRect{0, 0, 100, 200}), (QRect{0, 150, 100, 50}));
    QCOMPARE(RateHelper::getRatingRect(QRect{0, 0, 80, 100}), (QRect{0, 50, 80, 50}));
    QCOMPARE(RateHelper::getRatingRect(QRect{0, 0, 50, 80}), (QRect{0, 30, 50, 50}));
    QCOMPARE(RateHelper::getRatingRect(QRect{0, 0, 50, 50}), (QRect{0, 0, 50, 50}));

    QCOMPARE(RateHelper::getRatingRect(QRect{0, 20, 100, 200}), (QRect{0, 170, 100, 50}));
    QCOMPARE(RateHelper::getRatingRect(QRect{0, 20, 80, 100}), (QRect{0, 70, 80, 50}));
    QCOMPARE(RateHelper::getRatingRect(QRect{0, 20, 50, 80}), (QRect{0, 50, 50, 50}));
    QCOMPARE(RateHelper::getRatingRect(QRect{0, 20, 50, 50}), (QRect{0, 20, 50, 50}));
  }

  void isClickPointInsideRatingBar_ok() {
    const QRect rect{0, 0, 100, 400};
    // 0 <= x < 100
    // 350 <= y < 400
    QVERIFY(RateHelper::isClickPointInsideRatingBar((QPoint{0, 399}), rect));
    QVERIFY(RateHelper::isClickPointInsideRatingBar((QPoint{0, 350}), rect));
    QVERIFY(RateHelper::isClickPointInsideRatingBar((QPoint{99, 350}), rect));
    QVERIFY(RateHelper::isClickPointInsideRatingBar((QPoint{99, 399}), rect));

    QVERIFY(RateHelper::isClickPointInsideRatingBar((QPoint{50, 375}), rect));

    QVERIFY(!RateHelper::isClickPointInsideRatingBar((QPoint{100, 400}), rect));
    QVERIFY(!RateHelper::isClickPointInsideRatingBar((QPoint{100, 375}), rect));
    QVERIFY(!RateHelper::isClickPointInsideRatingBar((QPoint{50, 400}), rect));
    QVERIFY(!RateHelper::isClickPointInsideRatingBar((QPoint{0, 0}), rect));
    QVERIFY(!RateHelper::isClickPointInsideRatingBar((QPoint{200, 375}), rect));
    QVERIFY(!RateHelper::isClickPointInsideRatingBar((QPoint{50, 500}), rect));
  }

  void rate_pixmap_ok() {
    static constexpr int SLICE_COUNT = 100;
    QPixmap rate0Pix = RateHelper::GenerateRatePixmap(0, SLICE_COUNT, false);  // no border
    QVERIFY(!rate0Pix.isNull());
    QPixmap rate100Pix = RateHelper::GenerateRatePixmap(100, SLICE_COUNT, true);
    QVERIFY(!rate100Pix.isNull());
    QPixmap rateInvalidPix = RateHelper::GenerateRatePixmap(101, SLICE_COUNT, true);
    QVERIFY(rateInvalidPix.isNull());
  }

  void invalid_scenario() {
    // Missing JSON test
    {
      QVERIFY(!RateHelper::RateMovie(tDir.itemPath("rate failed.mp4"), 5));
      QVERIFY(!RateHelper::RateMovie(tDir.itemPath("rate failed.webp"), 5));
    }

    // Invalid JSON content
    { QVERIFY(!RateHelper::RateMovie(tDir.itemPath("rate failed.json"), 5)); }
  }

  void rate_helper_comprehensive() {
    // WEBP rating test passed
    {
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok.webp"), 9));
      QVariantHash data = MovieJsonLoader(tDir.itemPath("rate ok.json"));
      QCOMPARE(data["Rate"].toInt(), 9);
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok.webp"), 9));  // nothing should happened
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

    // Direct JSON test passed
    {
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok.json"), 6));
      QVariantHash data = MovieJsonLoader(tDir.itemPath("rate ok.json"));
      QCOMPARE(data["Rate"].toInt(), 6);
    }

    // Folder test passed
    {
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("subfolder"), 1));
      QVariantHash data = MovieJsonLoader(tDir.itemPath("subfolder/subfolder.json"));
      QCOMPARE(data["Rate"].toInt(), 1);
    }

    // Boundary
    {
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok - 1.png"), 0));   // 最小值
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok - 1.png"), 10));  // 最大值
      QVariantHash data = MovieJsonLoader(tDir.itemPath("rate ok.json"));
      QCOMPARE(data["Rate"].toInt(), 10);  // 最后一次评分应该是10
    }

    // rating clamping
    {
      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok - 1.png"), 15));
      QVariantHash data = MovieJsonLoader(tDir.itemPath("rate ok.json"));
      QCOMPARE(data["Rate"].toInt(), 10);  // 应该被钳制到10

      QVERIFY(RateHelper::RateMovie(tDir.itemPath("rate ok - 1.png"), -15));
      data = MovieJsonLoader(tDir.itemPath("rate ok.json"));
      QCOMPARE(data["Rate"].toInt(), 0);  // 应该被钳制到0
    }
  }

  void rate_recursively_ok() {
    static constexpr int RATE_VALUE_WHEN_KEY_NONEXIST = -1;
    const QStringList jsonsPath{
        tDir.itemPath("rate ok.json"),                // 0
        tDir.itemPath("rate failed 2.json"),          // RATE_VALUE_WHEN_KEY_NONEXIST
        tDir.itemPath("subfolder/rate failed.json"),  // RATE_VALUE_WHEN_KEY_NONEXIST
        tDir.itemPath("subfolder/subfolder.json"),    // 1
    };

    static const auto GetRatesFromJson = [](const QStringList& jsonsPath) -> QList<int> {
      QVariantHash data;
      QList<int> beforeRateList;
      beforeRateList.reserve(jsonsPath.size());
      for (const QString& jsonPath : jsonsPath) {
        data = MovieJsonLoader(jsonPath);
        beforeRateList.push_back(data.value("Rate", RATE_VALUE_WHEN_KEY_NONEXIST).toInt());
      }
      return beforeRateList;
    };

    {
      QCOMPARE(RateHelper::RateMovieRecursively(tDir.path(), 10, false), 4);  // non override
      const QList<int> expectsRates{0, 10, 10, 1};
      QList<int> actualRates = GetRatesFromJson(jsonsPath);
      QCOMPARE(actualRates, expectsRates);
    }

    {
      QCOMPARE(RateHelper::RateMovieRecursively(tDir.path(), 3, true), 4);  // force override
      const QList<int> expectsRates{3, 3, 3, 3};
      QList<int> actualRates = GetRatesFromJson(jsonsPath);
      QCOMPARE(actualRates, expectsRates);
    }
  }
};

#include "RateHelperTest.moc"
REGISTER_TEST(RateHelperTest, false)
