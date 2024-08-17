#ifndef AIMEDIADUPLICATE_H
#define AIMEDIADUPLICATE_H

#include <QString>
#include <QSet>
#include <QHash>
#include <QMap>
#include <QSqlDatabase>
#include <QSqlQuery>

// input string => output string, for example:
// C:/A/B/C.ext => B/C.ext
// C:/A/Videos/C.ext => A/Videos/C.ext
// C:/A/Video/C.ext => A/Video/C.ext
// C:/A/Vid/C.ext => A/Vid/C.ext
// C:/A/VIDEO_TS/C.ext => A/VIDEO_TS/C.ext
QString GetEffectiveName(const QString& itemPath);
// C:/DISK/F24 => DISK_F24
// /home/costa/Document => _HOME_COSTA_DOCUMENT
QString GetTableName(const QString& pathName);

enum class MATCH_MODE { SIZE, TOLERANCE_SIZE, FILE_NAME, CORE_FILE_NAME, DURATION, TOLERANCE_DURATION, HASH_OF_FILE_PART, HASH };

class AIMediaDuplicate {
 public:
  static AIMediaDuplicate& GetInst();
  ~AIMediaDuplicate();
  int ScanLocations(const QStringList& paths, bool eraseFirst = false, bool skipWhenItemExist = true);
  bool ScanALocation(const QString& path, bool eraseFirst = false, bool skipWhenItemExist = true);
  bool IsTableExist(const QString& tableName) const;
  int DropTables(const QStringList& delTables, bool dropAll = false);
  int GetTablesCnt() const;
  QHash<qint64, QString> ReadATabel(const QString& tabelName);

 private:
  AIMediaDuplicate();
  static const char DB[];
  static const char CONNECTION_NAME[];
  MATCH_MODE mMatchMode{MATCH_MODE::SIZE};
  QStringList mLocations;
  QString mInfosDBSavedPath;
  static const QString UNITED_TABLE_NAME;
  QHash<QString, QString> mEscapePairPath{{"C:/DISK/F24", "C:/DISK/F24BKP"}, {"C:/DISK/LD2", "C:/DISK/LDBKPP"}};
};

#endif  // AIMEDIADUPLICATE_H
