#ifndef DUPVIDSMANAGER_H
#define DUPVIDSMANAGER_H

#include <QString>
#include <QSet>
#include <QHash>
#include <QMap>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "DbManager.h"

struct DUP_INFO {
  QString name;
  qint64 sz;
  int dur;
  qint64 modifiedDate;
  QString abspath;
  QString hash;
  bool exist;
};

// C:/DISK/F24 => C__DISK_F24
// /home/costa/Document => _HOME_COSTA_DOCUMENT
QString GetTableName(const QString& pathName);
// DISK_F24 => C:/DISK/F24
// _HOME_COSTA_DOCUMENT => /home/costa/Document
QString TableName2Path(const QString& tableName);

struct DupTableModelData {
  QString tableName;
  int count;
};

class DupVidsManager : public DbManager {
 private:
  DupVidsManager(const QString& dbName, const QString& connName, QObject* parent = nullptr);

 public:
  static DupVidsManager& GetInst();

  static const QString CREATE_DUP_VID_TABLE_TEMPLATE;
  static const QString INSERT_DUP_VID_TEMPLATE;
  int ScanLocations(const QStringList& paths);
  bool ScanALocation(const QString& path);
  bool IsTableExist(const QString& tableName) const;

  int DropTables(const QStringList& delTables);
  int AuditTables(const QStringList& atTables, bool auditAll = false);
  int RebuildTables(const QStringList& atTables);

  int GetTablesCnt() const;

  static const QString READ_DUP_VID_TABLE_META_INFO;
  QHash<qint64, QString> ReadATable(const QString& tableName);

  static const QString FIND_SAME_SIZE_VID;
  static const QString UPDATE_HASH_BY_PRIMARY_KEY;
  int FillHashFieldIfSizeConflict(const QString& path);

  static const QString READ_DUP_INFO_FROM_TABLES;
  int ReadSpecifiedTables2List(const QStringList& tbls, QList<DUP_INFO>& vidsInfo);
  QList<DupTableModelData> TableName2Cnt();
  static QString GetAiDupVidDbPath();

 private:
  static constexpr char CONNECTION_NAME[]{"AI_MEDIA_DUP_CONNECT"};
  QHash<QString, QString> mEscapePairPath{{"C:/DISK/F24", "C:/DISK/F24BKP"}, {"C:/DISK/LD2", "C:/DISK/LDBKPP"}};
};

#endif  // DUPVIDSMANAGER_H
