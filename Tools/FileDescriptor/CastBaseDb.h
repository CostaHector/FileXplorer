#ifndef CASTBASEDB_H
#define CASTBASEDB_H

#include "DbManager.h"

class CastBaseDb : public DbManager {
 public:
  typedef QMap<QString, std::pair<QString, QStringList>> TCast2OriImgs;

  using DbManager::DbManager;
  int ReadFromImageHost(const QString& imgsHostPath);
  int AppendCastFromMultiLineInput(const QString& perfsText);
  int LoadFromPsonFile(const QString& imgsHostPath);

  static const QString CREATE_PERF_TABLE_TEMPLATE;

  static QMap<QString, QString> GetFreqName2AkaNames(const QStringList& perfsList);
  static QMap<QString, QString> GetFreqName2AkaNames(const QString& perfsText);
  static QString GetImgsFieldNewValue(const QString& oriFullPath, const QString& actorName);
  static auto FromFileSystemStructure(const QString& imgsHostPath) -> TCast2OriImgs;  
};

#endif  // CASTBASEDB_H
