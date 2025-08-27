#ifndef QUICKWHERECLAUSEHELPER_H
#define QUICKWHERECLAUSEHELPER_H

#include <QString>
#include <QHash>
#include <QString>
#include <QStack>
class QSqlQuery;

namespace QuickWhereClauseHelper {
extern const QString FUZZY_LIKE;
extern const QString OPEATOR_RELATION;
extern const QHash<QChar, QString> op2Str;
constexpr char LOGIC_OR_CHAR = '|';
constexpr char LOGIC_AND_CHAR = '&';

QString InfixNotation2RPN2Value(const QString& fieldName,                       //,
                                const QString& infixNot,                        //
                                const QString& binaryCondition = FUZZY_LIKE,  //
                                const QHash<QString, QString>& ALIAS_MAP = {});
void OperatorJoinOperands(QStack<QString>& rpn, QStack<QChar>& ops);

QString GetSelectMovieByCastStatement(const QString& aCastName, const QString& akas, const QString& tableName);
QString GetMovieFullPathFromSqlQry(QSqlQuery& query);
}

#endif // QUICKWHERECLAUSEHELPER_H
