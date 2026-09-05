#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "JsonParser.h"

#include "FileTool.h"
#include "JsonFieldBoundary.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class JsonParserTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void init() {  //
    GlobalMockObject::reset();
  }
  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void GetDurationFromJsonFile_ok() {
    bool bReadOk = true;
    MOCKER(FileTool::ByteArrayReader).expects(exactly(3)).with(any(), outBoundP(&bReadOk, sizeof(bReadOk)))
        .will(returnValue(QByteArray{R"({"Duration": 2264212,"Hot": []})"}))
        .then(returnValue(QByteArray{R"({"Duration": 2264213})"}))
        .then(returnValue(QByteArray{R"({})"}));
    bool bSucceed{false};
    QCOMPARE(JsonParser::GetDurationFromJsonFile("valid.json", &bSucceed, JsonFieldBoundary::DURATION_GET_FAILED_VALUE), 2264212);
    QVERIFY(bSucceed);
    bSucceed = false;
    QCOMPARE(JsonParser::GetDurationFromJsonFile("valid.json", &bSucceed, JsonFieldBoundary::DURATION_GET_FAILED_VALUE), 2264213);
    QVERIFY(bSucceed);
    bSucceed = true;
    QCOMPARE(JsonParser::GetDurationFromJsonFile("valid.json", &bSucceed, JsonFieldBoundary::DURATION_GET_FAILED_VALUE), JsonFieldBoundary::DURATION_GET_FAILED_VALUE);
    QVERIFY(!bSucceed);
  }

  void GetRateFromJsonFile_ok() {
    bool bReadOk = true;
    MOCKER(FileTool::ByteArrayReader).expects(exactly(3)).with(any(), outBoundP(&bReadOk, sizeof(bReadOk)))
        .will(returnValue(QByteArray{R"({"Rate": 8,"Hot": []})"}))
        .then(returnValue(QByteArray{R"({"Rate": 10})"}))
        .then(returnValue(QByteArray{R"({})"}));
    QCOMPARE(JsonParser::GetRateFromJsonFile("valid.json", JsonFieldBoundary::MovieRateE::RATE_MIN_UNINITIALIZED_V), 8);
    QCOMPARE(JsonParser::GetRateFromJsonFile("valid.json", JsonFieldBoundary::MovieRateE::RATE_MIN_UNINITIALIZED_V), 10);
    QCOMPARE(JsonParser::GetRateFromJsonFile("valid.json", JsonFieldBoundary::MovieRateE::RATE_MIN_UNINITIALIZED_V), JsonFieldBoundary::MovieRateE::RATE_MIN_UNINITIALIZED_V);
  }

  void GetMD5FromJsonFile_ok() {
    bool bReadOk = true;
    MOCKER(FileTool::ByteArrayReader).expects(exactly(4)).with(any(), outBoundP(&bReadOk, sizeof(bReadOk)))
        .will(returnValue(QByteArray{R"({"MD5": "01234567890123456789012345678901","Hot": []})"}))
        .then(returnValue(QByteArray{R"({"MD5": "01234567890123456789012345678910"})"}))
        .then(returnValue(QByteArray{R"({"MD5": "01"})"}))
        .then(returnValue(QByteArray{R"({})"}));
    QCOMPARE(JsonParser::GetMD5FromJsonFile("valid.json"), (QByteArray{"01234567890123456789012345678901"}));
    QCOMPARE(JsonParser::GetMD5FromJsonFile("valid.json"), (QByteArray{"01234567890123456789012345678910"}));
    QCOMPARE(JsonParser::GetMD5FromJsonFile("valid.json"), (QByteArray{}));
    QCOMPARE(JsonParser::GetMD5FromJsonFile("valid.json"), (QByteArray{}));
  }

  void GetTagsFromJsonFile_ok() {
    bool bReadOk = true;
    MOCKER(FileTool::ByteArrayReader).expects(exactly(3)).with(any(), outBoundP(&bReadOk, sizeof(bReadOk)))
        .will(returnValue(QByteArray{R"({"Tags": ["A", "B"],"Hot": []})"}))
        .then(returnValue(QByteArray{R"({"Tags": ["C"]})"}))
        .then(returnValue(QByteArray{R"({})"}));
    QCOMPARE(JsonParser::GetTagsFromJsonFile("valid.json"), (QStringList{"A", "B"}));
    QCOMPARE(JsonParser::GetTagsFromJsonFile("valid.json"), (QStringList{"C"}));
    QCOMPARE(JsonParser::GetTagsFromJsonFile("valid.json"), (QStringList{}));
  }

  void GetRateAndTagsFromJsonFile_ok () {
    bool bReadOk = true;
    MOCKER(FileTool::ByteArrayReader).expects(exactly(3)).with(any(), outBoundP(&bReadOk, sizeof(bReadOk)))
        .will(returnValue(QByteArray{R"({"Rate": 8,"Hot": [],"Tags": ["A", "B"],"Hot": []})"}))
        .then(returnValue(QByteArray{R"({"Rate": 10,"Tags": ["C"]})"}))
        .then(returnValue(QByteArray{R"({})"}));
    QCOMPARE(JsonParser::GetRateAndTagsFromJsonFile("valid.json", 0), (std::make_pair(8, QStringList{"A", "B"})));
    QCOMPARE(JsonParser::GetRateAndTagsFromJsonFile("valid.json", 0), (std::make_pair(10, QStringList{"C"})));
    QCOMPARE(JsonParser::GetRateAndTagsFromJsonFile("valid.json", 0), (std::make_pair(0, QStringList{})));
  }

  void JsonFileReadFailed() {
    bool bReadOk = false;
    MOCKER(FileTool::ByteArrayReader).expects(exactly(1)).with(any(), outBoundP(&bReadOk, sizeof(bReadOk))).will(returnValue(QByteArray{"{}"}));
    QCOMPARE(JsonParser::ParseEssentialFieldJson("inexist json file"), JsonOp::ResultE::ERROR);
  }

  void JsonFileContentEmpty() {
    bool bReadOk = true;
    MOCKER(FileTool::ByteArrayReader).expects(exactly(1)).with(any(), outBoundP(&bReadOk, sizeof(bReadOk))).will(returnValue(QByteArray{}));
    QCOMPARE(JsonParser::ParseEssentialFieldJson("empty json file"), JsonOp::ResultE::ERROR);
  }

  void standardJson_Formatted_NeedFurtherProcess() {
    bool bReadOk = true;
    MOCKER(FileTool::ByteArrayReader).stubs().with(any(), outBoundP(&bReadOk, sizeof(bReadOk))).will(returnValue(QByteArray{R"({
    "Bitrate": "5000 kbps",
    "Cast": [
        "Person 2",
        "99 Person"
    ],
    "ContentFixed": true,
    "Detail": "Release date: 1st June 2026",
    "Duration": 2264212,
    "Hot": [
    ],
    "ImgName": [
        "AAAA 1.jpg",
        "AAAA 2.jpg",
        "AAAA 3.jpg"
    ],
    "MD5": "e4484fb7dcb7c5d248f60b3bbd311354",
    "Name": "AAAA",
    "Rate": 8,
    "Resolution": "1080p",
    "Size": 1543270854,
    "Studio": "Pride",
    "Tags": [
        "2tage",
        "tage"
    ],
    "Uploaded": "15:16 01-Jun-2026",
    "VidName": "AAAA.mp4"
})"}));
    QByteArray sampleMd5Val;
    QString name, studio, vidName, detail;
    qint64 size{-1};
    int duration{-1}, rate{-1};
    QStringList casts, tags;
    QCOMPARE(JsonParser::ParseEssentialFieldJson("ValidJsonPath", &sampleMd5Val, &name, &vidName, &size, &duration, &studio, &casts, &tags, &rate, &detail), JsonOp::ResultE::OK_NEED_FURTHER_PROCESS);
    QCOMPARE(name, "AAAA");
    QCOMPARE(studio, "Pride");
    QCOMPARE(size, 1543270854);
    QCOMPARE(duration, 2264212);
    QCOMPARE(rate, 8);
    QCOMPARE(vidName, "AAAA.mp4");
    QCOMPARE(casts, (QStringList{"Person 2", "99 Person"}));
  }

  void standardJson_Formatted_butSomeFieldNotExist_NeedFurtherProcess() {
    // "Name" exist, MD5!=""
    bool bReadOk = true;
    MOCKER(FileTool::ByteArrayReader).stubs().with(any(), outBoundP(&bReadOk, sizeof(bReadOk))).will(returnValue(QByteArray{R"({
    "Cast": ["Person 2", "99 Person"],
    "Detail": "Release date: 1st June 2026",
    "Name": "AAAA",
    "Studio": "Pride",
    "Tags": ["2tage", "tage"],
    "MD5": "01234567890123456789012345678901",
    "VidName": "AAAA.dvd"
})"}));
    QByteArray sampleMd5Val;
    QString name, studio, vidName, detail;
    qint64 size{-1};
    int duration{-1}, rate{-1};
    QStringList casts, tags;
    QCOMPARE(JsonParser::ParseEssentialFieldJson("ValidJsonPath", &sampleMd5Val, &name, &vidName, &size, &duration, &studio, &casts, &tags, &rate, &detail), JsonOp::ResultE::OK_NEED_FURTHER_PROCESS);
    QCOMPARE(name, "AAAA");
    QCOMPARE(studio, "Pride");
    QCOMPARE(size, -1);
    QCOMPARE(duration, -1);
    QCOMPARE(rate, -1);
    QCOMPARE(casts, (QStringList{"Person 2", "99 Person"}));
  }

  void standardJson_NoMD5_MD5EmptyVidNameNotNull_noNeedFurtherProcess() {
    bool bReadOk = true;
    // 1. 无Name                       IGNORE_NO_NEED_FURTHER_PROCESS
    // 2. 有Name, MD5="", VidName=""   IGNORE_NO_NEED_FURTHER_PROCESS
    // 3. 有Name, MD5="", VidName!=""  ERROR log(Dismatch) and return ERROR
    // 4. 有Name, MD5!="", VidName=""  ERROR log(Dismatch) and return ERROR
    MOCKER(FileTool::ByteArrayReader).expects(exactly(4)).with(any(), outBoundP(&bReadOk, sizeof(bReadOk))).will(returnValue(QByteArray{R"({
    "Cast": ["Person 2", "99 Person"],
    "Detail": "Release date: 1st June 2026",
    "Studio": "Pride",
    "Tags": ["2tage", "tage"]
})"})).then(returnValue(QByteArray{R"({
    "Cast": ["Person 2", "99 Person"],
    "Detail": "Release date: 1st June 2026",
    "Name": "AAAA",
    "Studio": "Pride",
    "Tags": ["2tage", "tage"],
    "MD5": "",
    "VidName": ""
})"})).then(returnValue(QByteArray{R"({
    "Cast": ["Person 2", "99 Person"],
    "Detail": "Release date: 1st June 2026",
    "Name": "AAAA",
    "Studio": "Pride",
    "Tags": ["2tage", "tage"],
    "MD5": "",
    "VidName": "AAAA.dvd"
})"})).then(returnValue(QByteArray{R"({
    "Cast": ["Person 2", "99 Person"],
    "Detail": "Release date: 1st June 2026",
    "Name": "AAAA",
    "Studio": "Pride",
    "Tags": ["2tage", "tage"],
    "MD5": "01234567890123456789012345678901",
    "VidName": ""
})"}));
    QByteArray sampleMd5Val;
    QString name, studio, vidName, detail;
    qint64 size{-1};
    int duration{-1}, rate{-1};
    QStringList casts, tags;
    QCOMPARE(JsonParser::ParseEssentialFieldJson("No Name Field.json", &sampleMd5Val, &name, &vidName, &size, &duration, &studio, &casts, &tags, &rate, &detail), JsonOp::ResultE::IGNORE_NO_NEED_FURTHER_PROCESS);
    QCOMPARE(JsonParser::ParseEssentialFieldJson("Both MD5 and VidName Empty.json", &sampleMd5Val, &name, &vidName, &size, &duration, &studio, &casts, &tags, &rate, &detail), JsonOp::ResultE::IGNORE_NO_NEED_FURTHER_PROCESS);
    QCOMPARE(JsonParser::ParseEssentialFieldJson("MD5 Empty and VidName Not Empty.json", &sampleMd5Val, &name, &vidName, &size, &duration, &studio, &casts, &tags, &rate, &detail), JsonOp::ResultE::ERROR);
    QCOMPARE(JsonParser::ParseEssentialFieldJson("MD5 Not Empty and VidName Empty.json", &sampleMd5Val, &name, &vidName, &size, &duration, &studio, &casts, &tags, &rate, &detail), JsonOp::ResultE::ERROR);
  }

};

#include "JsonParserTest.moc"
REGISTER_TEST(JsonParserTest, false)