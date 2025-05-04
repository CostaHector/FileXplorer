#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QFileInfo>
#include <QSqlDatabase>

struct VolumeUpdateResult {
  void Init() {
    deleteCnt = 0;
    insertCnt = 0;
    updateCnt = 0;
  }
  int deleteCnt;
  int insertCnt;
  int updateCnt;
};

enum FD_ERROR_CODE {
  FD_DISK_OFFLINE = -1000,
  FD_NOT_DIR,
  FD_NOT_INITED,
  FD_TABLE_NAME_INVALID,
  FD_TABLE_INEXIST,
  FD_FIELD_VALUE_INVALID,
  FD_DB_INVALID,
  FD_DB_OPEN_FAILED,
  FD_CONNECT_NAME_NOT_EXIST,
  FD_PREPARE_FAILED,
  FD_TRANSACTION_FAILED,
  FD_REPLACE_INTO_FAILED,
  FD_EXEC_FAILED,
  FD_COMMIT_FAILED,
  FD_QRY_PK_FAILED,
  FD_JSON_PATH_NOT_EXIST,
  FD_INVALID,
  FD_SKIP,
  FD_OK = 0,
};

// DROP DATABASE `DB_NAME`;

class DbManager : public QObject {
 public:
  enum class DROP_OR_DELETE {
    DROP = 0,
    DELETE = 1,
  };
  static QString GetRmvCmd(DROP_OR_DELETE dropOrDelete);
  bool IsValid() const { return mIsValid; }
  DbManager(const QString& dbName, const QString& connName, QObject* parent = nullptr);
  ~DbManager();

  bool CreateDatabase();
  bool CreateTable(const QString& tableName, const QString& tableDefinitionTemplate);
  int RmvTable(const QString& tableNameRegexPattern, DROP_OR_DELETE dropOrDelete, bool isFullMatch = true);
  int DropTable(const QString& tableNameRegexPattern) {
    // Deletes the entire table along with its structure, indexes, triggers, and all associated objects.
    return RmvTable(tableNameRegexPattern, DROP_OR_DELETE::DROP);
  }
  int ClearTable(const QString& tableNameRegexPattern) {
    // Deletes specific records (rows) from the table but retains the table structure and its associated objects (such as indexes, triggers, etc.).
    return RmvTable(tableNameRegexPattern, DROP_OR_DELETE::DELETE);
  }
  bool DeleteDatabase();
  QSqlDatabase GetDb(bool open = true) const;
  QString GetCfgDebug() const { return "table:" + mDbName + "| conn:" + mConnName; }
  bool CheckValidAndOpen(QSqlDatabase& db) const;
  static bool IsMatch(const QString& s, const QRegularExpression& regex);
  static const QString DROP_TABLE_TEMPLATE;
  static const QString DELETE_TABLE_TEMPLATE;

  bool QueryForTest(const QString& qryCmd, QList<QSqlRecord>& records) const;
  int UpdateForTest(const QString& qryCmd) const;
  bool QueryPK(const QString& tableName, const QString& pk, QSet<QString>& vals) const;
  bool QueryPK(const QString& tableName, const QString& pk, QSet<int>& vals) const;
  bool QueryPK(const QString& tableName, const QString& pk, QSet<qint64>& vals) const;

  int CountRow(const QString& tableName, const QString& whereClause = "");
  bool IsTableEmpty(const QString& tableName) const;
  int DeleteByWhereClause(const QString& tableName, const QString& whereClause);

  bool IsTableVolumeOnline(const QString& tableName) const;

 protected:
  static QString GetDeleteInPlaceholders(int n);
  void ReleaseConnection();
  bool mIsValid{false};
  const QString mDbName;
  const QString mConnName;
  static constexpr int MAX_BATCH_SIZE = 100;  // 每100条提交一次
};

#endif  // DBMANAGER_H
