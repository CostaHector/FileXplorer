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
    QCOMPARE(whereClause, R"(INSTR(`Name`,"  ")>0)");
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
    QCOMPARE(whereClause, R"(INSTR(`Name`,"Ricky Martin")>0)");
  }

  void SearchTwoPerfAAndB() {
    PerformersAkaManager& dbTM{PerformersAkaManager::getIns()};
    const QString& s2 = "Ricky Martin&Darin Zanyar";
    const QString& whereClause = dbTM.PlainLogicSentence2FuzzySqlWhere("Name", s2);
    QCOMPARE(whereClause, R"((INSTR(`Name`,"Ricky Martin")>0 AND INSTR(`Name`,"Darin Zanyar")>0))");
  }

  void SearchTwoPerfAOrB() {
    PerformersAkaManager& dbTM{PerformersAkaManager::getIns()};
    const QString& s2 = "Ricky Martin|Darin Zanyar";
    const QString& whereClause = dbTM.PlainLogicSentence2FuzzySqlWhere("Name", s2);
    QCOMPARE(whereClause, R"((INSTR(`Name`,"Ricky Martin")>0 OR INSTR(`Name`,"Darin Zanyar")>0))");
  }

  void SearchTwoPerfA1AkaA2OrB() {
    PerformersAkaManager& dbTM{PerformersAkaManager::getIns()};
    const QString& s2 = "(Ricky Martin|Ricky)&Darin Zanyar";
    const QString& whereClause = dbTM.PlainLogicSentence2FuzzySqlWhere("Name", s2);
    QCOMPARE(whereClause, R"(((INSTR(`Name`,"Ricky Martin")>0 OR INSTR(`Name`,"Ricky")>0) AND INSTR(`Name`,"Darin Zanyar")>0))");
  }

  void SearchThreePerfsABC() {
    PerformersAkaManager& dbTM{PerformersAkaManager::getIns()};
    const QString& s3 = "A&B&C";
    const QString& whereClause = dbTM.PlainLogicSentence2FuzzySqlWhere("Name", s3);
    QCOMPARE(whereClause, R"(((INSTR(`Name`,"A")>0 AND INSTR(`Name`,"B")>0) AND INSTR(`Name`,"C")>0))");
  }

  void SearchSizeRelationExpression() {
    const QString& rangeStr = ">=10&<50|=99";
    const QString& whereClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere("Duration", rangeStr, "%1%2");
    QCOMPARE(whereClause, R"(((Duration>=10 AND Duration<50) OR Duration=99))");
  }

};

QuickWhereClauseTest g_QuickWhereClauseTest;
#include "QuickWhereClauseTest.moc"
