#ifndef PERFORMERSAKAMANAGER_H
#define PERFORMERSAKAMANAGER_H

#include <QHash>
#include <QMessageBox>
#include <QSqlRecord>
#include <QStack>
#include <QString>
#include <QWidget>

class PerformersAkaManager {
 public:
  static PerformersAkaManager& getIns();
  static const QString FUZZY_LIKE;
  static const QString OPEATOR_RELATION;
  static QString PlainLogicSentence2FuzzySqlWhere(const QString& keyName,                       //,
                                                  const QString& tokens,                        //
                                                  const QString& binaryCondition = FUZZY_LIKE,  //
                                                  const QHash<QString, QString>& ALSO_DICT = {});

  PerformersAkaManager(const PerformersAkaManager& rhs) noexcept = delete;

  QHash<QString, QString> ReadOutAkaName();
  int ForceReloadAkaName();

  static int UpdateAKAHash(const bool isForce = false);
  QHash<QString, QString> m_akaPerf;
  QString GetMovieTablePerformerSelectCommand(const QSqlRecord& record) const;

  inline int count() const { return m_akaPerf.size(); }

 private:
  PerformersAkaManager();

  static void OperatorJoinOperands(QStack<QString>& values, QStack<QChar>& ops);
  static const QHash<QChar, QString> op2Str;
  static constexpr char LOGIC_OR_CHAR = '|';
  static constexpr char LOGIC_AND_CHAR = '&';
};

#endif  // PERFORMERSAKAMANAGER_H
