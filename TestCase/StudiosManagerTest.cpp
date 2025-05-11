#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
// add necessary includes here
#include "Tools/StudiosManager.h"

class StudiosManagerTest : public MyTestSuite {
  Q_OBJECT
 public:
  StudiosManagerTest() : MyTestSuite{false} {}
 private slots:
  void test_performersDictNotEmpty() {
    const StudiosManager& psm{StudiosManager::getIns()};
    QVERIFY2(psm.count() > 0, "production studio hash table should not be empty");
  }
  void test_standardProdStudioNameFrom() {
    const StudiosManager& psm{StudiosManager::getIns()};
    const QStringList& fromLst = psm.GetCoarseStudioNames("LucasEntertainment");
    const QStringList expectCaurseStudiosName{"lucasentertainment", "lucas entertainment"};
    QCOMPARE(expectCaurseStudiosName, fromLst);

    const QStringList& fromLstWithADigit = psm.GetCoarseStudioNames("My9Inches");
    QVERIFY2(fromLstWithADigit.contains("my 9 inches"), "only lowercase edition");
    QVERIFY2(fromLstWithADigit.contains("my9inches"), "split by A-Z0-9 and join by space");

    const QStringList& fromLstWithNumber = psm.GetCoarseStudioNames("Studio2000");
    QVERIFY2(fromLstWithNumber.contains("studio2000"), "only lowercase edition");
    QVERIFY2(fromLstWithNumber.contains("studio 2000"), "split by A-Z0-9 and join by space");

    const QStringList& fromLstFullCapitalizer = psm.GetCoarseStudioNames("MEN");
    QVERIFY2(fromLstFullCapitalizer.contains("men"), "only lowercase edition");
    QVERIFY2(fromLstFullCapitalizer.contains("m e n"), "split by A-Z0-9 and join by space");
  }

  void test_filterProdStudioNameOut() {
    const StudiosManager& psm{StudiosManager::getIns()};
    QCOMPARE(psm("[FFL] Lucas Entertainment - ABC.mp4"), "LucasEntertainment");
    QCOMPARE(psm("[FFL] LucasEntertainment - ABC.mp4"), "LucasEntertainment");
    QCOMPARE(psm("[FFL] lucas entertainment - ABC.mp4"), "LucasEntertainment");
    QCOMPARE(psm("[FFL] lucasentertainment - ABC.mp4"), "LucasEntertainment");
    QCOMPARE(psm("[BaitBus] 2008 - Part.mp4"), "BaitBus");
    QCOMPARE(psm("[Bait Bus] 2008 - Part.mp4"), "BaitBus");
    QCOMPARE(psm("[bait bus] 2008 - Part.mp4"), "BaitBus");
    QCOMPARE(psm("[baitbus] 2008 - Part.mp4"), "BaitBus");
    QCOMPARE(psm("[treasure island media] 2008 - Part.mp4"), "TreasureIslandMedia");
    QCOMPARE(psm("too long long long long name - Part.mp4"), "");
  }

  void test_hintStudioName() {
    const StudiosManager& psm{StudiosManager::getIns()};
    QCOMPARE(psm.hintStdStudioName("not a studio map at all - 2008 - Part.mp4"), "");
    QCOMPARE(psm.hintStdStudioName("Men at play - 2008 - Part.mp4"), "MenAtPlay");
  }

};

StudiosManagerTest g_StudiosManagerTest;
#include "StudiosManagerTest.moc"
