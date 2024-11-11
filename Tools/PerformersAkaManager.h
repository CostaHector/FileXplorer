#ifndef PERFORMERSAKAMANAGER_H
#define PERFORMERSAKAMANAGER_H

#include <QHash>
#include <QMessageBox>
#include <QSqlRecord>
#include <QStack>
#include <QString>
#include <QWidget>
#include "PublicVariable.h"

class PerformersAkaManager {
 public:
  static PerformersAkaManager& getIns();
  PerformersAkaManager(const PerformersAkaManager& rhs) noexcept = delete;

  QHash<QString, QString> ReadOutAkaName();
  int ForceReloadAkaName();

  QString PlainLogicSentence2FuzzySqlWhere(const QString& tokens,
                                           const QString& keyName = DB_HEADER_KEY::Name,
                                           const bool autoCompleteAka = false,
                                           const QString& binaryCondition = "%1 like \"%%2%\"") const;

  static int UpdateAKAHash(const bool isForce = false);
  QHash<QString, QString> akaPerf;
  QString GetMovieTablePerformerSelectCommand(const QSqlRecord& record) const;

  inline int count() const { return akaPerf.size(); }

 private:
  PerformersAkaManager();

  static void OperatorJoinOperands(QStack<QString>& values, QStack<QChar>& ops);
  static const QHash<QChar, QString> op2Str;
  static constexpr char LOGIC_OR_CHAR = '|';
  static constexpr char LOGIC_AND_CHAR = '&';
};

#endif  // PERFORMERSAKAMANAGER_H
