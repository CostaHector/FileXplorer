#ifndef PERFBASEDB_H
#define PERFBASEDB_H

#include "DbManager.h"

class PerfBaseDb : public DbManager {
 public:
  PerfBaseDb(const QString& dbName, const QString& connName, QObject* parent = nullptr)  //
      : DbManager{dbName, connName, parent} {}
  int InsertPerformers(const QStringList& perfList);
  int ReadFromImageHost(const QString& imgsHostPath);
  int ReadFromUserInputSentence(const QString& perfsText);
  int LoadFromPJsonFile(const QString& imgsHostPath);
  static const QString CREATE_PERF_TABLE_TEMPLATE;

  static QMap<QString, QString> GetFreqName2AkaNames(const QString& perfsText);
};

#endif  // PERFBASEDB_H
