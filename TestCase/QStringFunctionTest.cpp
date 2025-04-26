#include <QtTest>
#include <QCoreApplication>
#include "pub/MyTestSuite.h"
// add necessary includes here
#include "Tools/FileDescriptor/DbManager.h"
#include <QRegularExpression>

class QStringFunctionTest : public MyTestSuite {
  Q_OBJECT

 public:
  void toUpper() {
    QString s{"abc123ABC!@#"};
    QCOMPARE(s.toUpper(), QString("ABC123ABC!@#"));
  }

 private slots:
  void initTestCase() {}

  void cleanupTestCase() {}

  void test_regexReplace() {
    QRegExp continousBlank("\\s\\s+");
    QString fileNameWithBlank("01 2  3   4    5     .txt");
    QString expectFileName("01 2 3 4 5 .txt");
    QString afterFileName = fileNameWithBlank.replace(continousBlank, " ");
    QCOMPARE(afterFileName, expectFileName);
  }

  void test_folderDirname() {
#ifdef _WIN32
    QFileInfo cUsers("C:/Users");
    QVERIFY2(not cUsers.isRoot(), "[C:/Users] is not root.");

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
};

QStringFunctionTest g_QStringFunctionTest;
#include "QStringFunctionTest.moc"
