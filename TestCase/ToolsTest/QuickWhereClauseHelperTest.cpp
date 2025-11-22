#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
// add necessary includes here
#include "QuickWhereClauseHelper.h"

using namespace QuickWhereClauseHelper;
class QuickWhereClauseHelperTest : public PlainTestSuite {
  Q_OBJECT
public:
  QuickWhereClauseHelperTest() : PlainTestSuite{} {}
private slots:
  void test_SearchSpaceString() {
    const QString& sSpace = "  ";
    const QString& instrWhereClause = InfixNotation2RPN2Value(sSpace, FUZZY_INSTR.arg("Name", "%1"));
    QCOMPARE(instrWhereClause, R"(INSTR(`Name`,"  ")>0)");

    const QString& likewhereClause = InfixNotation2RPN2Value(sSpace, FUZZY_LIKE.arg("Name", "%1"));
    QCOMPARE(likewhereClause, R"(`Name` LIKE "%  %")");
  }

  void test_SearchEmptyString() {
    const QString& sEmpty = "";
    const QString& instrWhereClause = InfixNotation2RPN2Value(sEmpty, FUZZY_INSTR.arg("Name", "%1"));
    QCOMPARE(instrWhereClause, "");

    const QString& likewhereClause = InfixNotation2RPN2Value(sEmpty, FUZZY_LIKE.arg("Name", "%1"));
    QCOMPARE(likewhereClause, "");
  }

  void test_SearchOnePerf() {
    const QString& s1 = "(Ricky Martin)";
    const QString& instrWhereClause = InfixNotation2RPN2Value(s1, FUZZY_INSTR.arg("Name", "%1"));
    QCOMPARE(instrWhereClause, R"(INSTR(`Name`,"Ricky Martin")>0)");

    const QString& likewhereClause = InfixNotation2RPN2Value(s1, FUZZY_LIKE.arg("Name", "%1"));
    QCOMPARE(likewhereClause, R"(`Name` LIKE "%Ricky Martin%")");
  }

  void SearchTwoPerfAAndB() {
    const QString& s2 = "Ricky Martin&Darin Zanyar";
    const QString& instrWhereClause = InfixNotation2RPN2Value(s2, FUZZY_INSTR.arg("Name", "%1"));
    QCOMPARE(instrWhereClause, R"((INSTR(`Name`,"Ricky Martin")>0 AND INSTR(`Name`,"Darin Zanyar")>0))");

    const QString& likewhereClause = InfixNotation2RPN2Value(s2, FUZZY_LIKE.arg("Name", "%1"));
    QCOMPARE(likewhereClause, R"((`Name` LIKE "%Ricky Martin%" AND `Name` LIKE "%Darin Zanyar%"))");
  }

  void SearchTwoPerfAOrB() {
    const QString& s2 = "Ricky Martin|Darin Zanyar";
    const QString& whereClause = InfixNotation2RPN2Value(s2, FUZZY_INSTR.arg("Name", "%1"));
    QCOMPARE(whereClause, R"((INSTR(`Name`,"Ricky Martin")>0 OR INSTR(`Name`,"Darin Zanyar")>0))");

    const QString& likewhereClause = InfixNotation2RPN2Value(s2, FUZZY_LIKE.arg("Name", "%1"));
    QCOMPARE(likewhereClause, R"((`Name` LIKE "%Ricky Martin%" OR `Name` LIKE "%Darin Zanyar%"))");
  }

  void SearchTwoPerfA1AkaA2OrB() {
    const QString& s2 = "(Ricky Martin|Ricky)&Darin Zanyar";
    const QString& whereClause = InfixNotation2RPN2Value(s2, FUZZY_INSTR.arg("Name", "%1"));
    QCOMPARE(whereClause, R"(((INSTR(`Name`,"Ricky Martin")>0 OR INSTR(`Name`,"Ricky")>0) AND INSTR(`Name`,"Darin Zanyar")>0))");

    const QString& likewhereClause = InfixNotation2RPN2Value(s2, FUZZY_LIKE.arg("Name", "%1"));
    QCOMPARE(likewhereClause, R"(((`Name` LIKE "%Ricky Martin%" OR `Name` LIKE "%Ricky%") AND `Name` LIKE "%Darin Zanyar%"))");
  }

  void SearchThreePerfsABC() {
    const QString& s3 = "A&B&C";
    const QString& whereClause = InfixNotation2RPN2Value(s3, FUZZY_INSTR.arg("Name", "%1"));
    QCOMPARE(whereClause, R"(((INSTR(`Name`,"A")>0 AND INSTR(`Name`,"B")>0) AND INSTR(`Name`,"C")>0))");
  }

  void SearchSizeRelationExpression() {
    const QString& rangeStr = ">=10&<50|=99";
    const QString& whereClause = InfixNotation2RPN2Value(rangeStr, OPEATOR_RELATION.arg("Duration", "%1"));
    QCOMPARE(whereClause, R"(((`Duration`>=10 AND `Duration`<50) OR `Duration`=99))");
  }

  void test_GetNameSelectStatement_with_akas() {
    QString aCastName{"Chris Evans"};
    QString akas{"Captain America\nSuper Hero"};
    const QString selectStatement{GetSelectMovieByCastStatement(aCastName, akas, "AnyTableName1")};
    const QString expectStatement{
        R"(SELECT `PrePathLeft`, `PrePathRight`, `Name` FROM `AnyTableName1` WHERE ((`Name` LIKE "%Chris Evans%" OR `Name` LIKE "%Captain America%") OR `Name` LIKE "%Super Hero%"))"
    };
    QCOMPARE(selectStatement, expectStatement);
  }

  void test_GetNameSelectStatement_with_no_akas() {
    QString aCastName{"Chris Evans"};
    QString noAkas;
    const QString selectStatement{GetSelectMovieByCastStatement(aCastName, noAkas, "AnyTableName2")};
    const QString expectStatement{
        R"(SELECT `PrePathLeft`, `PrePathRight`, `Name` FROM `AnyTableName2` WHERE `Name` LIKE "%Chris Evans%")"
    };
    QCOMPARE(selectStatement, expectStatement);
  }
};

#include "QuickWhereClauseHelperTest.moc"
REGISTER_TEST(QuickWhereClauseHelperTest, false)
