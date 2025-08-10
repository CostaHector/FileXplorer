#include <QCoreApplication>
#include <QtTest>
#include "MyTestSuite.h"
// add necessary includes here
#include "PerformersAkaManager.h"
class QuickWhereClauseTest : public MyTestSuite {
  Q_OBJECT

 public:
  QuickWhereClauseTest() : MyTestSuite{false} {}
 private slots:
  void test_SearchSpaceString() {
    PerformersAkaManager& dbTM{PerformersAkaManager::getIns()};
    const QString& sSpace = "  ";
    const QString& whereClause = dbTM.PlainLogicSentence2FuzzySqlWhere("Name", sSpace);
    QCOMPARE(whereClause, "Name like \"%  %\"");
  }

  void test_SearchEmptyString() {
    PerformersAkaManager& dbTM{PerformersAkaManager::getIns()};
    const QString& sEmpty = "";
    const QString& whereClause = dbTM.PlainLogicSentence2FuzzySqlWhere("Name", sEmpty);
    QCOMPARE(whereClause, "");
  }

  void test_SearchOnePerf() {
    PerformersAkaManager& dbTM{PerformersAkaManager::getIns()};
    const QString& s1 = "(Ricky Martin)";
    const QString& whereClause = dbTM.PlainLogicSentence2FuzzySqlWhere("Name", s1);
    QCOMPARE(whereClause, "Name like \"%Ricky Martin%\"");
  }

  void SearchTwoPerfAAndB() {
    PerformersAkaManager& dbTM{PerformersAkaManager::getIns()};
    const QString& s2 = "Ricky Martin&Darin Zanyar";
    const QString& whereClause = dbTM.PlainLogicSentence2FuzzySqlWhere("Name", s2);
    QCOMPARE(whereClause, "(Name like \"%Ricky Martin%\" AND Name like \"%Darin Zanyar%\")");
  }

  void SearchTwoPerfAOrB() {
    PerformersAkaManager& dbTM{PerformersAkaManager::getIns()};
    const QString& s2 = "Ricky Martin|Darin Zanyar";
    const QString& whereClause = dbTM.PlainLogicSentence2FuzzySqlWhere("Name", s2);
    QCOMPARE(whereClause, "(Name like \"%Ricky Martin%\" OR Name like \"%Darin Zanyar%\")");
  }

  void SearchTwoPerfA1AkaA2OrB() {
    PerformersAkaManager& dbTM{PerformersAkaManager::getIns()};
    const QString& s2 = "(Ricky Martin|Ricky)&Darin Zanyar";
    const QString& whereClause = dbTM.PlainLogicSentence2FuzzySqlWhere("Name", s2);
    QCOMPARE(whereClause, "((Name like \"%Ricky Martin%\" OR Name like \"%Ricky%\") AND Name like \"%Darin Zanyar%\")");
  }

  void SearchThreePerfsABC() {
    PerformersAkaManager& dbTM{PerformersAkaManager::getIns()};
    const QString& s3 = "A&B&C";
    const QString& whereClause = dbTM.PlainLogicSentence2FuzzySqlWhere("Name", s3);
    QCOMPARE(whereClause, "((Name like \"%A%\" AND Name like \"%B%\") AND Name like \"%C%\")");
  }

  void SearchSizeRelationExpression() {
    const QString& rangeStr = ">=10&<50|=99";
    const QString& whereClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere("Duration", rangeStr, "%1%2");
    QCOMPARE(whereClause, R"(((Duration>=10 AND Duration<50) OR Duration=99))");
  }

};

QuickWhereClauseTest g_QuickWhereClauseTest;
#include "QuickWhereClauseTest.moc"
