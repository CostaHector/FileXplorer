#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"

// add necessary includes here
#include "Tools/CastManager.h"
class CastManagerTest : public MyTestSuite {
  Q_OBJECT
public:
  CastManagerTest() : MyTestSuite{true}, pm{CastManager::getIns()} {}
  const CastManager& pm;

private slots:
  void test_performersDictNotEmpty() {  //
    QVERIFY2(!pm.m_performers.isEmpty(), "performers list should not be empty");
  }
  void test_sentenceSplit() {
    const auto& wordsList = pm.SplitSentence("Production Name - Movie Core Name - Nice & Devin Franco BB 4K");
    QVERIFY2(not wordsList.isEmpty(), "words list should not empty");
  }

  void test_filterOutNameFromWordsList() {
    const auto& wordsList = pm.SplitSentence({"Jesse Theo Brady Devin Franco Vincent O'Reilly Topher Di Maggio Aspen"});
    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QSet<QString>& perfsSet{perfsList.cbegin(), perfsList.cend()};
    QVERIFY2(perfsList.size() >= 4, "perfs list should contains >= 5 performers");
    // 2 word cast are allowed, 1 word cast not allowed
    const QSet<QString>& realPerfs{"Theo Brady", "Devin Franco", "Vincent O'Reilly", "Topher Di Maggio"};
    QCOMPARE(perfsSet, realPerfs);
  }

  void test_OneLetterXNameCommaAddSep() {
    QVERIFY(pm.m_performers.contains("gabriel clark"));
    QVERIFY(pm.m_performers.contains("cutler x"));
    QVERIFY(pm.m_performers.contains("theo brady"));

    const auto& wordsList = pm.SplitSentence({"Gabriel Clark, Cutler X + Theo Brady"});
    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QStringList expectCasts{"Gabriel Clark", "Cutler X", "Theo Brady"};
    QCOMPARE(perfsList, expectCasts);
  }

  void test_cast_emdwith_comma_Period_colon_exclamation_question() {
    QVERIFY(pm.m_performers.contains("gabriel clark"));
    QVERIFY(pm.m_performers.contains("cutler x"));
    QVERIFY(pm.m_performers.contains("theo brady"));
    QVERIFY(pm.m_performers.contains("britishtwunk"));
    const auto& wordsList = pm.SplitSentence({"Gabriel Clark! Cutler X. Theo Brady; britishtwunk?"});
    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QStringList expectCasts{"Gabriel Clark", "Cutler X", "Theo Brady", "britishtwunk"};
    QCOMPARE(perfsList, expectCasts);
  }

  void test_sentenceWithPerfsAtLast() {
    QVERIFY(pm.m_performers.contains("britishtwunk"));
    QVERIFY(pm.m_performers.contains("craig kennedy"));
    const auto& wordsList = pm.SplitSentence("britishtwunk fucked by Craig Kennedy");
    QVERIFY2(!wordsList.isEmpty(), "words list should not empty");

    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QStringList expectCasts{"britishtwunk", "Craig Kennedy"};
    QCOMPARE(perfsList, expectCasts);
  }

  void test_sentenceWithNewLineSeperator() {
    QVERIFY(pm.m_performers.contains("britishtwunk"));
    QVERIFY(pm.m_performers.contains("craig kennedy"));
    QVERIFY(pm.m_performers.contains("gabriel clark"));
    const auto& wordsList = pm.SplitSentence("britishtwunk\r\nCraig Kennedy\nGabriel Clark");
    QVERIFY2(not wordsList.isEmpty(), "words list should not empty");

    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QStringList expectCasts{"britishtwunk", "Craig Kennedy", "Gabriel Clark"};
    QCOMPARE(perfsList, expectCasts);
  }
};

#include "CastManagerTest.moc"
CastManagerTest g_CastManagerTest;
