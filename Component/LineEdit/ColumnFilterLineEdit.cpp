#include "ColumnFilterLineEdit.h"
#include "QuickWhereClauseHelper.h"
#include "CastAkasManager.h"

ColumnFilterLineEdit::ColumnFilterLineEdit(const QString &columnName, QWidget *parent)
  : ReturnEventConsumingLineEdit{parent}
  , mColumnName{columnName}
  , mExprType{GetExprTypeFromColumnName(columnName)} {
  setPlaceholderText(GetConditionTemplate(mColumnName, mExprType));
}

QString ColumnFilterLineEdit::GetFilterExpression() const {
  const QString coreText = text();
  if (coreText.isEmpty()) {
    return coreText;
  }

  static QHash<QString, QString> EMPTY_HASH;
  const auto *p2AkaHash = &EMPTY_HASH;
  const bool bNeedAka{false && GetLineEditName() == "Name" || GetLineEditName() == "Cast"};
  if (bNeedAka) {
    static auto &dbTM = CastAkasManager::getInst();
    p2AkaHash = &dbTM.CastAkaMap();
  }

  return QuickWhereClauseHelper::InfixNotation2RPN2Value(coreText, placeholderText(), *p2AkaHash);
}

QString ColumnFilterLineEdit::GetConditionTemplate(const QString &columnName, const EXPR_TYPE exprType) {
  switch (exprType) {
    case EXPR_TYPE::STR_FUZZY_LIKE: {
      // R"(`Name` LIKE "%Chris Evans%")"
      return QuickWhereClauseHelper::FUZZY_LIKE.arg(columnName, "%1");
    }
    case EXPR_TYPE::STR_RELATION: // let it flow down
    case EXPR_TYPE::NUMBER_RELATION: {
      // R"(`Name` <>"Chris Evans")"
      // R"(`Height` > 180)"
      // R"(`Rate` >= 98)"
      return QuickWhereClauseHelper::OPEATOR_RELATION.arg(columnName, "%1");
    }
    case EXPR_TYPE::STR_INSTR: {
      // R"(INSTR(`ID`, "630")>0)"
      return QuickWhereClauseHelper::FUZZY_INSTR.arg(columnName, "%1");
    }
    default:
      return "";
  }
}

ColumnFilterLineEdit::EXPR_TYPE ColumnFilterLineEdit::GetExprTypeFromColumnName(const QString &name) {
  static const QMap<QString, EXPR_TYPE> name2Expr{
      {"Name", EXPR_TYPE::STR_FUZZY_LIKE},    //
      {"Rate", EXPR_TYPE::NUMBER_RELATION},   //
      {"Tags", EXPR_TYPE::STR_FUZZY_LIKE},    //
      {"Ori", EXPR_TYPE::STR_FUZZY_LIKE},     //
      {"Height", EXPR_TYPE::NUMBER_RELATION}, //
      {"Size", EXPR_TYPE::NUMBER_RELATION},   //
      {"Birth", EXPR_TYPE::STR_FUZZY_LIKE},   //
      {"ID", EXPR_TYPE::STR_INSTR},
  };
  return name2Expr.value(name, EXPR_TYPE::DEFAULT_EXPR_TYPE);
}

QString ColumnFilterLineEdit::BuildCombinedWhereClause(const QList<ColumnFilterLineEdit *> &lineEdits) {
  QStringList searchStatements;
  searchStatements.reserve(lineEdits.size());
  for (ColumnFilterLineEdit *pLe : lineEdits) {
    const QString fieldWhere = pLe->GetFilterExpression();
    if (fieldWhere.isEmpty()) {
      continue;
    }
    searchStatements.push_back(fieldWhere);
  }

  QString searchCmd;
  if (!searchStatements.isEmpty()) {
    searchCmd += searchStatements.join(" AND ");
  }
  return searchCmd;
}
