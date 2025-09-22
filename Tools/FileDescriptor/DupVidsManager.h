#ifndef DUPVIDSMANAGER_H
#define DUPVIDSMANAGER_H

#include <QString>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "DbManager.h"
#include "DupVideosHelper.h"

class DupVidsManager : public DbManager {
 public:
  explicit DupVidsManager(QObject* parent = nullptr);

  static const QString CREATE_DUP_VID_TABLE_TEMPLATE;
  static const QString INSERT_DUP_VID_TEMPLATE;
  int ScanLocations(const QStringList& paths);
  bool ScanALocation(const QString& path);
  bool IsTableExist(const QString& tableName) const;

  int DropTables(const QStringList& delTables);
  int AuditTables(const QStringList& atTables);
  int RebuildTables(const QStringList& atTables);

  int GetTablesCnt() const;

  static const QString TABLE_NAME_2_VIDEOS_COUNT;
  DupVidTableName2RecordCountList TableName2Cnt();

  static const QString FIND_SAME_SIZE_VID;
  static const QString UPDATE_HASH_BY_PRIMARY_KEY;
  int FillHashFieldIfSizeConflict(const QString& path);

  static const QString READ_DUP_INFO_FROM_TABLES;
  int ReadSpecifiedTables2List(const QStringList& tbls, DupVidMetaInfoList& vidInfoList);
  static QString GetAiDupVidDbPath();

 private:
  static DupVidsManager& GetInst() = delete; // singleton is forbidden

  static constexpr char VID_DUP_CONNECTION_NAME[]{"AI_MEDIA_DUP_CONNECT"};
  QHash<QString, QString> mEscapePairPath{{"C:/DISK/F24", "C:/DISK/F24BKP"}, {"C:/DISK/LD2", "C:/DISK/LDBKPP"}};
};

#endif  // DUPVIDSMANAGER_H
