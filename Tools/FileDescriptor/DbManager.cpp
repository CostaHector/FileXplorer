#include "DbManager.h"
#include "MountHelper.h"
#include "PublicTool.h"
#include "Logger.h"

#include <QSet>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDirIterator>
#include <QVariant>
#include <QRegularExpression>

SafeTransaction::SafeTransaction(QSqlDatabase db) : m_db(db) {
  if (!DbManager::CheckValidAndOpen(m_db)) {
    LOG_W("invalid QSqlDatabase invalid or closed");
    mIsDbValid = false;
    return;
  }
  if (!m_db.transaction()) {
    LOG_W("Start transaction failed(Database may already in transaction): %s", qPrintable(m_db.lastError().text()));
    mIsDbValid = false;
    return;
  }
  mIsDbValid = true;
}

SafeTransaction::~SafeTransaction() {
  if (!mIsDbValid) {  // invalid database
    return;
  }
  if (mIsAlreadyCommitted) {  // transaction already commit no need
    return;
  }
  if (m_db.rollback()) {
    LOG_D("Transaction auto-rolled back succeed");
  } else {
    LOG_W("Auto rollback failed: %s", qPrintable(m_db.lastError().text()));
  }
}

bool SafeTransaction::commit() {
  if (!mIsDbValid) {
    LOG_W("invalid QSqlDatabase invalid or closed. cannot commit");
    return false;
  }

  if (m_db.commit()) {
    mIsAlreadyCommitted = true;
    return true;
  }

  LOG_W("Commit failed: %s", qPrintable(m_db.lastError().text()));
  return false;
}

constexpr int DbManager::MAX_BATCH_SIZE;

#ifdef RUNNING_UNIT_TESTS
bool DbManager::DropAllTablesForTest(const QString& connName) {
  if (!QSqlDatabase::connectionNames().contains(connName)) {
    LOG_D("connName[%s] not in pool, cannot used for table drop", qPrintable(connName));
    return true;
  }
  QSqlDatabase db = QSqlDatabase::database(connName, true);
  if (!db.isOpen() && !db.open()) {
    LOG_E("Failed to open database to drop all tables");
    return false;
  }
  db.transaction();
  const QStringList& tables = db.tables();
  LOG_D("Start to drop %d table(s)", tables.size());
  QSqlQuery query(db);

  if (!query.exec("PRAGMA foreign_keys = OFF")) {
    LOG_W("Failed to disable foreign keys: %s", qPrintable(query.lastError().text()));
  }

  for (const QString& table : tables) {
    if (!query.exec(DbManagerHelper::command(DbManagerHelper::DropOrDeleteE::DROP).arg(table))) {
      LOG_W("Drop failed table[%s]: %s", qPrintable(table), qPrintable(query.lastError().text()));
      db.rollback();
      return false;
    } else {
      LOG_D("Drop ok table[%s]", qPrintable(table));
    }
  }

  if (!db.commit()) {
    LOG_E("Commit failed: %s", qPrintable(db.lastError().text()));
    db.rollback();
    return false;
  }
  return true;
}
#endif

DbManager::DbManager(const QString& dbName, const QString& connName, QObject* parent)  //
    : QObject{parent}, mDbName{dbName}, mConnName{connName} {                          //
  if (dbName.isEmpty() || connName.isEmpty()) {
    LOG_W("dbName[%s] or connName[%s] is empty", qPrintable(dbName), qPrintable(connName));
    return;
  }
  mIsValid = true;
}

void DbManager::ReleaseConnection() {
  mIsValid = false;

  if (!QSqlDatabase::contains(mConnName)) {
    LOG_D("Connection[%s] already not found in connection pool", qPrintable(mConnName));
    return;
  }
  QSqlDatabase db = QSqlDatabase::database(mConnName, false);  // by default close
  if (db.isOpen()) {
    db.close();
    LOG_D("Database connection closed: %s", qPrintable(mConnName));
  }
  // Todo:
  // QSqlDatabase::removeDatabase(mConnName); // QSqlDatabasePrivate::removeDatabase: connection 'AI_MEDIA_DUP_CONNECT' is still in use, all queries
  // will cease to work.
}

DbManager::~DbManager() {
  if (!mIsValid) {
    return;
  }
  ReleaseConnection();  // todo
}

bool DbManager::IsTableExist(const QString& tableName) const {
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  return db.tables().contains(tableName);
}

bool DbManager::IsTableEmpty(const QString& tableName) const {
  QSqlDatabase db = GetDb();
  if (!CheckValidAndOpen(db)) {
    LOG_W("Open failed:%s", qPrintable(db.lastError().text()));
    return true;
  }
  QSqlQuery qry{db};
  qry.prepare(QString{"SELECT * FROM `%1`"}.arg(tableName));
  if (!qry.exec()) {
    LOG_W("select[%s] failed: %s",  //
          qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
  }
  return !qry.next();
}

QSqlDatabase DbManager::GetDb(bool open) const {
  if (!mIsValid) {
    LOG_W("invalid cannot Get db");
    return {};
  }
  QSqlDatabase db;
  if (QSqlDatabase::contains(mConnName)) {
    db = QSqlDatabase::database(mConnName, open);
  } else {
    db = QSqlDatabase::addDatabase("QSQLITE", mConnName);
    db.setDatabaseName(mDbName);
  }
  if (open && !db.isOpen()) {
    if (!db.open()) {
      LOG_W("Open %s failed", qPrintable(GetCfgDebug()));
    }
  }
  return db;
}

bool DbManager::QueryForTest(const QString& qryCmd, QList<QSqlRecord>& records) const {
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  QSqlQuery qry{db};
  if (!qry.exec(qryCmd)) {
    QSqlError error = qry.lastError();
    LOG_W("cmd[%s] failed", qPrintable(qry.executedQuery()));
    LOG_W("SQL error: %s", qPrintable(error.text()));
    LOG_W("Error type: %d", error.type());
    LOG_W("Error database text: %s", qPrintable(error.databaseText()));
    LOG_W("Error driver text: %s", qPrintable(error.driverText()));
    db.rollback();
    return false;
  }
  while (qry.next()) {
    records << qry.record();
  }
  LOG_D("%d records find by [%s]", records.size(), qPrintable(qryCmd));
  return true;
}

int DbManager::UpdateForTest(const QString& qryCmd) const {
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_INVALID;
  }
  QSqlQuery qry{db};  // command in constructor call right now
  if (!qry.exec(qryCmd)) {
    QSqlError error = qry.lastError();
    LOG_W("cmd[%s] failed", qPrintable(qry.executedQuery()));
    LOG_W("SQL error: %s", qPrintable(error.text()));
    LOG_W("Error type: %d", error.type());
    LOG_W("Error database text: %s", qPrintable(error.databaseText()));
    LOG_W("Error driver text: %s", qPrintable(error.driverText()));
    db.rollback();
    return FD_EXEC_FAILED;
  }
  int affectedRows = qry.numRowsAffected();
  LOG_D("%d records affected by [%s]", affectedRows, qPrintable(qryCmd));
  qry.finish();
  return affectedRows;
}

bool DbManager::QueryPK(const QString& tableName, const QString& pk, QSet<QString>& vals) const {
  vals.clear();
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  const QString qryCmd{QString{"SELECT `%1` FROM `%2`"}.arg(pk).arg(tableName)};
  QSqlQuery qry{db};
  if (!qry.exec(qryCmd)) {
    LOG_W("cmd[%s] failed:%s", qPrintable(qry.lastQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return false;
  }
  while (qry.next()) {
    vals << qry.value(pk).toString();
  }
  LOG_D("%d records find by [%s]", vals.size(), qPrintable(qryCmd));
  return true;
}

bool DbManager::QueryPK(const QString& tableName, const QString& pk, QSet<int>& vals) const {
  vals.clear();
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  const QString qryCmd{QString{"SELECT `%1` FROM `%2`"}.arg(pk).arg(tableName)};
  QSqlQuery qry{db};
  if (!qry.exec(qryCmd)) {
    LOG_W("cmd[%s] failed:%s", qPrintable(qry.lastQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return false;
  }
  while (qry.next()) {
    vals << qry.value(pk).toInt();
  }
  LOG_D("%d records find by [%s]", vals.size(), qPrintable(qryCmd));
  return true;
}

bool DbManager::QueryPK(const QString& tableName, const QString& pk, QSet<QByteArray>& vals) const {
  vals.clear();
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  const QString qryCmd{QString{"SELECT `%1` FROM `%2`"}.arg(pk).arg(tableName)};
  QSqlQuery qry{db};
  if (!qry.exec(qryCmd)) {
    LOG_W("cmd[%s] failed:%s", qPrintable(qry.lastQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return false;
  }
  while (qry.next()) {
    vals << qry.value(pk).toByteArray();
  }
  LOG_D("%d records find by [%s]", vals.size(), qPrintable(qryCmd));
  return true;
}

int DbManager::CountRow(const QString& tableName, const QString& whereClause) {
  QSqlDatabase db = GetDb();
  if (!CheckValidAndOpen(db)) {
    LOG_W("Open failed:%s", qPrintable(db.lastError().text()));
    return FD_DB_OPEN_FAILED;
  }

  QString countCmd = QString("SELECT COUNT(*) FROM `%1`").arg(tableName);
  if (!whereClause.isEmpty()) {
    countCmd += (" WHERE " + whereClause);
  }

  QSqlQuery qry{db};
  if (!qry.exec(countCmd)) {
    LOG_W("count[%s] failed: %s",  //
          qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_EXEC_FAILED;
  }
  qry.next();
  return qry.value(0).toInt();
}

int DbManager::DeleteByWhereClause(const QString& tableName, const QString& whereClause) {
  QSqlDatabase db = GetDb();
  if (!CheckValidAndOpen(db)) {
    LOG_W("Open failed:%s", qPrintable(db.lastError().text()));
    return FD_DB_OPEN_FAILED;
  }

  QString deleteCmd{QString(R"(DELETE FROM "%1")").arg(tableName)};
  if (!whereClause.isEmpty()) {  // empty means delete all records
    deleteCmd += " WHERE ";
    deleteCmd += whereClause;
  }
  deleteCmd += ';';

  QSqlQuery qry{db};
  if (!qry.exec(deleteCmd)) {
    db.rollback();
    LOG_W("delete cmd[%s] failed: %s",  //
          qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_EXEC_FAILED;
  }
  const int affectedRows = qry.numRowsAffected();
  LOG_D("affectedRows:%d by cmd:%s", affectedRows, qPrintable(deleteCmd));
  qry.finish();
  return affectedRows;
}

bool DbManager::IsTableVolumeOnline(const QString& tableName) {
  QString mountPath = MountPathTableNameMapper::toMountPath(tableName);
  return QFileInfo(mountPath).isDir();
}

bool DbManager::onShowInFileSystemView() const {
  if (!QFile::exists(mDbName)) {
    LOG_W("Database[%s] not exist, open failed", qPrintable(mDbName));
    return false;
  }
  return FileTool::OpenLocalFileUsingDesktopService(mDbName);
}

QString DbManager::GetDeleteInPlaceholders(int n) {
  QString placeholders;
  placeholders.reserve(n * 2);
  for (int i = 0; i < n; ++i) {
    placeholders += "?,";
  }
  if (!placeholders.isEmpty()) {  // chop last 1 char(s) ","
    static constexpr int LAST_CHARS_CNT = 1;
    placeholders.chop(LAST_CHARS_CNT);
  }
  return placeholders;
}

bool DbManager::CheckValidAndOpen(QSqlDatabase& db) {
  if (!db.isValid()) {
    LOG_W("db[%s] is invalid", qPrintable(db.connectionName()));
    return false;
  }
  if (!db.isOpen()) {
    LOG_W("db[%s] not opened %s", qPrintable(db.connectionName()), qPrintable(db.lastError().text()));
    return false;
  }
  return true;
}

bool DbManager::CreateDatabase() {
  const auto db = GetDb();
  return db.isValid();
}

bool DbManager::CreateTable(const QString& tableName, const QString& tableDefinitionTemplate) {
  if (IsTableExist(tableName)) {
    LOG_D("table[%s] already exist. skip create", qPrintable(tableName));
    return true;
  }

  if (tableName.isEmpty()) {
    LOG_W("table name[%s] cannot be empty", qPrintable(tableName));
    return false;
  }
  if (!tableDefinitionTemplate.contains("%1")) {
    LOG_W("table definition[%s] invalid", qPrintable(tableDefinitionTemplate));
    return false;
  }
  const QString& tableDefinition = tableDefinitionTemplate.arg(tableName);
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }

  // 启用外键支持和WAL模式提升性能
  QSqlQuery query(db);
  query.exec("PRAGMA journal_mode = WAL");  // WAL/DELETE, the former may generate .db-wal file
  query.exec("PRAGMA foreign_keys = ON");
  query.exec("PRAGMA synchronous = NORMAL");
  if (!query.exec(tableDefinition)) {
    LOG_W("Create table[%s] failed: %s", qPrintable(tableDefinition), qPrintable(query.lastError().text()));
    return false;  // db and query will destroyed when out of scope
  }
  LOG_D("Table[%s] create succeed", qPrintable(tableName));
  return true;
}

int DbManager::RmvTable(const QString& tableName, DbManagerHelper::DropOrDeleteE dropOrDelete) {
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }
  const QStringList& allTables = db.tables();
  if (!allTables.contains(tableName)) {
    LOG_D("table[%s] already not", qPrintable(tableName));
    return 0;
  }

  const QString rmvCmdTemplate{DbManagerHelper::command(dropOrDelete)};
  if (rmvCmdTemplate.isEmpty()) {
    LOG_W("rmvCmdTemplate empty. mode[%d] invalid", (int)dropOrDelete);
    return FD_DB_INVALID;
  }

  QSqlQuery query(db);
  if (!query.exec(rmvCmdTemplate.arg(tableName))) {
    LOG_W("Drop table[%s] failed: %s",  //
          qPrintable(tableName), qPrintable(query.lastError().text()));
    db.rollback();
    return FD_EXEC_FAILED;
  }
  LOG_D("Drop table[%s] succced", qPrintable(tableName));
  return 1;
}
