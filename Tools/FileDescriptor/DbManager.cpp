#include "DbManager.h"
#include "FileDescriptor.h"
#include "public/PathTool.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDirIterator>
#include <QVariant>
#include <QSqlError>
#include <QRegularExpression>

const QString DbManager::DROP_TABLE_TEMPLATE{"DROP TABLE `%1`;"};
const QString FdBasedDb::CREATE_TABLE_TEMPLATE{
    "CREATE TABLE IF NOT EXISTS %1 ("
    "fd BIGINT, "  // BIGINT
    "prePathLeft VARCHAR(255), "
    "prePathRight VARCHAR(255), "
    "Name VARCHAR(255), "
    "Size INTEGER, "
    "Duration INTEGER, "
    "tags TEXT, "
    "PRIMARY KEY (fd, prePathRight, Name))"};
const QString FdBasedDb::REPLACE_RECORD_TEMPLATE{
    "REPLACE INTO %1 (fd, prePathLeft, prePathRight, Name, Size, Duration, tags) "
    "VALUES (:fd, :prePathLeft, :prePathRight, :Name, :Size, :Duration, :tags)"};
constexpr int FdBasedDb::MAX_BATCH_SIZE;

DbManager::DbManager(const QString& dbName, const QString& connName)  //
    : mDbName{dbName}, mConnName{connName} {                          //
  if (dbName.isEmpty() || connName.isEmpty()) {
    qWarning("dbName[%s] or connName[%s] is empty", qPrintable(dbName), qPrintable(connName));
    return;
  }
  mIsValid = true;
}

void DbManager::ReleaseConnection() {
  // will not check. user must check if mConnName is invalid
  if (QSqlDatabase::contains(mConnName)) {
    QSqlDatabase::removeDatabase(mConnName);
  }
}

DbManager::~DbManager() {
  if (!mIsValid) {
    return;
  }
  ReleaseConnection();
}

QSqlDatabase DbManager::GetDb() {
  if (!mIsValid) {
    qWarning("invalid cannot Get db");
    return {};
  }
  QSqlDatabase db;
  if (QSqlDatabase::contains(mConnName)) {
    db = QSqlDatabase::database(mConnName);
  } else {
    db = QSqlDatabase::addDatabase("QSQLITE", mConnName);
    db.setDatabaseName(mDbName);
  }
  return db;
}

bool DbManager::IsMatch(const QString& s, const QRegularExpression& regex) {
  return regex.isValid() && regex.match(s).hasMatch();
}

bool DbManager::CreateDatabase() {
  if (!mIsValid) {
    qWarning("invalid cannot create database");
    return false;
  }
  const auto db = GetDb();
  return db.isValid();
}

bool DbManager::CreateTable(const QString& tableName, const QString& tableDefinitionTemplate) {
  if (!mIsValid) {
    qWarning("invalid cannot create table[%s]", qPrintable(tableName));
    return false;
  }
  if (!tableDefinitionTemplate.contains("%1")) {
    qWarning("tableDefinitionTemplate [%s] invalid", qPrintable(tableDefinitionTemplate));
    return false;
  }
  const QString& tableDefinition = tableDefinitionTemplate.arg(tableName);
  auto db = GetDb();
  if (!db.isValid()) {
    qWarning("db[%s] is invalid", qPrintable(db.connectionName()));
    return false;
  }
  if (!db.open()) {
    qWarning("db[%s] open failed", qPrintable(db.connectionName()));
    return false;
  }
  // 启用外键支持和WAL模式提升性能
  QSqlQuery query(db);
  query.exec("PRAGMA foreign_keys = ON");
  query.exec("PRAGMA journal_mode = WAL");
  query.exec("PRAGMA synchronous = NORMAL");
  if (!query.exec(tableDefinition)) {
    qWarning("Create table[%s] failed:\n%s", qPrintable(tableDefinition), qPrintable(query.lastError().text()));
    return false;  // db and query will destroyed when out of scope
  }
  qWarning("Table[%s] create succeed", qPrintable(tableName));
  return true;
}

int DbManager::DropTable(const QString& tableNameRegexPattern) {
  if (!mIsValid) {
    qWarning("invalid cannot drop table");
    return -1;
  }
  const QRegularExpression regex{tableNameRegexPattern};
  if (!regex.isValid()) {
    qWarning("regex[%s] is invalid", qPrintable(tableNameRegexPattern));
    return -1;
  }

  auto db = GetDb();
  if (!db.isValid()) {
    qWarning("db[%s] is invalid", qPrintable(db.connectionName()));
    return -1;
  }
  if (!db.open()) {
    qWarning("db[%s] open failed", qPrintable(db.connectionName()));
    return -1;
  }

  int succeedDropCnt = 0;
  QSqlQuery query(db);
  const QStringList& tables = db.tables();
  const QStringList& allTables = db.tables();
  for (const QString& tableName : allTables) {
    if (!IsMatch(tableName, regex)) {
      continue;
    }
    if (!query.exec(DROP_TABLE_TEMPLATE.arg(tableName))) {
      qWarning("Drop table[%s] failed:\n%s",  //
               qPrintable(tableName), qPrintable(query.lastError().text()));
      db.rollback();
      return -1;
    }
    ++succeedDropCnt;
  }
  qDebug("Drop %d table(s) from %d table(s) by user specified pattern[%s]",  //
         succeedDropCnt, allTables.size(), qPrintable(tableNameRegexPattern));
  return succeedDropCnt;
}

bool DbManager::DeleteDatabase() {
  if (!mIsValid) {
    qWarning("invalid cannot drop database");
    return false;
  }
  ReleaseConnection();
  if (QFile{mDbName}.exists()) {
    if (!QFile(mDbName).moveToTrash()) {
      qWarning("database[%s] move to trash failed", qPrintable(mDbName));
      return false;
    }
  }
  mIsValid = false;
  return true;
}

int FdBasedDb::ReadADirectory(const QString& folderAbsPath, const QString& tableName) {
  if (!QFileInfo(folderAbsPath).isDir()) {
    qWarning("folderAbsPath[%s] is not a directory", qPrintable(folderAbsPath));
    return FD_NOT_DIR;
  }

  auto db = GetDb();
  if (!db.isValid()) {
    qWarning("db[%s] is invalid", qPrintable(db.connectionName()));
    return FD_DB_INVALID;
  }
  if (!db.open()) {
    qWarning("db[%s] open failed", qPrintable(db.connectionName()));
    return FD_DB_OPEN_FAILED;
  }

  const QString replaceATable = REPLACE_RECORD_TEMPLATE.arg(tableName);

  // 准备查询语句
  QSqlQuery query(db);
  if (!query.prepare(replaceATable)) {
    qWarning("prepare command[%s] failed:\n%s",  //
             qPrintable(replaceATable), qPrintable(query.lastError().text()));
    return FD_PREPARE_FAILED;
  }

  // 开始事务
  if (!db.transaction()) {
    qWarning("start the %dth transaction failed:\n%s",  //
             1, qPrintable(query.lastError().text()));
    return FD_TRANSACTION_FAILED;
  }

  QDirIterator it(folderAbsPath, QStringList() << "*.mp4", QDir::Files, QDirIterator::Subdirectories);
  FileDescriptor fd;
  QString prePathLeft, prePathRight;
  int count = 0;
  while (it.hasNext()) {
    const QString& absFilePath = it.next();
    const QFileInfo fileInfo(absFilePath);
    PATHTOOL::GetPrepathParts(absFilePath, prePathLeft, prePathRight);

    auto fdVal = fd.GetFileUniquedId(absFilePath);
    // 绑定参数
    query.bindValue(":fd", fdVal);
    query.bindValue(":prePathLeft", prePathLeft);
    query.bindValue(":prePathRight", prePathRight);
    query.bindValue(":Name", fileInfo.fileName());
    query.bindValue(":Size", fileInfo.size());
    query.bindValue(":Duration", 49);
    query.bindValue(":tags", "");

    if (!query.exec()) {
      db.rollback();
      qWarning("replace[%s] failed:\n%s",  //
               qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
      return -1;
    }

    count++;

    // 分批提交
    if (count % MAX_BATCH_SIZE == 0) {
      if (!db.commit()) {
        db.rollback();
        qWarning("commit the %dth batch record(s) failed:\n%s",  //
                 count / MAX_BATCH_SIZE + 1, qPrintable(query.lastError().text()));
        return FD_COMMIT_FAILED;
      }
      if (!db.transaction()) {
        qWarning("start the %dth transaction failed:\n%s",  //
                 count / MAX_BATCH_SIZE + 2, qPrintable(query.lastError().text()));
        return FD_TRANSACTION_FAILED;
      }
    }
  }

  // 提交剩余记录
  if (!db.commit()) {
    db.rollback();
    qWarning("remain record(s) commit failed:\n%s", qPrintable(query.lastError().text()));
    return FD_COMMIT_FAILED;
  }

  qWarning("%d record(s) commit replaced into succeed", count);
  return count;
}
