#include <QCoreApplication>
#include <QtTest>
#include "PublicVariable.h"

// add necessary includes here
#include "Component/QuickWhereClause.h"
class QuickWhereClauseTest : public QObject {
  Q_OBJECT

 public:
 private slots:
  void test_SearchSpaceString();
  void test_SearchEmptyString();
  void test_SearchOnePerf();
  void SearchTwoPerfAAndB();
  void SearchTwoPerfAOrB();
  void SearchTwoPerfA1AkaA2OrB();
  void SearchThreePerfsABC();
};

void QuickWhereClauseTest::test_SearchSpaceString() {
  const QString& sSpace = "  ";
  const QString& whereClause = QuickWhereClause::PlainLogicSentence2FuzzySqlWhere(sSpace);
  QCOMPARE(whereClause, "Name like \"%  %\"");
}

void QuickWhereClauseTest::test_SearchEmptyString() {
  const QString& sEmpty = "";
  const QString& whereClause = QuickWhereClause::PlainLogicSentence2FuzzySqlWhere(sEmpty);
  QCOMPARE(whereClause, "");
}

void QuickWhereClauseTest::test_SearchOnePerf() {
  const QString& s1 = "(Ricky Martin)";
  const QString& whereClause = QuickWhereClause::PlainLogicSentence2FuzzySqlWhere(s1);
  QCOMPARE(whereClause, "Name like \"%Ricky Martin%\"");
}

void QuickWhereClauseTest::SearchTwoPerfAAndB() {
  const QString& s2 = "Ricky Martin&Darin Zanyar";
  const QString& whereClause = QuickWhereClause::PlainLogicSentence2FuzzySqlWhere(s2);
  QCOMPARE(whereClause, "(Name like \"%Ricky Martin%\" AND Name like \"%Darin Zanyar%\")");
}

void QuickWhereClauseTest::SearchTwoPerfAOrB() {
  const QString& s2 = "Ricky Martin|Darin Zanyar";
  const QString& whereClause = QuickWhereClause::PlainLogicSentence2FuzzySqlWhere(s2);
  QCOMPARE(whereClause, "(Name like \"%Ricky Martin%\" OR Name like \"%Darin Zanyar%\")");
}

void QuickWhereClauseTest::SearchTwoPerfA1AkaA2OrB() {
  const QString& s2 = "(Ricky Martin|Ricky)&Darin Zanyar";
  const QString& whereClause = QuickWhereClause::PlainLogicSentence2FuzzySqlWhere(s2);
  QCOMPARE(whereClause, "((Name like \"%Ricky Martin%\" OR Name like \"%Ricky%\") AND Name like \"%Darin Zanyar%\")");
}

void QuickWhereClauseTest::SearchThreePerfsABC() {
  const QString& s3 = "A&B&C";
  const QString& whereClause = QuickWhereClause::PlainLogicSentence2FuzzySqlWhere(s3);
  QCOMPARE(whereClause, "((Name like \"%A%\" AND Name like \"%B%\") AND Name like \"%C%\")");
}

//QTEST_MAIN(QuickWhereClauseTest)

#include "QuickWhereClauseTest.moc"
