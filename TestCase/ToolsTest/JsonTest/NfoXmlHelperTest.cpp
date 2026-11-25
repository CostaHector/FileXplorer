#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "NfoXmlHelper.h"
#include "FileTool.h"
#include "JsonHelper.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

    namespace NfoXmlHelper {
  QString ProcessDetailMap(const QStringList& castLst, const QString& castDetail, QMap<QString, QString>& detailMap);
  std::pair<bool, QVariantHash> MovieNfoXml2Json(const QString& nfoFileContentsRaw);
}

class NfoXmlHelperTest : public PlainTestSuite {
  Q_OBJECT
public:
private:
  QString rawContents;
private slots:
  void initTestCase() {
    rawContents = R"xml(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<!--created on 2021-11-10 13:43:38 - tinyMediaManager 4.2.2-->
<movie>
  <title>Movie Title Here</title>
  <originaltitle>Movie Title Here</originaltitle>
  <sorttitle>Movie Title Here</sorttitle>
  <epbookmark/>
  <year>2021</year>
  <ratings>
    <rating default="false" max="100" name="default">
      <value>9.5</value>
      <votes>1</votes>
    </rating>
  </ratings>
  <userrating>0.0</userrating>
  <top250>0</top250>
  <set/>
  <plot>Description on this movie</plot>
  <outline>Description on this movie</outline>
  <tagline/>
  <runtime>35</runtime>
  <mpaa>NC-17</mpaa>
  <certification>NC-17</certification>
  <id/>
  <tmdbid/>
  <tmdbCollectionId/>
  <country>US</country>
  <status/>
  <code/>
  <premiered>2021-10-07</premiered>
  <watched>false</watched>
  <playcount>0</playcount>
  <lastplayed/>
  <genre>tag1</genre>
  <genre>tag2</genre>
  <studio>Marvel Studios</studio>
  <actor>
    <name>Chris Evans</name>
    <role>captain</role>
    <thumb>imgb1f36d17.jpg</thumb>
    <profile>profile here</profile>
  </actor>
  <actor>
    <name>Michael Fassbender</name>
    <role>main character</role>
    <thumb>img7f3ef7cc.jpg</thumb>
    <profile/>
  </actor>
  <trailer/>
  <languages>English</languages>
  <dateadded>2021-11-02 11:45:37</dateadded>
  <fileinfo>
    <streamdetails>
      <video>
        <codec>HEVC</codec>
        <aspect>1.78</aspect>
        <width>1280</width>
        <height>720</height>
        <durationinseconds>2114</durationinseconds>
        <stereomode/>
      </video>
      <audio>
        <codec>AAC</codec>
        <language>eng</language>
        <channels>2</channels>
      </audio>
    </streamdetails>
  </fileinfo>
  <!--tinyMediaManager meta data-->
  <source>WEB_DL</source>
  <edition>NONE</edition>
  <original_filename>Movie Title Here.mkv</original_filename>
  <user_note/>
</movie>
)xml";
  }

  void init() {  //
    GlobalMockObject::reset();
  }
  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void MovieNfoXml2Json_ok() {
    bool parseOk{false};
    QVariantHash parseResult;
    std::tie(parseOk, parseResult) = NfoXmlHelper::MovieNfoXml2Json(rawContents);
    QVERIFY(parseOk);
    QCOMPARE(parseResult["Name"].toString(), "Movie Title Here");
    QCOMPARE(parseResult["Studio"].toString(), "Marvel Studios");
    QCOMPARE(parseResult["Cast"].toStringList(), (QStringList{"Chris Evans", "Michael Fassbender"}));
    QCOMPARE(parseResult["Tags"].toStringList(), (QStringList{"tag1", "tag2"}));
    QCOMPARE(parseResult["Resolution"].toString(), "1280x720");
    const QString detailMessage = parseResult["Detail"].toString();
    const QString startStr{"Casts: Chris Evans, Michael Fassbender\n"
                           "Roles:\n"
                           "Chris Evans, captain, imgb1f36d17.jpg, profile here\n"
                           "Michael Fassbender, main character, img7f3ef7cc.jpg, "
                           "\n\nDescription on this movie\n\n"};
    QVERIFY(detailMessage.startsWith(startStr));
    QVERIFY(detailMessage.contains("country: US"));
  }

  void GenerateJsonFromNfoXML_ok() {
    bool bReadInexistCSVFailed = false;
    MOCKER(FileTool::StringTextReader).expects(exactly(1)) //
        .with(eq(QString{"inexist movie nfo file path.nfo"}), outBoundP(&bReadInexistCSVFailed, sizeof(bReadInexistCSVFailed)))
        .will(returnValue(QString{""})) // call1
        .id("ReadInexistCSVFailed");

    bool bReadExistCSVOk = true;
    MOCKER(FileTool::StringTextReader).expects(exactly(2))//
        .with(eq(QString{"exist nfo file path.nfo"}), outBoundP(&bReadExistCSVOk, sizeof(bReadExistCSVOk)))
        .after("ReadInexistCSVFailed")
        .will(returnValue(QString{"<movie>"}))    // call2 corrupt contents file return empty contents
        .then(returnValue(QString{rawContents})); // call3 return valid movie nfo

    MOCKER(JsonHelper::DumpJsonDict).expects(exactly(1)).will(returnValue(true));

    // 不以.nfo结尾, 直接跳过
    QCOMPARE(NfoXmlHelper::GenerateJsonFromNfoXML("FileNotEndswithNfo.json"), false);
    QCOMPARE(NfoXmlHelper::GenerateJsonsFromNfoXMLs({"FileNotEndswithNfo.json"}), 0);

    // call 1. nfo 文件不存在, 直接跳过
    QCOMPARE(NfoXmlHelper::GenerateJsonFromNfoXML("inexist movie nfo file path.nfo"), false);

    // call 2. nfo 文件内容corrupt, 直接跳过
    QCOMPARE(NfoXmlHelper::GenerateJsonFromNfoXML("exist nfo file path.nfo"), false);

    // call 3. 解析正确, 写入成功
    QCOMPARE(NfoXmlHelper::GenerateJsonFromNfoXML("exist nfo file path.nfo"), true);
  }
};

#include "NfoXmlHelperTest.moc"
REGISTER_TEST(NfoXmlHelperTest, false)