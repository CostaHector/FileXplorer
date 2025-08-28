#include <QCoreApplication>
#include <QtTest>
#include "MyTestSuite.h"
// add necessary includes here
#include "QuickWhereClauseHelper.h"

using namespace QuickWhereClauseHelper;
class QuickWhereClauseHelperTest : public MyTestSuite {
  Q_OBJECT
public:
  QuickWhereClauseHelperTest() : MyTestSuite{false} {}
private slots:
  void test_SearchSpaceString() {
    const QString& sSpace = "  ";
    const QString& whereClause = InfixNotation2RPN2Value("Name", sSpace, FUZZY_LIKE);
    QCOMPARE(whereClause, R"(INSTR(`Name`,"  ")>0)");
  }

  void test_SearchEmptyString() {
    const QString& sEmpty = "";
    const QString& whereClause = InfixNotation2RPN2Value("Name", sEmpty, FUZZY_LIKE);
    QCOMPARE(whereClause, "");
  }

  void test_SearchOnePerf() {
    const QString& s1 = "(Ricky Martin)";
    const QString& whereClause = InfixNotation2RPN2Value("Name", s1, FUZZY_LIKE);
    QCOMPARE(whereClause, R"(INSTR(`Name`,"Ricky Martin")>0)");
  }

  void SearchTwoPerfAAndB() {
    const QString& s2 = "Ricky Martin&Darin Zanyar";
    const QString& whereClause = InfixNotation2RPN2Value("Name", s2, FUZZY_LIKE);
    QCOMPARE(whereClause, R"((INSTR(`Name`,"Ricky Martin")>0 AND INSTR(`Name`,"Darin Zanyar")>0))");
  }

  void SearchTwoPerfAOrB() {
    const QString& s2 = "Ricky Martin|Darin Zanyar";
    const QString& whereClause = InfixNotation2RPN2Value("Name", s2, FUZZY_LIKE);
    QCOMPARE(whereClause, R"((INSTR(`Name`,"Ricky Martin")>0 OR INSTR(`Name`,"Darin Zanyar")>0))");
  }

  void SearchTwoPerfA1AkaA2OrB() {
    const QString& s2 = "(Ricky Martin|Ricky)&Darin Zanyar";
    const QString& whereClause = InfixNotation2RPN2Value("Name", s2, FUZZY_LIKE);
    QCOMPARE(whereClause, R"(((INSTR(`Name`,"Ricky Martin")>0 OR INSTR(`Name`,"Ricky")>0) AND INSTR(`Name`,"Darin Zanyar")>0))");
  }

  void SearchThreePerfsABC() {
    const QString& s3 = "A&B&C";
    const QString& whereClause = InfixNotation2RPN2Value("Name", s3, FUZZY_LIKE);
    QCOMPARE(whereClause, R"(((INSTR(`Name`,"A")>0 AND INSTR(`Name`,"B")>0) AND INSTR(`Name`,"C")>0))");
  }

  void SearchSizeRelationExpression() {
    const QString& rangeStr = ">=10&<50|=99";
    const QString& whereClause = InfixNotation2RPN2Value("Duration", rangeStr, OPEATOR_RELATION);
    QCOMPARE(whereClause, R"(((`Duration`>=10 AND `Duration`<50) OR `Duration`=99))");
  }

  void test_GetNameSelectStatement_with_akas() {
    QString aCastName{"Chris Evans"};
    QString akas{"Captain America\nSuper Hero"};
    const QString selectStatement{GetSelectMovieByCastStatement(aCastName, akas, "AnyTableName1")};
    const QString expectStatement{
      R"(SELECT `PrePathLeft`, `PrePathRight`, `Name` FROM `AnyTableName1` WHERE ((INSTR(`Name`,"Chris Evans")>0 OR INSTR(`Name`,"Captain America")>0) OR INSTR(`Name`,"Super Hero")>0))"
    };
    QCOMPARE(selectStatement, expectStatement);
  }

  void test_GetNameSelectStatement_with_no_akas() {
    QString aCastName{"Chris Evans"};
    QString noAkas;
    const QString selectStatement{GetSelectMovieByCastStatement(aCastName, noAkas, "AnyTableName2")};
    const QString expectStatement{
        R"(SELECT `PrePathLeft`, `PrePathRight`, `Name` FROM `AnyTableName2` WHERE INSTR(`Name`,"Chris Evans")>0)"
    };
    QCOMPARE(selectStatement, expectStatement);
  }
};

#include "QuickWhereClauseHelperTest.moc"
QuickWhereClauseHelperTest g_QuickWhereClauseHelperTest;
