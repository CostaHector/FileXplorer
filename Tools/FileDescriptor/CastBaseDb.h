#ifndef CASTBASEDB_H
#define CASTBASEDB_H

#include "DbManager.h"

class CastBaseDb : public DbManager {
 public:
  typedef QMap<QString, std::pair<QString, QStringList>> TCast2OriImgs;

  CastBaseDb(const QString& dbName, const QString& connName, QObject* parent = nullptr)  //
      : DbManager{dbName, connName, parent} {}
  int ReadFromImageHost(const QString& imgsHostPath);
  int AppendCastFromMultiLineInput(const QString& perfsText);
  int LoadFromPsonFile(const QString& imgsHostPath);
  static const QString CREATE_PERF_TABLE_TEMPLATE;

  static QMap<QString, QString> GetFreqName2AkaNames(const QStringList& perfsList);
  static QMap<QString, QString> GetFreqName2AkaNames(const QString& perfsText);
  static bool UpdateRecordImgsField(QSqlRecord& sqlRecord, const QString& imageHostPath);
  static QString GetCastPath(const QSqlRecord& sqlRecord, const QString& imageHostPath);
  static QString GetCastFilePath(const QSqlRecord& sqlRecord, const QString& imageHostPath);
  static bool IsNewOriFolderPathValid(const QString& destPath, const QString& imageHost, QString& newOri);
  static int MigrateToNewOriFolder(QSqlRecord& sqlRecord, QDir& imageHostDir, const QString& newOriFolder);
  static auto FromFileSystemStructure(const QString& imgsHostPath) -> TCast2OriImgs;
};

#endif  // CASTBASEDB_H
