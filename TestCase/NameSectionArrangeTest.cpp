#include <QCoreApplication>
#include <QtTest>

#include "Tools/NameSectionArrange.h"

class NameSectionArrangeTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  void arrange0213Basic() {
    NameSectionArrange nss("0213");

    QString actual = nss("1-2-3-4");
    QString expect("1 - 3 - 2 - 4");
    QCOMPARE(expect, actual);
  }

  void arrangeWastedPattern40123BasicStrict() {
    NameSectionArrange nss("4012");
    QString actual = nss("1-2-3-4");
    QString expect("1-2-3-4");
    QCOMPARE(expect, actual);
  }

  void arrangeRedundantPattern40213BasicLoose() {
    NameSectionArrange nss("40213", false);
    QString actual = nss("1-2-3-4");
    QString expect("1 - 3 - 2 - 4");
    QCOMPARE(expect, actual);
  }

  void arrangeWastedSectionPattern021() {
    NameSectionArrange nss("4021", false);
    QStringList actual = nss.BatchSwapper({"1-2-3-4", "A-B-C-D", "X - Y - Z"});
    QStringList expect{"1 - 3 - 2", "A - C - B", "X - Z - Y"};
    QCOMPARE(nss.m_wastedList.size(), 2);
    QCOMPARE(expect, actual);
  }

  void arrangeSwapper12() {
    NameSectionArrange nss(1, 2, false);
    QStringList actual = nss.BatchSwapper({"1-2-3-4", "A-B-C-D", "X - Y - Z"});
    QStringList expect{"1 - 3 - 2 - 4", "A - C - B - D", "X - Z - Y"};
    QCOMPARE(nss.m_wastedList.size(), 0);
    QCOMPARE(expect, actual);
  }
};

void NameSectionArrangeTest::initTestCase() {}

void NameSectionArrangeTest::cleanupTestCase() {}

void NameSectionArrangeTest::init() {}

void NameSectionArrangeTest::cleanup() {}

QTEST_MAIN(NameSectionArrangeTest)

#include "NameSectionArrangeTest.moc"
