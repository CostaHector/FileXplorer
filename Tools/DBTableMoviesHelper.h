#ifndef DBTABLEMOVIESHELPER_H
#define DBTABLEMOVIESHELPER_H

#include <QHash>
#include <QStack>
#include <QString>
#include "PublicVariable.h"
#include <QSqlRecord>

class DBTableMoviesHelper {
 public:
  static QString PlainLogicSentence2FuzzySqlWhere(const QString& tokens,
                                                  const QString& keyName = DB_HEADER_KEY::Name,
                                                  const bool autoCompleteAka = false,
                                                  const QString& binaryCondition = "%1 like \"%%2%\"");

  static int UpdateAKAHash(const bool isForce = false);
  static QHash<QString, QString> akaPerf;
  static QString GetMovieTablePerformerSelectCommand(const QSqlRecord& record);


 private:
  static void OperatorJoinOperands(QStack<QString>& values, QStack<QChar>& ops);
  static const QHash<QChar, QString> op2Str;
  static constexpr char LOGIC_OR_CHAR = '|';
  static constexpr char LOGIC_AND_CHAR = '&';
};

#endif  // DBTABLEMOVIESHELPER_H
