#include <QtTest>
#include <QCoreApplication>
#include "PlainTestSuite.h"
#include "DataFormatter.h"
#include "JsonKey.h"
#include "SortedUniqStrLst.h"
#include <QJsonValue>

using namespace DataFormatter;
class DataFormatterTest : public PlainTestSuite {
  Q_OBJECT
public:
  DataFormatterTest() : PlainTestSuite{} {}
private slots:
  void test_formatDefault() {
    QVariant aInt{7};
    const auto& ansInt = formatDefault(aInt);
    QCOMPARE(ansInt, aInt);
    QCOMPARE(&ansInt, &aInt);

    QVariant aFloat{7.f};
    const auto& ansFloat = formatDefault(aFloat);
    QVERIFY(std::abs(ansFloat.toFloat() - aFloat.toFloat()) < 1E-6);
    QCOMPARE(&ansFloat, &aFloat);

    QVariant aBool{false};
    const auto& ansBool = formatDefault(aBool);
    QCOMPARE(ansBool, aBool);
    QCOMPARE(&ansBool, &aBool);

    QVariant aQString{"Hello world"};
    const auto& ansQString = formatDefault(aQString);
    QCOMPARE(ansQString, aQString);
    QCOMPARE(&ansQString, &aQString);
  }
  void test_writeQString() {
    QVariant srcStr{"Hello world"};
    QString dst;
    QVERIFY(writeQString(dst, srcStr));
    QVERIFY(!writeQString(dst, srcStr));  // no change
  }
  void test_writeInt() {
    QVariant srcInt{145};
    int dst;
    QVERIFY(writeInt(dst, srcInt));
    QVERIFY(!writeInt(dst, srcInt));  // no change
  }
  void test_writeFloat() {
    QVariant srcFloat{3.14f};
    float dst;
    QVERIFY(writeFloat(dst, srcFloat));
    QVERIFY(!writeFloat(dst, srcFloat));  // no change
  }
  void test_writeDouble() {
    QVariant srcDouble{3.14};
    double dst;
    QVERIFY(writeDouble(dst, srcDouble));
    QVERIFY(!writeDouble(dst, srcDouble));  // no change
  }
  void test_initQString() {
    QJsonObject json;
    json["exist key"] = "Hello world";
    QCOMPARE(initQString(json, "no exist key", "default value"), "default value");
    QCOMPARE(initQString(json, "exist key", "default value"), "Hello world");
  }
  void test_initStudioQString() {
    QJsonObject json;
    // no Studio, no ProductionStudio => default
    QCOMPARE(initStudioQString(json, ENUM_2_STR(Studio), "default value"), "default value");
    // no Studio, only ProductionStudio => use ProductionStudio
    json["ProductionStudio"] = "Fox 2000";
    QCOMPARE(initStudioQString(json, ENUM_2_STR(Studio), "default value"), "Fox 2000");
    // both Studio and ProductionStudio => use Studio
    json[ENUM_2_STR(Studio)] = "Columbia Pictures";
    QCOMPARE(initStudioQString(json, ENUM_2_STR(Studio), "default value"), "Columbia Pictures");
    // only Studio, no ProductionStudio => use Studio
    json.remove("ProductionStudio");
    QCOMPARE(initStudioQString(json, ENUM_2_STR(Studio), "default value"), "Columbia Pictures");
  }
  void test_initInt() {  //
    QJsonObject json;
    QCOMPARE(initInt(json, "int7", 0), 0);  // not exist
    json["int7"] = 7;
    QCOMPARE(initInt(json, "int7", 0), 7);  // exist
  }
  void test_initQStringLst() {  //
    QJsonObject json;
    QStringList defLst{"Hello", "Army"};
    QCOMPARE(initQStringLst(json, "not exist", defLst), defLst);
  }

  void test_initSortedLst() {  //
    QJsonObject json;
    QStringList defLst{"Hello", "Army"};
    QStringList expectDefLst{"Hello", "Army"};
    SortedUniqStrLst expectSortedLst{expectDefLst};
    QCOMPARE(initSortedLst(json, "not exist", defLst), expectSortedLst);

    QStringList randomLst{"B", "A"};
    QStringList expectRandomLst{"A", "B"};
    SortedUniqStrLst expectRandomSortedLst{expectRandomLst};
    json["exist"] = QJsonArray::fromStringList(randomLst);
    QCOMPARE(initSortedLst(json, "exist", defLst), expectRandomSortedLst);
  }

  void test_initCastSortedLst() {  //
    QJsonObject json;
    // no Cast, no Performers => default
    QStringList defaultSorted;
    SortedUniqStrLst defaultSortedContainer;
    QCOMPARE(initCastSortedLst(json, ENUM_2_STR(Cast), defaultSorted), defaultSortedContainer);

    // only Performers
    json[ENUM_2_STR(Performers)] = QJsonArray::fromStringList(QStringList{"C", "B", "A"});
    SortedUniqStrLst fromPerfs{QStringList{"A", "B", "C"}};
    QCOMPARE(initCastSortedLst(json, ENUM_2_STR(Performers), defaultSorted), fromPerfs);

    // both Cast and Performers
    json[ENUM_2_STR(Cast)] = QJsonArray::fromStringList(QStringList{"Z", "Y", "X"});
    SortedUniqStrLst fromCast{QStringList{"X", "Y", "Z"}};
    QCOMPARE(initCastSortedLst(json, ENUM_2_STR(Cast), defaultSorted), fromCast);

    // only Cast
    json.remove(ENUM_2_STR(Performers));
    QCOMPARE(initCastSortedLst(json, ENUM_2_STR(Cast), defaultSorted), fromCast);
  }
  void test_writeJsonObjectInt() {  //
    QJsonObject json;
    writeJsonObjectInt(json, "int", 7);
    QCOMPARE(json["int"], 7);
  }

  void test_writeJsonObjectFloat() {  //
    QJsonObject json;
    writeJsonObjectFloat(json, "float", 3.14f);
    QVERIFY(std::abs(json["float"].toDouble() - 3.14f) < 1E-6);
  }

  void test_writeJsonObjectDouble() {  //
    QJsonObject json;
    writeJsonObjectDouble(json, "double", 3.14);
    QVERIFY(std::abs(json["double"].toDouble() - 3.14) < 1E-6);
  }

  void test_writeJsonObjectBool() {  //
    QJsonObject json;
    writeJsonObjectBool(json, "bool", true);
    QCOMPARE(json["bool"], true);
  }

  void test_writeJsonObjectQString() {  //
    QJsonObject json;
    writeJsonObjectQString(json, "QString", "Nice job");
    QCOMPARE(json["QString"], "Nice job");
  }

  void test_writeJsonObjectQStringList() {  //
    QJsonObject json;
    QStringList vals{"B", "A"};           // no sorted
    QStringList notExpectVals{"B", "A"};  // no sorted
    writeJsonObjectQStringLst(json, "QStringList", vals);
    QCOMPARE(json["QStringList"].toVariant().toStringList(), notExpectVals);
  }

  void test_writeJsonObjectQIntList() {  //
    QJsonObject json;
    QList<int> val{2, 1};  // no sorted
    writeJsonObjectQIntList(json, "QIntList", val);

    QJsonArray ja;
    ja.append(2);
    ja.append(1);
    QCOMPARE(json["QIntList"].toArray(), ja);
  }
  void test_writeJsonObjectDefault() {  //
    QJsonObject json;
    writeJsonObjectDefault(json, "int", 1);
    QCOMPARE(json["int"], 1);
    writeJsonObjectDefault(json, "char", 'A');
    QCOMPARE(json["char"], 'A');
    writeJsonObjectDefault(json, "float", 3.14f);
    QVERIFY(std::abs(json["float"].toDouble() - 3.14f) < 1E-6);
    writeJsonObjectDefault(json, "QString", "Nice");
    QCOMPARE(json["QString"], "Nice");
  }
  void test_writeJsonObjectSortedStrLst() {  //
    QJsonObject json;
    QStringList rawLst{"C", "A", "B"};
    QStringList expectSortedLst{"A", "B", "C"};
    SortedUniqStrLst vals{rawLst};
    writeJsonObjectSortedStrLst(json, ENUM_2_STR(Cast), vals);
    QCOMPARE(json[ENUM_2_STR(Cast)].toVariant().toStringList(), expectSortedLst);

    writeJsonObjectSortedStrLst(json, ENUM_2_STR(Tags), vals);
    QCOMPARE(json[ENUM_2_STR(Tags)].toVariant().toStringList(), expectSortedLst);

    writeJsonObjectSortedStrLst(json, ENUM_2_STR(Hot), vals);
    QCOMPARE(json[ENUM_2_STR(Hot)].toVariant().toStringList(), expectSortedLst);
  }

  void test_formatSortedLst() {  //
    QJsonObject json;
    QStringList rawLst{"C", "A", "B"};
    QStringList expectSortedLst{"A", "B", "C"};
    SortedUniqStrLst vals{rawLst};
    QCOMPARE("A,B,C", vals.join());
    QCOMPARE(formatSortedLst(vals), vals.join());
  }

  void test_writeSortedLst() {  //
    SortedUniqStrLst container;
    QVariant srcEmpty{""};
    QVERIFY(!writeSortedLst(container, srcEmpty));

    QVariant srcLst{"B,A"};
    QVERIFY(writeSortedLst(container, srcLst));
    QCOMPARE(container.join(), "A,B");

    QVariant noChangeSrcLst{"A,B"};
    QVERIFY(!writeSortedLst(container, noChangeSrcLst));
    QCOMPARE(container.join(), "A,B");

    QVERIFY(writeSortedLst(container, srcEmpty));
    QCOMPARE(container.join(), "");
    QVERIFY(container.isEmpty());
  }

  void test_formatQStringLst() {
    QStringList emptyLst;
    QCOMPARE(formatQStringLst(emptyLst), "");

    QStringList container{"A", "C", "B"};
    QCOMPARE(formatQStringLst(container), "A,C,B");
  }

  void test_writeQStringLst() {
    QStringList container;
    QVariant src{QString{"A,C,B"}};
    QStringList expectContainer{"A", "C", "B"};
    QVERIFY(writeQStringLst(container, src));
    QCOMPARE(container, expectContainer);
    QVERIFY(!writeQStringLst(container, src));
    QCOMPARE(container, expectContainer);
  }

  void test_formatGender() {  //
    QCOMPARE(formatGender(Gender::male), "male");
    QCOMPARE(formatGender(Gender::female), "female");

    Gender current = Gender::female;
    QVERIFY(writeGender(current, "male"));
    QCOMPARE(current, Gender::male);
    QVERIFY(writeGender(current, "female"));
    QCOMPARE(current, Gender::female);
    QVERIFY(!writeGender(current, "female"));  // unchange

    // invalid value
    Gender prev = Gender::male;
    QVERIFY(writeGender(prev, "invalid"));
    QCOMPARE(prev, Gender::female);
  }

  void test_formatPhoneNumber() {  //
    QCOMPARE(formatPhoneNumber("8617788886666"), "86-177-8888-6666");

    QString current;
    QVERIFY(writePhoneNumber(current, "86-177-8888-6666"));
    QCOMPARE(current, QString("8617788886666"));

    QVERIFY(writePhoneNumber(current, "8617788887777"));
    QCOMPARE(current, QString("8617788887777"));

    QVERIFY(!writePhoneNumber(current, "86-177-8888-7777"));  // unchange

    QVERIFY(writePhoneNumber(current, ""));  // empty
    QCOMPARE(current, "");
  }

  void test_formatFloat() {  //
    QCOMPARE(formatFloat2Prec(3.14f), QString{"3.14"});
  }
  void test_writeRateAnnual() {  // not sure
    QList<char> current = {'A', 'B'};
    QVERIFY(writeRateAnnual(current, "X,Y,Z"));
    QCOMPARE(current, QList<char>({'X', 'Y', 'Z'}));

    QVERIFY(!writeRateAnnual(current, "X,Y,Z"));  // unchange

    QVERIFY(writeRateAnnual(current, ""));  // empty
    QVERIFY(current.isEmpty());

    QVERIFY(writeRateAnnual(current, " A, B , C "));  // skip blank char
    QCOMPARE(current, QList<char>({'A', 'B', 'C'}));

    QVERIFY(writeRateAnnual(current, "P,,Q,,"));  // skip empty
    QCOMPARE(current, QList<char>({'P', 'Q'}));

    // multi word, only first char keep
    QVERIFY(writeRateAnnual(current, "Hello,World"));
    QCOMPARE(current, QList<char>({'H', 'W'}));
  }
  void test_formatRateAnnual() {  //
    QList<char> emptyAnnual;
    QCOMPARE(formatRateAnnual(emptyAnnual), QString{""});
    QList<char> only1Annual{'A'};
    QCOMPARE(formatRateAnnual(only1Annual), QString{"A"});
    QList<char> rateAnnual{'A', 'B', 'B', 'A'};
    QCOMPARE(formatRateAnnual(rateAnnual), QString{"A,B,B,A"});
  }
  void test_formatHobbies() {
    QStringList hobbies{"hiking", "adventure"};
    QCOMPARE(formatHobbies(hobbies), QString{"adventure,hiking"});
  }
  void test_writeHobbies() {
    QString hobbiesSentence{"hiking,adventure"};
    QStringList hobbies;
    writeHobbies(hobbies, hobbiesSentence);
  }
  void test_formatBool() {  //
    QCOMPARE(formatBool(false), QString{"false"});
    QCOMPARE(formatBool(true), QString{"true"});

    bool current = false;
    QVERIFY(writeBool(current, "true"));
    QVERIFY(current);
    QVERIFY(writeBool(current, "false"));
    QVERIFY(!current);
    QVERIFY(!writeBool(current, "false"));

    QVERIFY(writeBool(current, "TRUE"));
    QVERIFY(current);
    QVERIFY(writeBool(current, "FALSE"));
    QVERIFY(!current);

    bool prev = true;
    QVERIFY(writeBool(prev, "invalid"));  // invalid to false by default
    QVERIFY(!prev);

    prev = false;
    QVERIFY(writeBool(prev, "1"));
    QVERIFY(prev);

    prev = true;
    QVERIFY(writeBool(prev, "0"));
    QVERIFY(!prev);
  }

  void test_fileSizeGMKB_ok() {
    QCOMPARE(formatFileSizeGMKB(1 * 1024 * 1024 * 1024  //
                                + 1 * 1024 * 1024       //
                                + 1 * 1024              //
                                + 1),                   //
             "1'1'1'1");                                     // 1G 1M 1K 1B
    QCOMPARE(formatFileSizeGMKB(0 * 1024 * 1024 * 1024  //
                                + 99 * 1024 * 1024      //
                                + 1023 * 1024           //
                                + 1),                   //
             "0'99'1023'1");                                 // 99M 1023K 1B
  }
  void test_sizeToFileSizeDetail_ok() {
    QCOMPARE(formatFileSizeWithBytes(0 * 1024 * 1024 * 1024  //
                                     + 1 * 1024 * 1024       //
                                     + 0 * 1024              //
                                     + 1),                   //
             "0'1'0'1 (1048577 Bytes)");                  // 1M 1B
  }
  void test_durationToHumanReadFriendly() {
    QCOMPARE(formatDurationISOMs((1 * 60 * 60   //
                                + 1 * 60      //
                                + 1)          //
                               * 1000),       //
             "01:01:01.000");                           // 01h01m01s
    QCOMPARE(formatDurationISOMs((23 * 60 * 60  //
                                + 0 * 60      //
                                + 59)         //
                               * 1000),       //
             "23:00:59.000");                           // 23h00m59s
    QCOMPARE(formatDurationISOMs((25 * 60 * 60  //
                                + 0 * 60      //
                                + 0)          //
                               * 1000),       //
             "");                                       // 25h > max 24h
  }
};

#include "DataFormatterTest.moc"
REGISTER_TEST(DataFormatterTest, false)
