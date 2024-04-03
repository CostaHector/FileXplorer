#include <QCoreApplication>
#include <QtTest>

#include "Tools/ConvertBoldUnicodeCharsetToAscii.h"

class ConvertBoldUnicodeCharsetToAsciiTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  void allCharset() {
    const QString& actual = ConvertBoldUnicodeCharsetToAscii()("𝗔𝗕𝗖𝗗𝗘𝗙𝗚𝗛𝗜𝗝𝗞𝗟𝗠𝗡𝗢𝗣𝗤𝗥𝗦𝗧𝗨𝗩𝗪𝗫𝗬𝗭𝗮𝗯𝗰𝗱𝗲𝗳𝗴𝗵𝗶𝗷𝗸𝗹𝗺𝗻𝗼𝗽𝗾𝗿𝘀𝘁𝘂𝘃𝘄𝘅𝘆𝘇");
    const QString& expect = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    QCOMPARE(expect, actual);
  }

  void extraUnicodeCharset() {
    const QString& actual = ConvertBoldUnicodeCharsetToAscii()("𝐂𝗗𝗘𝐬𝐭𝐰");
    const QString& expect = "CDEstw";
    QCOMPARE(expect, actual);
  }
};

void ConvertBoldUnicodeCharsetToAsciiTest::initTestCase() {}

void ConvertBoldUnicodeCharsetToAsciiTest::cleanupTestCase() {}

void ConvertBoldUnicodeCharsetToAsciiTest::init() {}

void ConvertBoldUnicodeCharsetToAsciiTest::cleanup() {}

//QTEST_MAIN(ConvertBoldUnicodeCharsetToAsciiTest)

//#include "ConvertBoldUnicodeCharsetToAsciiTest.moc"
