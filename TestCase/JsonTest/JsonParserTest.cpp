#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "JsonParser.h"
#include "EndToExposePrivateMember.h"

#include "FileTool.h"

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


  void whenJsonFileReadFailed() {
    bool bReadOk = true;
    MOCKER(FileTool::ByteArrayReader).stubs().with(any(), outBoundP(&bReadOk, sizeof(bReadOk))).will(returnValue(QByteArray{}));
    QVERIFY(!JsonParser::ParseEssentialFieldJson("inexist json file"));
  }

  void standardJson_Formatted_Ok() {
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
    QVERIFY(JsonParser::ParseEssentialFieldJson("ValidJsonPath", &sampleMd5Val, &name, &vidName, &size, &duration, &studio, &casts, &tags, &rate, &detail));
    QCOMPARE(name, "AAAA");
    QCOMPARE(studio, "Pride");
    QCOMPARE(size, 1543270854);
    QCOMPARE(duration, 2264212);
    QCOMPARE(rate, 8);
    QCOMPARE(vidName, "AAAA.mp4");
    QCOMPARE(casts, (QStringList{"Person 2", "99 Person"}));
  }

  void standardJson_Formatted_butSomeFieldNotExist_Ok() {
    bool bReadOk = true;
    MOCKER(FileTool::ByteArrayReader).stubs().with(any(), outBoundP(&bReadOk, sizeof(bReadOk))).will(returnValue(QByteArray{R"({
    "Cast": ["Person 2", "99 Person"],
    "Detail": "Release date: 1st June 2026",
    "Name": "AAAA",
    "Studio": "Pride",
    "Tags": ["2tage", "tage"],
})"}));
    QByteArray sampleMd5Val;
    QString name, studio, vidName, detail;
    qint64 size{-1};
    int duration{-1}, rate{-1};
    QStringList casts, tags;
    QVERIFY(JsonParser::ParseEssentialFieldJson("ValidJsonPath", &sampleMd5Val, &name, &vidName, &size, &duration, &studio, &casts, &tags, &rate, &detail));
    QCOMPARE(name, "AAAA");
    QCOMPARE(studio, "Pride");
    QCOMPARE(size, -1);
    QCOMPARE(duration, -1);
    QCOMPARE(rate, -1);
    QCOMPARE(casts, (QStringList{"Person 2", "99 Person"}));
  }

};

#include "JsonParserTest.moc"
REGISTER_TEST(JsonParserTest, true)