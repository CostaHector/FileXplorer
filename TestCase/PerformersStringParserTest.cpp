#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "Tools/PerformersStringParser.h"

class PerformersManagerTest : public QObject {
  Q_OBJECT
 public:
  explicit PerformersManagerTest(QObject* parent = nullptr) : QObject(parent), pm(PerformersStringParser::getIns()) {}
  PerformersStringParser& pm;

 private slots:
  void test_performersDictNotEmpty() { QVERIFY2(not pm.m_performers.isEmpty(), "performers list should not be empty"); }
  void test_sentenseSplit() {
    const auto& wordsList = pm.SplitSentence("Production Name - Movie Core Name - Nice & Devin Franco BB 4K");
    QVERIFY2(not wordsList.isEmpty(), "words list should not empty");
  }
  void test_filterOutNameFromWordsList() {
    const auto& wordsList = pm.SplitSentence({"Jesse Theo Brady Devin Franco Vincent O'Reilly Topher Di Maggio Aspen"});
    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QSet<QString>& perfsSet{perfsList.cbegin(), perfsList.cend()};
    QVERIFY2(perfsList.size() >= 5, "perfs list should contains >= 5 performers");

    const QSet<QString>& realPerfs{{"Jesse"}, {"Theo Brady"}, {"Devin Franco"}, {"Vincent O'Reilly"}, {"Topher Di Maggio"}, {"Aspen"}};
    QCOMPARE(perfsSet, realPerfs);
  }
  void test_sentenseWithPerfsAtLast() {
    const auto& wordsList = pm.SplitSentence("britishtwunk fucked by Craig Kennedy");
    QVERIFY2(not wordsList.isEmpty(), "words list should not empty");

    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QSet<QString>& perfsSet{perfsList.cbegin(), perfsList.cend()};
    QVERIFY2(perfsList.size() >= 1, "perfs list should contains >= 1 performers");
  }

  void test_sentenseWithNewLineSeperator() {
    const auto& wordsList = pm.SplitSentence("britishtwunk\r\nCraig Kennedy\nGabriel Clark");
    QVERIFY2(not wordsList.isEmpty(), "words list should not empty");

    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QSet<QString>& perfsSet{perfsList.cbegin(), perfsList.cend()};
    QVERIFY2(perfsList.size() >= 1, "perfs list should contains >= 1 performers");
  }
};

// QTEST_MAIN(PerformersManagerTest)
#include "PerformersStringParserTest.moc"
