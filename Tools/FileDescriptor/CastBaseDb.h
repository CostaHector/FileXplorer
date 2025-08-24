#ifndef CASTBASEDB_H
#define CASTBASEDB_H

#include "DbManager.h"

class CastBaseDb : public DbManager {
 public:
  CastBaseDb(const QString& dbName, const QString& connName, QObject* parent = nullptr)  //
      : DbManager{dbName, connName, parent} {}
  int ReadFromImageHost(const QString& imgsHostPath);
  int AppendCastFromMultiLineInput(const QString& perfsText);
  int LoadFromPsonFile(const QString& imgsHostPath);
  static const QString CREATE_PERF_TABLE_TEMPLATE;

  static QMap<QString, QString> GetFreqName2AkaNames(const QString& perfsText);
  static bool UpdateRecordImgsField(QSqlRecord& sqlRecord, const QString& imageHostPath);
  static QString GetCastPath(const QSqlRecord& sqlRecord, const QString& imageHostPath);
  static QString GetCastFilePath(const QSqlRecord& sqlRecord, const QString& imageHostPath);
};

#endif  // CASTBASEDB_H
