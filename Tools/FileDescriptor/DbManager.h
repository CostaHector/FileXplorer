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

class DbManager {
 public:
  DbManager(const QString& dbName, const QString& connName);
  ~DbManager();
  bool CreateDatabase();
  bool CreateTable(const QString& tableName, const QString& tableDefinitionTemplate);
  int DropTable(const QString& tableNameRegexPattern);
  bool DeleteDatabase();
  static bool IsMatch(const QString& s, const QRegularExpression& regex);
  static const QString DROP_TABLE_TEMPLATE;

 protected:
  void ReleaseConnection();
  QSqlDatabase GetDb();
  bool mIsValid{false};
  const QString mDbName;
  const QString mConnName;
};

class FdBasedDb : public DbManager {
 public:
  enum FD_ERROR_CODE {
    FD_NOT_DIR = -1000,
    FD_DB_INVALID,
    FD_DB_OPEN_FAILED,
    FD_CONNECT_NAME_NOT_EXIST,
    FD_PREPARE_FAILED,
    FD_TRANSACTION_FAILED,
    FD_REPLACE_INTO_FAILED,
    FD_COMMIT_FAILED,
    FD_OK = 0,
  };
  FdBasedDb(const QString& dbName, const QString& connName) : DbManager{dbName, connName} {}
  int ReadADirectory(const QString& folderAbsPath, const QString& tableName);
  static const QString CREATE_TABLE_TEMPLATE;
  static const QString REPLACE_RECORD_TEMPLATE;

 private:
  static constexpr int MAX_BATCH_SIZE = 100;  // 每100条提交一次
};
#endif  // DBMANAGER_H
