#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "Component/PerformersManager.h"

class PerformersManagerTest : public QObject {
  Q_OBJECT
 public:
  explicit PerformersManagerTest(QObject* parent = nullptr) : QObject(parent), pm(PerformersManager::getIns()) {}
  PerformersManager& pm;

 private slots:
  void test_performersDictNotEmpty() { QVERIFY2(not pm.performers.isEmpty(), "performers list should not be empty"); }
  void test_sentenseSplit() {
    const auto& wordsList = pm.MovieNameWordsSplit("Production Name - Movie Core Name - Nice & Devin Franco BB 4K");
    QVERIFY2(not wordsList.isEmpty(), "words list should not empty");
  }
  void test_filterOutNameFromWordsList() {
    const auto& wordsList = pm.MovieNameWordsSplit({"Jesse Theo Brady Devin Franco Vincent O'Reilly Topher Di Maggio Aspen"});
    const auto& perfsList = pm.PeformersFilterOut(wordsList);
    const QSet<QString>& perfsSet{perfsList.cbegin(), perfsList.cend()};
    QVERIFY2(perfsList.size() >= 5, "perfs list should contains >= 5 performers");

    const QSet<QString>& realPerfs{{"Jesse"}, {"Theo Brady"}, {"Devin Franco"}, {"Vincent O'Reilly"}, {"Topher Di Maggio"}, {"Aspen"}};
    QCOMPARE(perfsSet, realPerfs);
  }
};

QTEST_MAIN(PerformersManagerTest)

#include "PerformersManagerTest.moc"
