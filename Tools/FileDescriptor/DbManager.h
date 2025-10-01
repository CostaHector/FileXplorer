#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QFileInfo>
#include <QSqlDatabase>
#include "DbManagerHelper.h"

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
  FD_TABLE_NAME_PATTERN_INVALID,
  FD_TABLE_INEXIST = -1000 + 5,
  FD_FIELD_VALUE_INVALID,
  FD_DB_INVALID,
  FD_DB_OPEN_FAILED,
  FD_CONNECT_NAME_NOT_EXIST,
  FD_PREPARE_FAILED = -1000 + 10,
  FD_TRANSACTION_FAILED,
  FD_REPLACE_INTO_FAILED,
  FD_EXEC_FAILED,
  FD_COMMIT_FAILED,
  FD_QRY_PK_FAILED = -1000 + 15,
  FD_JSON_PATH_NOT_EXIST,
  FD_RENAME_FAILED,
  FD_CAST_NEW_ORI_PATH_INVALID,
  FD_SET_RECORDS_FAILED,
  FD_CAST_PSON_PREPATH_MAKE_FAILED = -1000 + 20,
  FD_INVALID,
  FD_SKIP,
  FD_OK = 0,
};

class SafeTransaction {
 public:
  explicit SafeTransaction(QSqlDatabase db);
  ~SafeTransaction();
  explicit operator bool() const {
    /*
     before:
      if (!db.transaction()) {
        db.rollback();
        return ERROR_CODE;
      }
     now:
      SafeTransaction safeTransaction;
      if (!safeTransaction) {
        return ERROR_CODE;
        // here in destructor will auto roll back
      }
     */
    return mIsDbValid;
  }
  bool commit();
 private:
  SafeTransaction(const SafeTransaction&) = delete;
  SafeTransaction& operator=(const SafeTransaction&) = delete;

  QSqlDatabase m_db;
  bool mIsDbValid{false};
  bool mIsAlreadyCommitted{false};
};

class DbManager : public QObject {
 public:
#ifdef RUNNING_UNIT_TESTS
  static bool DropAllTablesForTest(const QString& connName);
  static bool DropDatabaseForTest(const QString& dbFullName, const bool bRecycle = true);
  bool DeleteDatabaseIselfForTest(bool bRecyle = true);
#endif

  DbManager(const QString& dbName, const QString& connName, QObject* parent = nullptr);
  ~DbManager();

  bool IsValid() const { return mIsValid; }
  static bool CheckValidAndOpen(QSqlDatabase& db);

  bool CreateDatabase();
  bool CreateTable(const QString& tableName, const QString& tableDefinitionTemplate);

  int RmvTable(const QString& tableName, DbManagerHelper::DropOrDeleteE dropOrDelete);
  // Deletes the entire table along with its structure, indexes, triggers, and all associated objects.
  int DropTable(const QString& tableName) { return RmvTable(tableName, DbManagerHelper::DropOrDeleteE::DROP); }
  // Deletes specific records (rows) from the table but retains the table structure and its associated objects (such as indexes, triggers, etc.).
  int ClearTable(const QString& tableName) { return RmvTable(tableName, DbManagerHelper::DropOrDeleteE::DELETE); }

  bool IsTableExist(const QString& tableName) const;
  bool IsTableEmpty(const QString& tableName) const;

  QSqlDatabase GetDb(bool open = true) const;
  QString GetCfgDebug() const { return "table:" + mDbName + "| conn:" + mConnName; }

  bool QueryForTest(const QString& qryCmd, QList<QSqlRecord>& records) const;
  int UpdateForTest(const QString& qryCmd) const;
  bool QueryPK(const QString& tableName, const QString& pk, QSet<QString>& vals) const;
  bool QueryPK(const QString& tableName, const QString& pk, QSet<int>& vals) const;
  bool QueryPK(const QString& tableName, const QString& pk, QSet<qint64>& vals) const;

  int CountRow(const QString& tableName, const QString& whereClause = "");
  int DeleteByWhereClause(const QString& tableName, const QString& whereClause);

  bool IsTableVolumeOnline(const QString& tableName) const;

  void setSkipGetVideosDuration(bool bSkip) { mSkipGetVideosDuration = bSkip; }
  bool isSkipGetVideosDuration() const { return mSkipGetVideosDuration; }

  bool onShowInFileSystemView() const;

 protected:
  static QString GetDeleteInPlaceholders(int n);
  void ReleaseConnection();
  bool mIsValid{false};
  const QString mDbName;
  const QString mConnName;
  static constexpr int MAX_BATCH_SIZE = 100;  // 每100条提交一次

  bool mSkipGetVideosDuration = false;

 private:
  DbManager(const DbManager&) = delete;
  DbManager& operator=(const DbManager&) = delete;
};

#endif  // DBMANAGER_H
