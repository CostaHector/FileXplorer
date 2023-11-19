#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "Component/ProductionStudioManager.h"

class ProductionStudioManagerTest : public QObject {
  Q_OBJECT
 public:
  explicit ProductionStudioManagerTest(QObject* parent = nullptr) : QObject(parent), psm(ProductionStudioManager::getIns()) {}
  ProductionStudioManager& psm;

 private slots:
  void test_performersDictNotEmpty() { QVERIFY2(not psm.m_prodStudioMap.isEmpty(), "production studio hash table should not be empty"); }
  void test_standardProdStudioNameFrom(){
    const QStringList& fromLst = psm.StandardProductionStudioFrom("LucasEntertainment");
    QVERIFY2(fromLst.contains("lucasentertainment"), "only lowercase edition");
    QVERIFY2(fromLst.contains("lucas entertainment"), "split by A-Z and join by space");
  }
  void test_filterProdStudioNameOut(){
    QCOMPARE(psm("[FFL] Lucas Entertainment - ABC.mp4"), "LucasEntertainment");
    QCOMPARE(psm("[FFL] LucasEntertainment - ABC.mp4"), "LucasEntertainment");
    QCOMPARE(psm("[FFL] lucas entertainment - ABC.mp4"), "LucasEntertainment");
    QCOMPARE(psm("[FFL] lucasentertainment - ABC.mp4"), "LucasEntertainment");
    QCOMPARE(psm("[BaitBus] 2008 - Part.mp4"), "BaitBus");
    QCOMPARE(psm("[Bait Bus] 2008 - Part.mp4"), "BaitBus");
    QCOMPARE(psm("[bait bus] 2008 - Part.mp4"), "BaitBus");
    QCOMPARE(psm("[baitbus] 2008 - Part.mp4"), "BaitBus");
  }
};

QTEST_MAIN(ProductionStudioManagerTest)

#include "ProductionStudioManagerTest.moc"
