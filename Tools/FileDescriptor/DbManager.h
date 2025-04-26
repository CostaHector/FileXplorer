#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QFileInfo>
#include <QSqlDatabase>

// 审计结果结构
struct COUNT {
  int refound = 0;             // 从AGED表恢复的记录数
  int exist_and_same = 0;      // 存在且路径相同的记录
  int exist_and_not_same = 0;  // 存在但路径不同的记录
  int insert = 0;              // 新插入的记录
  int aged = 0;                // 标记为过期的记录
};

enum FD_ERROR_CODE {
  FD_NOT_DIR = -1000,
  FD_DB_INVALID,
  FD_DB_OPEN_FAILED,
  FD_CONNECT_NAME_NOT_EXIST,
  FD_PREPARE_FAILED,
  FD_TRANSACTION_FAILED,
  FD_REPLACE_INTO_FAILED,
  FD_EXEC_FAILED,
  FD_COMMIT_FAILED,
  FD_SKIP,
  FD_OK = 0,
};

class DbManager : public QObject {
 public:
  enum class DROP_OR_DELETE {
    DROP = 0,
    DELETE = 1,
  };
  static QString GetRmvCmd(DROP_OR_DELETE dropOrDelete);

  DbManager(const QString& dbName, const QString& connName, QObject* parent = nullptr);
  ~DbManager();
  bool CreateDatabase();
  bool CreateTable(const QString& tableName, const QString& tableDefinitionTemplate);
  int RmvTable(const QString& tableNameRegexPattern, DROP_OR_DELETE dropOrDelete);
  int DropTable(const QString& tableNameRegexPattern) {
    // Deletes the entire table along with its structure, indexes, triggers, and all associated objects.
    return RmvTable(tableNameRegexPattern, DROP_OR_DELETE::DROP);
  }
  int ClearTable(const QString& tableNameRegexPattern) {
    // Deletes specific records (rows) from the table but retains the table structure and its associated objects (such as indexes, triggers, etc.).
    return RmvTable(tableNameRegexPattern, DROP_OR_DELETE::DELETE);
  }
  bool DeleteDatabase();
  QSqlDatabase GetDb() const;
  bool CheckValidAndOpen(QSqlDatabase& db) const;
  static bool IsMatch(const QString& s, const QRegularExpression& regex);
  static const QString DROP_TABLE_TEMPLATE;
  static const QString DELETE_TABLE_TEMPLATE;

  bool QueryForTest(const QString& qryCmd, QList<QSqlRecord>& records) const;

 protected:
  void ReleaseConnection();
  bool mIsValid{false};
  const QString mDbName;
  const QString mConnName;
  static constexpr int MAX_BATCH_SIZE = 100;  // 每100条提交一次
};

class FdBasedDb : public DbManager {
 public:
  FdBasedDb(const QString& dbName, const QString& connName, QObject* parent = nullptr) : DbManager{dbName, connName, parent} {}
  int ReadADirectory(const QString& folderAbsPath, const QString& tableName);
  static const QString CREATE_TABLE_TEMPLATE;
  static const QString INSERT_NAME_ORI_IMGS_TEMPLATE;
  static QStringList VIDEOS_FILTER;
};

#endif  // DBMANAGER_H
