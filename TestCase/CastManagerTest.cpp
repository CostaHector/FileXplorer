#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"

// add necessary includes here
#include "Tools/CastManager.h"
class CastManagerTest : public MyTestSuite {
  Q_OBJECT
 public:
  CastManagerTest() : MyTestSuite{false}, pm{CastManager::getIns()} {}
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
    const QSet<QString>& realPerfs{{"Theo Brady"}, {"Devin Franco"}, {"Vincent O'Reilly"}, {"Topher Di Maggio"}};
    QCOMPARE(perfsSet, realPerfs);
  }

  void test_OneLetterXNameCommaAddSep() {
    const auto& wordsList = pm.SplitSentence({"Gabriel Clark, Cutler X + Theo Brady"});
    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QSet<QString>& perfsSet{perfsList.cbegin(), perfsList.cend()};
    QVERIFY2(perfsList.contains("Gabriel Clark"), "comma sep");
    QVERIFY2(perfsList.contains("Cutler X"), "add sep");
    QVERIFY2(perfsList.contains("Theo Brady"), "\\0 sep");
  }

  void test_sentenceWithPerfsAtLast() {
    const auto& wordsList = pm.SplitSentence("britishtwunk fucked by Craig Kennedy");
    QVERIFY2(not wordsList.isEmpty(), "words list should not empty");

    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QSet<QString>& perfsSet{perfsList.cbegin(), perfsList.cend()};
    QVERIFY2(perfsList.size() >= 1, "perfs list should contains >= 1 performers");
  }

  void test_sentenceWithNewLineSeperator() {
    const auto& wordsList = pm.SplitSentence("britishtwunk\r\nCraig Kennedy\nGabriel Clark");
    QVERIFY2(not wordsList.isEmpty(), "words list should not empty");

    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QSet<QString>& perfsSet{perfsList.cbegin(), perfsList.cend()};
    QVERIFY2(perfsList.size() >= 1, "perfs list should contains >= 1 performers");
  }
};

#include "CastManagerTest.moc"
CastManagerTest g_CastManagerTest;
