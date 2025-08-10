#include <QtTest>
#include <QCoreApplication>
#include "TestCase/pubTestTool/MyTestSuite.h"
// add necessary includes here
#include "public/PublicVariable.h"
#include "Tools/FileDescriptor/DbManager.h"

using namespace JSON_RENAME_REGEX;
class JsonRenameRegexTest : public MyTestSuite {
  Q_OBJECT

 public:
  JsonRenameRegexTest() : MyTestSuite{false} {}
 private slots:
  void test_toUpper() {
    QString s{"abc123ABC!@#"};
    QCOMPARE(s.toUpper(), QString("ABC123ABC!@#"));
  }

  void test_folderDirname() {
#ifdef _WIN32
    QFileInfo cUsers("C:/Users");
    QVERIFY2(!cUsers.isRoot(), "[C:/Users] is not root.");

    QFileInfo cDisk("C:/");
    QVERIFY2(cDisk.isRoot(), "[C:/] is root.");
    QCOMPARE(cDisk.absolutePath(), QString("C:/"));
#else
    QFileInfo linuxHomePath("/home/path");
    QVERIFY2(not linuxHomePath.isRoot(), "[/home/path] is not root.");
    QCOMPARE(linuxHomePath.absolutePath(), QString("/home"));
#endif
  }

  void test_wholeWordMatch() {
    QRegularExpression pattern{"\\blib\\b"};

    QString sLibaray{"How to go to school library"};
    QRegularExpressionMatch noMatchRet = pattern.match(sLibaray, 0, QRegularExpression::MatchType::NormalMatch);
    QCOMPARE(noMatchRet.hasMatch(), false);

    QString sLib{"Where is lib file in?"};
    QRegularExpressionMatch matchRet = pattern.match(sLib, 0, QRegularExpression::MatchType::NormalMatch);
    QCOMPARE(matchRet.hasMatch(), true);
    QCOMPARE(matchRet.captured(), "lib");

    QRegularExpression caseInsenPattern{"\\blib\\b", QRegularExpression::PatternOption::CaseInsensitiveOption};
    QString sCaseLib{"Where is Lib file in?"};
    QRegularExpressionMatch caseInsenMatchRet = caseInsenPattern.match(sCaseLib, 0, QRegularExpression::MatchType::NormalMatch);
    QCOMPARE(caseInsenMatchRet.hasMatch(), true);
    QCOMPARE(caseInsenMatchRet.captured(), "Lib");
  }

  void test_exactSentenceMatch() {
    QRegularExpression multiLinePattern{QRegularExpression::anchoredPattern("lib start")};
    QRegularExpression multiLinePatternAka{"^lib start$"};

    QString exactMatchStr{"lib start"};
    QRegularExpressionMatch multiLineMatchRet = multiLinePattern.match(exactMatchStr);
    QRegularExpressionMatch multiLineMatchRetAka = multiLinePatternAka.match(exactMatchStr);
    QCOMPARE(multiLineMatchRet.hasMatch(), true);
    QCOMPARE(multiLineMatchRetAka.hasMatch(), true);

    QString noExactMatchStr{"where is lib nstart begin"};
    QRegularExpressionMatch noMultiLineMatchRet = multiLinePattern.match(noExactMatchStr);
    QRegularExpressionMatch noMultiLineMatchAkaRet = multiLinePatternAka.match(noExactMatchStr);
    QCOMPARE(noMultiLineMatchRet.hasMatch(), false);
    QCOMPARE(noMultiLineMatchAkaRet.hasMatch(), false);
  }

  void test_table_name_match() {
    QRegularExpression invalidRegex{"(DB]"};
    QVERIFY(!DbManager::IsMatch("(DB]", invalidRegex));

    QRegularExpression regexDB{".*?DB.*?"};
    QVERIFY(!DbManager::IsMatch("db", regexDB));  // case sensitive
    QVERIFY(DbManager::IsMatch("DB", regexDB));
    QVERIFY(DbManager::IsMatch("DB__", regexDB));
    QVERIFY(DbManager::IsMatch("__DB__", regexDB));
    QVERIFY(DbManager::IsMatch("HelloDBWorld", regexDB));

    QRegularExpression fullPatternDB{"^DB$"};
    QVERIFY(DbManager::IsMatch("DB", fullPatternDB));
    QVERIFY(!DbManager::IsMatch("db", fullPatternDB));  // case sensitive
    QVERIFY(!DbManager::IsMatch("DB__", fullPatternDB));
    QVERIFY(!DbManager::IsMatch("__DB", fullPatternDB));

    QRegularExpression plainPartPatternDB{"DB"};
    QVERIFY(DbManager::IsMatch("DB", plainPartPatternDB));
    QVERIFY(!DbManager::IsMatch("db", plainPartPatternDB));     // case sensitive
    QVERIFY(DbManager::IsMatch("DBrain", plainPartPatternDB));  // surprise! me match
    QVERIFY(DbManager::IsMatch("HARDB", plainPartPatternDB));   // surprise! me match
  }

  void test_INVALID_CHARS_IN_FILENAME() {
    QStringList invalidNames{"*", "<", ">", "|", ":", "?", "/", "\\"};
    for (const QString& s : invalidNames) {
      QVERIFY2(INVALID_CHARS_IN_FILENAME.match(s).hasMatch(), qPrintable(s));
    }
  }

  void test_INVALID_QUOTE_IN_FILENAME() {
    QStringList invalidNames{"\"", "’", "“", "”"};
    for (const QString& s : invalidNames) {
      QVERIFY2(INVALID_QUOTE_IN_FILENAME.match(s).hasMatch(), qPrintable(s));
    }
  }

  void test_TORRENT_LEADING_STR() {
    static const QRegularExpression TORRENT_LEADING_STR_AKA1("(^\\[GT\\])|(^\\[FFL\\])|(^\\[FL\\])", QRegularExpression::PatternOption::CaseInsensitiveOption);  // delete it
    static const QRegularExpression TORRENT_LEADING_STR_AKA2("^((\\[GT\\])|(\\[FFL\\])|(\\[FL\\]))", QRegularExpression::PatternOption::CaseInsensitiveOption);  // delete it
    QVERIFY(TORRENT_LEADING_STR_COMP.isValid());
    QVERIFY(TORRENT_LEADING_STR_AKA1.isValid());
    QVERIFY(TORRENT_LEADING_STR_AKA2.isValid());

    QStringList leadingLst{"[FL] French", "[FFL] Football", "[GT] Jock"};
    QStringList notLeadingLst{"Hello [FL]", "[FC world FL]", "Nice [GT]"};
    for (const QString& s : leadingLst) {
      QVERIFY2(TORRENT_LEADING_STR_COMP.match(s).hasMatch(), qPrintable(s));
      QVERIFY2(TORRENT_LEADING_STR_AKA1.match(s).hasMatch(), qPrintable(s));
      QVERIFY2(TORRENT_LEADING_STR_AKA2.match(s).hasMatch(), qPrintable(s));
    }
    for (const QString& s : notLeadingLst) {
      QVERIFY2(!TORRENT_LEADING_STR_COMP.match(s).hasMatch(), qPrintable(s));
      QVERIFY2(!TORRENT_LEADING_STR_AKA1.match(s).hasMatch(), qPrintable(s));
      QVERIFY2(!TORRENT_LEADING_STR_AKA2.match(s).hasMatch(), qPrintable(s));
    }
  }

  void test_leadingOpenBracketComp() {
    QVERIFY(LEADING_OPEN_BRACKET_COMP.isValid());
    QStringList leadingLst{"[French", "{Football", "(Jock"};
    QStringList noleadingLst{" [French", "X {Football", "J("};

    for (const QString& s : leadingLst) {
      QVERIFY2(LEADING_OPEN_BRACKET_COMP.match(s).hasMatch(), qPrintable(s));
    }
    for (const QString& s : noleadingLst) {
      QVERIFY2(!LEADING_OPEN_BRACKET_COMP.match(s).hasMatch(), qPrintable(s));
    }
  }

  void test_continuousSpaceComp() {
    QVERIFY(AT_LEAST_1_SPACE_COMP.isValid());
    QVERIFY(CONTINOUS_SPACE_COMP.isValid());

    QString fileNameWithMoreBlank{"01 2  3   4    5     .txt"};
    const QString fileNameWith1Blank{"01 2 3 4 5 .txt"};

    const QStringList actuals = fileNameWithMoreBlank.split(AT_LEAST_1_SPACE_COMP);
    const QStringList actuals2 = fileNameWith1Blank.split(AT_LEAST_1_SPACE_COMP);
    const QStringList expects{"01", "2", "3", "4", "5", ".txt"};
    QCOMPARE(actuals, expects);
    QCOMPARE(actuals2, expects);

    fileNameWithMoreBlank.replace(CONTINOUS_SPACE_COMP, " ");
    QCOMPARE(fileNameWithMoreBlank, fileNameWith1Blank);
  }

  void test_nonLeadingBracketComp() {
    QVERIFY(NON_LEADING_BRACKET_COMP.isValid());
    QString fileNameWithBracked{"To be or not to be, that is the question(Shakespear)"};
    const QString noBracket{"To be or not to be, that is the question-Shakespear-"};
    fileNameWithBracked.replace(NON_LEADING_BRACKET_COMP, "-");
    QCOMPARE(fileNameWithBracked, noBracket);

    QStringList bracketsLst{"[French]", "{Football}", "(Jock)"};
    QStringList expectAns{"-French-", "-Football-", "-Jock-"};
    for (int i = 0; i < bracketsLst.size(); ++i) {
      const QString& actual = bracketsLst[i].replace(NON_LEADING_BRACKET_COMP, "-");
      QCOMPARE(actual, expectAns[i]);
    }
  }
  void test_spaceBarSpaceComp() {
    QVERIFY(SPACE_HYPEN_SPACE_COMP.isValid());

    QString fileNameWithSpaceHypen{"Hello -  world, Good  - evening"};
    const QString noSpaceHypen{"Hello-world, Good-evening"};
    fileNameWithSpaceHypen.replace(SPACE_HYPEN_SPACE_COMP, "-");
    QCOMPARE(fileNameWithSpaceHypen, noSpaceHypen);
  }

  void test_continousHypenComp() {
    QVERIFY(CONTINOUS_HYPEN_COMP.isValid());

    QString fileNameContinousHypen{"Hello --  world, Good  --- evening"};
    const QString noContinousHypen{"Hello -  world, Good  - evening"};
    fileNameContinousHypen.replace(CONTINOUS_HYPEN_COMP, "-");
    QCOMPARE(fileNameContinousHypen, noContinousHypen);
  }

  void test_hypenOrSpaceFollowedWithDotPat() {
    QVERIFY(HYPEN_OR_SPACE_END_WITH_DOT_COMP.isValid());

    QString fileNameHypenOrSpaceFollowedWithDot{"Hello world-.cpp, Hello world .hpp"};
    const QString noHypenOrSpaceFollowedWithDot{"Hello world.cpp, Hello world.hpp"};
    fileNameHypenOrSpaceFollowedWithDot.replace(HYPEN_OR_SPACE_END_WITH_DOT_COMP, ".");
    QCOMPARE(fileNameHypenOrSpaceFollowedWithDot, noHypenOrSpaceFollowedWithDot);
  }

  void test_trailingHypenComp() {
    QVERIFY(TRAILING_HYPEN_COMP.isValid());

    QString fileNametrailingHypen{"Hello world-"};
    const QString noTrailingHypen{"Hello world"};
    fileNametrailingHypen.replace(TRAILING_HYPEN_COMP, "");
    QCOMPARE(fileNametrailingHypen, noTrailingHypen);
  }

  void test_DISCRAD_LETTER_COMP() {
    QVERIFY(DISCRAD_LETTER_COMP.isValid());
    QString fileNameWithDiscardChar{"Hello <|/\\?:>world.;"};
    const QString noDiscardChar{"Hello world"};
    fileNameWithDiscardChar.replace(DISCRAD_LETTER_COMP, "");
    QCOMPARE(fileNameWithDiscardChar, noDiscardChar);
  }

  void test_INVALID_TABLE_NAME_LETTER() {
    QVERIFY(INVALID_TABLE_NAME_LETTER.isValid());
    QString fileNameWithInvalidTableChar{"09Hellor_<|/\\?:>world"};
    const QString noWith{"09Hellor_world"};
    fileNameWithInvalidTableChar.replace(INVALID_TABLE_NAME_LETTER, "");
    QCOMPARE(fileNameWithInvalidTableChar, noWith);
  }

  void test_AND_COMP() {
    QVERIFY(AND_COMP.isValid());
    QString fileNameWithAnd{"first day and second day a pool guy get fucked by life once and again"};
    const QString noWith{"first day&second day a pool guy get&life once&again"};
    fileNameWithAnd.replace(AND_COMP, "&");
    QCOMPARE(fileNameWithAnd, noWith);
  }

  void test_RESOLUTION_COMP() {
    QVERIFY(RESOLUTION_COMP.isValid());
    QString fileNameWithRes{"Henry Cavill 2160p 1080p 360p 480p 720p 810p 4K FHD HD SD.mp4"};
    const QString noWith{"Henry Cavill          .mp4"};
    fileNameWithRes.replace(RESOLUTION_COMP, "");
    QCOMPARE(fileNameWithRes, noWith);
  }

  void test_SPLIT_BY_UPPERCASE() {
    QVERIFY(SPLIT_BY_UPPERCASE_COMP1.isValid());
    QVERIFY(SPLIT_BY_UPPERCASE_COMP2.isValid());

    QString before = "KobiIsOneOfMyFavoriteBasketballPlayer";
    const QString after = "Kobi Is One Of My Favorite Basketball Player";
    before.replace(SPLIT_BY_UPPERCASE_COMP1, "\\1 \\2");
    QCOMPARE(before, after);

    QString sentence{"RealMadridCF"};
    sentence.replace(SPLIT_BY_UPPERCASE_COMP1, "\\1 \\2");
    QCOMPARE(sentence, "Real Madrid CF");

    QString studio2000Sentence{"Studio2000"};
    studio2000Sentence.replace(SPLIT_BY_UPPERCASE_COMP1, "\\1 \\2");
    QCOMPARE(studio2000Sentence, "Studio2000");

    QString my7InchesSentence{"My7Gifts"};
    my7InchesSentence.replace(SPLIT_BY_UPPERCASE_COMP1, "\\1 \\2");
    my7InchesSentence.replace(SPLIT_BY_UPPERCASE_COMP2, "\\1 \\2");
    QCOMPARE(my7InchesSentence, "My 7 Gifts");

    QString my2BSentence{"2B"};
    my2BSentence.replace(SPLIT_BY_UPPERCASE_COMP1, "\\1 \\2");
    my2BSentence.replace(SPLIT_BY_UPPERCASE_COMP2, "\\1 \\2");
    QCOMPARE(my2BSentence, "2 B");
  }
};

#include "JsonRenameRegexTest.moc"
JsonRenameRegexTest g_JsonRenameRegexTest;
