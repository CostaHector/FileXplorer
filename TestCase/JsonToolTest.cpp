#include <QtTest>
#include <QCoreApplication>
#include "Tools/JsonFileHelper.h"

class JsonToolTest : public QObject
{
  Q_OBJECT

public:

private slots:
  void initTestCase();
  void cleanupTestCase();
  void test_existedKeySorter();
  void test_inexistedKeySorter();

};

void JsonToolTest::initTestCase()
{

}

void JsonToolTest::cleanupTestCase()
{

}
using namespace JSONKey;

void JsonToolTest::test_existedKeySorter() {
  QList<QPair<QString, QVariant>> prs;
  prs << qMakePair(Detail, QString("This is the introduction"))
      << qMakePair(Rate, 10)
      << qMakePair(Name, QString("Do you like it"));
  std::sort(prs.begin(), prs.end(), JsonKeySorter);

  QCOMPARE(prs.size(), 3);
  QCOMPARE(prs[0].first, Name);
  QCOMPARE(prs[1].first, Rate);
  QCOMPARE(prs[2].first, Detail);
}

void JsonToolTest::test_inexistedKeySorter() {
  QList<QPair<QString, QVariant>> prs;
  prs << qMakePair(QString("any key"), QString("any name"))
      << qMakePair(Name, QString("Do you like it"));
  std::sort(prs.begin(), prs.end(), JsonKeySorter);

  QCOMPARE(prs.size(), 2);
  QCOMPARE(prs[0].first, Name);
  QCOMPARE(prs[1].first, QString("any key"));
}

//QTEST_MAIN(JsonToolTest)
#include "JsonToolTest.moc"
