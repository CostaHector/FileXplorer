#ifndef COLUMNFILTERLINEEDIT_H
#define COLUMNFILTERLINEEDIT_H

#include "ReturnEventConsumingLineEdit.h"

class ColumnFilterLineEdit : public ReturnEventConsumingLineEdit {
public:
  enum class EXPR_TYPE {
    DEFAULT_EXPR_TYPE = 0,
    STR_FUZZY_LIKE = DEFAULT_EXPR_TYPE, // case in-sensitive
    STR_RELATION,
    NUMBER_RELATION,
    STR_INSTR, // case sensitive
  };

  explicit ColumnFilterLineEdit(const QString& columnName, QWidget* parent = nullptr);
  QString GetFilterExpression() const;
  static QString BuildCombinedWhereClause(const QList<ColumnFilterLineEdit*>& lineEdits);
  static QString GetConditionTemplate(const QString& columnName, const EXPR_TYPE exprType);
  const QString& GetLineEditName() const { return mColumnName; }

private:
  static EXPR_TYPE GetExprTypeFromColumnName(const QString& name);
  const QString mColumnName;
  const EXPR_TYPE mExprType;
};

#endif // COLUMNFILTERLINEEDIT_H
