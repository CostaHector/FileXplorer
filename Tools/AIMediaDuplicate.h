#ifndef AIMEDIADUPLICATE_H
#define AIMEDIADUPLICATE_H

#include <QString>
#include <QSet>
#include <QHash>
#include <QMap>
#include <QSqlDatabase>
#include <QSqlQuery>

struct DUP_INFO {
  QString name;
  qint64 sz;
  int dur;
  qint64 modifiedDate;
  QString abspath;
  QString hash;
  bool exist;
};

// input string => output string, for example:
// C:/A/B/C.ext => B/C.ext
// C:/A/Videos/C.ext => A/Videos/C.ext
// C:/A/Video/C.ext => A/Video/C.ext
// C:/A/Vid/C.ext => A/Vid/C.ext
// C:/A/VIDEO_TS/C.ext => A/VIDEO_TS/C.ext
QString GetEffectiveName(const QString& itemPath);
// C:/DISK/F24 => C__DISK_F24
// /home/costa/Document => _HOME_COSTA_DOCUMENT
QString GetTableName(const QString& pathName);
// DISK_F24 => C:/DISK/F24
// _HOME_COSTA_DOCUMENT => /home/costa/Document
QString TableName2Path(const QString& tableName);

struct DupTableModelData{
  QString tableName;
  int count;
};

class AIMediaDuplicate {
 public:
  static AIMediaDuplicate& GetInst();
  ~AIMediaDuplicate();
  int ScanLocations(const QStringList& paths, bool dropFirst = false, bool skipWhenTableExist = true);
  bool ScanALocation(const QString& path, bool dropFirst = false, bool skipWhenTableExist = true);
  bool IsTableExist(const QString& tableName) const;

  int DropTables(const QStringList& delTables, bool dropAll = false);
  int AuditTables(const QStringList& atTables, bool auditAll = false);
  int RebuildTables(const QStringList& atTables, bool rebuildAll = false);

  int GetTablesCnt() const;
  QHash<qint64, QString> ReadATabel(const QString& tableName);
  int FillHashFieldIfSizeConflict(const QString& path);
  int ReadSpecifiedTables2List(const QStringList& tbls, QList<DUP_INFO>& vidsInfo);
  QList<DupTableModelData> TableName2Cnt();

 private:
  AIMediaDuplicate();
  static bool SKIP_GETTER_DURATION;
  static const char CONNECTION_NAME[];
  QString mInfosDBSavedPath;
  QHash<QString, QString> mEscapePairPath{{"C:/DISK/F24", "C:/DISK/F24BKP"}, {"C:/DISK/LD2", "C:/DISK/LDBKPP"}};

  static bool IS_TEST; // if true, mod on another table.
};

#endif  // AIMEDIADUPLICATE_H
