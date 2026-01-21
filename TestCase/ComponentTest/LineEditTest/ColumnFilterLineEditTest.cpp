#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ColumnFilterLineEdit.h"
#include "EndToExposePrivateMember.h"

class ColumnFilterLineEditTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void GetConditionTemplate_ok() {
    QCOMPARE(ColumnFilterLineEdit::GetExprTypeFromColumnName("Name"), ColumnFilterLineEdit::EXPR_TYPE::STR_FUZZY_LIKE);
    QCOMPARE(ColumnFilterLineEdit::GetExprTypeFromColumnName("Size"), ColumnFilterLineEdit::EXPR_TYPE::NUMBER_RELATION);
    QCOMPARE(ColumnFilterLineEdit::GetExprTypeFromColumnName("preserved_for_future"), ColumnFilterLineEdit::EXPR_TYPE::STR_FUZZY_LIKE);

    ColumnFilterLineEdit filterLineEdit{"CASE_SENSITVE_COLUMN"};
    const QString clause = filterLineEdit.GetConditionTemplate("CASE_SENSITVE_COLUMN", ColumnFilterLineEdit::EXPR_TYPE::STR_INSTR);
    const QString expectClause = R"(INSTR(`CASE_SENSITVE_COLUMN`,"%1")>0)";
    QCOMPARE(clause, expectClause);
  }
};

#include "ColumnFilterLineEditTest.moc"
REGISTER_TEST(ColumnFilterLineEditTest, false)
