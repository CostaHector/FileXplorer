#include <QCoreApplication>
#include <QtTest>

#include "Tools/ConvertUnicodeCharsetToAscii.h"

class ConvertUnicodeCharsetToAsciiTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  void CodingTest() {
    QString startOfBoldA = QChar{0xD835};
    startOfBoldA.append(QChar{0XDDD4});
    QCOMPARE(startOfBoldA, QString("𝗔"));

    QString startOfNotBoldA = QChar{0xD835};
    startOfNotBoldA.append(QChar{0XDC00});
    QCOMPARE(startOfNotBoldA, QString("𝐀"));
  }

  void charsetBoldUppercase() {
    const QString& actual = ConvertUnicodeCharsetToAscii()("𝗔𝗕𝗖𝗗𝗘𝗙𝗚𝗛𝗜𝗝𝗞𝗟𝗠𝗡𝗢𝗣𝗤𝗥𝗦𝗧𝗨𝗩𝗪𝗫𝗬𝗭");
    const QString& expect = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QCOMPARE(expect, actual);
  }

  void charsetBoldLowercase() {
    const QString& actual = ConvertUnicodeCharsetToAscii()("𝗮𝗯𝗰𝗱𝗲𝗳𝗴𝗵𝗶𝗷𝗸𝗹𝗺𝗻𝗼𝗽𝗾𝗿𝘀𝘁𝘂𝘃𝘄𝘅𝘆𝘇");
    const QString& expect = "abcdefghijklmnopqrstuvwxyz";
    QCOMPARE(expect, actual);
  }

  void charsetNormalUppercase() {
    const QString& actual = ConvertUnicodeCharsetToAscii()("𝐀𝐂𝐃𝐅𝐆");
    const QString& expect = "ACDFG";
    QCOMPARE(expect, actual);
  }

  void charsetNormalLowercase() {
    const QString& actual = ConvertUnicodeCharsetToAscii()("𝐚𝐝𝐞𝐠𝐡");
    const QString& expect = "adegh";
    QCOMPARE(expect, actual);
  }

  void charsetItalicUppercase() {
    const QString& actual = ConvertUnicodeCharsetToAscii()("𝑷𝑨𝑫𝑫𝒀");
    const QString& expect = "PADDY";
    QCOMPARE(expect, actual);
  }

  void skipExtraUnicodeCharset() {
    const QString& actual = ConvertUnicodeCharsetToAscii()("𝐂𝗗𝗘𝐬𝐭𝐰 - ABC");
    const QString& expect = "CDEstw - ABC";
    QCOMPARE(expect, actual);
  }
};

void ConvertUnicodeCharsetToAsciiTest::initTestCase() {}

void ConvertUnicodeCharsetToAsciiTest::cleanupTestCase() {}

void ConvertUnicodeCharsetToAsciiTest::init() {}

void ConvertUnicodeCharsetToAsciiTest::cleanup() {}

//QTEST_MAIN(ConvertUnicodeCharsetToAsciiTest)

//#include "ConvertUnicodeCharsetToAsciiTest.moc"
