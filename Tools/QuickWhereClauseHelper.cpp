#include "QuickWhereClauseHelper.h"
#include "StringTool.h"
#include "TableFields.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "PathTool.h"
#include <QSet>
#include <QSqlQuery>
#include <QVariant>

namespace QuickWhereClauseHelper {
const QString FUZZY_INSTR{R"(INSTR(`%1`,"%2")>0)"}; // use Case Sensitive Search. %1 is field, %2 is value
const QString FUZZY_LIKE{R"(`%1` LIKE "%%2%")"};    // Case Insensitive. `Name%1` like "%Henry%2%"
const QString OPEATOR_RELATION{R"(`%1`%2)"};        // >10 => `Rate%1`>8%2. here %1 is field, %2 is value

void OperatorJoinOperands(QStack<QString>& values, QStack<QChar>& ops) {
  QString val2 = values.top();
  values.pop();

  QString val1 = values.top();
  values.pop();

  QChar op = ops.top();
  ops.pop();
  static const QHash<QChar, QString> op2Str = {{'&', "AND"}, {'|', "OR"}};
  values << QString("(%1 %2 %3)").arg(val1).arg(op2Str[op]).arg(val2);
}

QString InfixNotation2RPN2Value(const QString& infixNot,       //
                                const QString& unaryCondition, //
                                const QHash<QString, QString>& ALIAS_MAP) {
  if (unaryCondition.isEmpty() || infixNot.isEmpty()) {
    return "";
  }
  static const QSet<QChar> CONTROL_CHAR = {'(', ')', '&', '|'};
  static const auto isdigit = [](QChar c) -> bool { return !CONTROL_CHAR.contains(c); };
  static const auto precedence = [](QChar c) -> int {
    if (c == '&') {
      return 2;
    }
    if (c == '|') {
      return 1;
    }
    return 0; // like open bracket '('
  };

  QStack<QString> values; // reverse poland expr
  QStack<QChar> ops;

  for (int i = 0; i < infixNot.size(); ++i) {
    if (infixNot[i] == '(') {
      ops << infixNot[i];
    } else if (infixNot[i] == ')') {
      while (!ops.empty() && ops.top() != '(') {
        OperatorJoinOperands(values, ops);
        // pop opening brace.
      }
      if (!ops.empty()) {
        ops.pop();
      }
    } else if (isdigit(infixNot[i])) {
      int startIndex = i;
      // There may be more than one
      // digits in number.
      while (i < infixNot.length() && isdigit(infixNot[i])) {
        i++;
      }
      // autoCompleteAka
      const QString& perf = infixNot.mid(startIndex, i - startIndex);
      values << unaryCondition.arg(ALIAS_MAP.value(perf, perf));

      // right now the i points to
      // the character next to the digit,
      // since the for loop also increases
      // the i, we would skip one
      //  token position; we need to
      // decrease the value of i by 1 to
      // correct the offset.
      i--;
    } else {
      // While top of 'ops' has same or greater
      // precedence to current token, which
      // is an operator. Apply operator on top
      // of 'ops' to top two elements in values stack.
      while (!ops.empty() && precedence(ops.top()) >= precedence(infixNot[i])) {
        OperatorJoinOperands(values, ops);
      }

      // Push current token to 'ops'.
      ops.push(infixNot[i]);
    }
  }
  while (!ops.empty()) {
    OperatorJoinOperands(values, ops);
  }
  return values.top();
}

QString GetSelectMovieByCastStatement(const QString& aCastName, const QString& akas, const QString& tableName) {
  // movies table
  using namespace MOVIE_TABLE;

  QString whereClause;
  if (akas.isEmpty()) {
    whereClause = FUZZY_LIKE.arg(ENUM_2_STR(Name)).arg(aCastName);
  } else {
    // castName with its akas seperated by '|'. e.g., "Henry Cavill|Superman"
    QString castNames{aCastName};
    castNames += LOGIC_OR_CHAR;
    castNames += QString{akas}.replace(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR, LOGIC_OR_CHAR);
    whereClause = InfixNotation2RPN2Value(castNames, FUZZY_LIKE.arg(ENUM_2_STR(Name), "%1"));
  }
  static const QString SELECT_NAME_TEMPLATE{                                   //
                                            QString{"SELECT `%1`, `%2`, `%3`"} //
                                                .arg(ENUM_2_STR(PrePathLeft))  //
                                                .arg(ENUM_2_STR(PrePathRight)) //
                                                .arg(ENUM_2_STR(Name))};
  return SELECT_NAME_TEMPLATE + " FROM `" + tableName + "` WHERE " + whereClause;
}

QString GetMovieFullPathFromSqlQry(QSqlQuery& query) {
  using namespace MOVIE_TABLE;
  return PathTool::RMFComponent::join(query.value(ENUM_2_STR(PrePathLeft)).toString(),  //
                                      query.value(ENUM_2_STR(PrePathRight)).toString(), //
                                      query.value(ENUM_2_STR(Name)).toString());
}

} // namespace QuickWhereClauseHelper
