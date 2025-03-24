#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
// add necessary includes here
#include "Tools/ProductionStudioManager.h"

class ProductionStudioManagerTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_performersDictNotEmpty() {
    const ProductionStudioManager& psm{ProductionStudioManager::getIns()};
    QVERIFY2(psm.count() > 0, "production studio hash table should not be empty");
  }
  void test_standardProdStudioNameFrom() {
    const ProductionStudioManager& psm{ProductionStudioManager::getIns()};
    const QStringList& fromLst = psm.StandardProductionStudioFrom("LucasEntertainment");
    QVERIFY2(fromLst.contains("lucasentertainment"), "only lowercase edition");
    QVERIFY2(fromLst.contains("lucas entertainment"), "split by A-Z0-9 and join by space");

    const QStringList& fromLstWithADigit = psm.StandardProductionStudioFrom("My9Inches");
    QVERIFY2(fromLstWithADigit.contains("my 9 inches"), "only lowercase edition");
    QVERIFY2(fromLstWithADigit.contains("my9inches"), "split by A-Z0-9 and join by space");

    const QStringList& fromLstWithNumber = psm.StandardProductionStudioFrom("Studio2000");
    QVERIFY2(fromLstWithNumber.contains("studio2000"), "only lowercase edition");
    QVERIFY2(fromLstWithNumber.contains("studio 2000"), "split by A-Z0-9 and join by space");

    const QStringList& fromLstFullCapitalizer = psm.StandardProductionStudioFrom("MEN");
    QVERIFY2(fromLstFullCapitalizer.contains("men"), "only lowercase edition");
    QVERIFY2(fromLstFullCapitalizer.contains("m e n"), "split by A-Z0-9 and join by space");
  }

  void test_filterProdStudioNameOut() {
    const ProductionStudioManager& psm{ProductionStudioManager::getIns()};
    QCOMPARE(psm("[FFL] Lucas Entertainment - ABC.mp4"), "LucasEntertainment");
    QCOMPARE(psm("[FFL] LucasEntertainment - ABC.mp4"), "LucasEntertainment");
    QCOMPARE(psm("[FFL] lucas entertainment - ABC.mp4"), "LucasEntertainment");
    QCOMPARE(psm("[FFL] lucasentertainment - ABC.mp4"), "LucasEntertainment");
    QCOMPARE(psm("[BaitBus] 2008 - Part.mp4"), "BaitBus");
    QCOMPARE(psm("[Bait Bus] 2008 - Part.mp4"), "BaitBus");
    QCOMPARE(psm("[bait bus] 2008 - Part.mp4"), "BaitBus");
    QCOMPARE(psm("[baitbus] 2008 - Part.mp4"), "BaitBus");
  }

  void test_hintStudioName() {
    const ProductionStudioManager& psm{ProductionStudioManager::getIns()};
    QCOMPARE(psm.hintStdStudioName("not a studio map at all - 2008 - Part.mp4"), "");
    QCOMPARE(psm.hintStdStudioName("Men at play - 2008 - Part.mp4"), "MenAtPlay");
  }
};

ProductionStudioManagerTest g_ProductionStudioManagerTest;
#include "ProductionStudioManagerTest.moc"
