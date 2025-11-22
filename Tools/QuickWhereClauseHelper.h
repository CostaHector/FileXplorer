#ifndef QUICKWHERECLAUSEHELPER_H
#define QUICKWHERECLAUSEHELPER_H

#include <QString>
#include <QHash>
#include <QString>
#include <QStack>
class QSqlQuery;

namespace QuickWhereClauseHelper {
extern const QString FUZZY_INSTR;
extern const QString FUZZY_LIKE;
extern const QString OPEATOR_RELATION;
constexpr char LOGIC_OR_CHAR = '|';
constexpr char LOGIC_AND_CHAR = '&';

void OperatorJoinOperands(QStack<QString>& rpn, QStack<QChar>& ops);
QString InfixNotation2RPN2Value(const QString& infixNot,        //
                                const QString& unaryCondition,  //
                                const QHash<QString, QString>& ALIAS_MAP = {});

QString GetSelectMovieByCastStatement(const QString& aCastName, const QString& akas, const QString& tableName);
QString GetMovieFullPathFromSqlQry(QSqlQuery& query);
}

#endif // QUICKWHERECLAUSEHELPER_H
